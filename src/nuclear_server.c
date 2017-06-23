#include "backend.h"
#include "nuclear_utils.h"
#include "backend-callbacks.h"
#include "nuclear_impl_compositor.h"
#include "nuclear_seat.h"
#include "nuclear_universe.h"
#include <stdlib.h>

#include "nuclear_server.h"

#define TOOLKIT_ECHO fprintf(stderr, "Toolkit function: %s\n", __func__);

static void createImpl(void* data) {
	TOOLKIT_ECHO
}
static void destroyImpl(void* data) {
}
static OSVR_CBool addOpenGLContextImpl(void* data, const OSVR_OpenGLContextParams* p) {
	TOOLKIT_ECHO
	return true;
}
static OSVR_CBool removeOpenGLContextsImpl(void* data) {
	TOOLKIT_ECHO
	return true;
}
static OSVR_CBool makeCurrentImpl(void* data, size_t display) {
	//TOOLKIT_ECHO
	backend_make_current();
	return true;
}
static OSVR_CBool swapBuffersImpl(void* data, size_t display) {
	//TOOLKIT_ECHO
	backend_swap_buffers();
	return true;
}
static OSVR_CBool setVerticalSyncImpl(void* data, OSVR_CBool verticalSync) {
	TOOLKIT_ECHO
	return true;
}
static OSVR_CBool handleEventsImpl(void* data) {
	//TOOLKIT_ECHO
	return true;
}
static OSVR_CBool getDisplayFrameBufferImpl(void* data, size_t display, GLuint* displayFrameBufferOut) {
	//TOOLKIT_ECHO
	*displayFrameBufferOut = 0;
	return false;
}
static OSVR_CBool getDisplaySizeOverrideImpl(void* data, size_t display, int* width, int* height) {
	TOOLKIT_ECHO
	*width = 1920;
	*height = 1080;
	return true;
}

#define CHECKOSVRCALL(osvrCall, message) if((osvrCall)!=OSVR_RETURN_SUCCESS) { fprintf(stderr, "%s\n", message); print_glerror(message); osvrDestroyRenderManager(srv->osvrRender); exit(1); }

void nuclear_server_init(nuclear_server *srv, bool enableOSVR)
{
	srv->enableOSVR = enableOSVR;
	if(srv->enableOSVR)
	{
		long init_start = backend_get_timestamp();
		srv->osvrCtx = osvrClientInit("local.nuclear_compositor", 0);
		while(osvrClientCheckStatus(srv->osvrCtx)!=OSVR_RETURN_SUCCESS)
		{
			if(backend_get_timestamp()>init_start+5000)
			{
				srv->enableOSVR = 0;
				break;
			}
		}
	}

	backend_init(&nuclear_backend_callbacks, srv);

	srv->display = wl_display_create();
	wl_display_add_socket_auto(srv->display);
	wl_global_create(srv->display, &wl_compositor_interface, 3, NULL, &nuclear_impl_compositor_bind);
	wl_global_create(srv->display, &wl_seat_interface, 1, NULL, &nuclear_seat_bind);
	//wl_global_create(display, &wl_shell_interface, 1, NULL, &shell_bind);
	//wl_global_create(display, &xdg_shell_interface, 1, NULL, &xdg_shell_bind);
	eglBindWaylandDisplayWL(backend_get_egl()->display, srv->display);
	wl_display_init_shm(srv->display);
	wl_list_init(&srv->universes);
	wl_list_init(&srv->clients);


	srv->redraw = 1;

	srv->active_universe = malloc(sizeof(nuclear_universe));
	nuclear_universe_init(srv->active_universe, srv);
	wl_list_insert(&srv->universes, &srv->active_universe->link);

	nuclear_universe_preinit();

	if(srv->enableOSVR)
	{
		// Init RenderManager
		OSVR_OpenGLToolkitFunctions *toolkit = calloc(1, sizeof(OSVR_OpenGLToolkitFunctions));
		toolkit->size = sizeof(OSVR_OpenGLToolkitFunctions);
		toolkit->data = srv;
		
		toolkit->create = createImpl;
		toolkit->destroy = destroyImpl;
		toolkit->addOpenGLContext = addOpenGLContextImpl;
		toolkit->removeOpenGLContexts = removeOpenGLContextsImpl;
		toolkit->makeCurrent = makeCurrentImpl;
		toolkit->swapBuffers = swapBuffersImpl;
		toolkit->setVerticalSync = setVerticalSyncImpl;
		toolkit->handleEvents = handleEventsImpl;
		toolkit->getDisplayFrameBuffer = getDisplayFrameBufferImpl;
		toolkit->getDisplaySizeOverride = getDisplaySizeOverrideImpl;
		srv->osvrLibrary.toolkit = toolkit;

		if(OSVR_RETURN_SUCCESS != osvrCreateRenderManagerOpenGL(srv->osvrCtx, "OpenGL", srv->osvrLibrary, &srv->osvrRender, &srv->osvrRenderGL))
		{
			fprintf(stderr, "Failed to create an OpenGL render manager\n");
			exit(1);
		}

		// Open the display
		OSVR_OpenResultsOpenGL openResults;
		if(osvrRenderManagerOpenDisplayOpenGL(srv->osvrRenderGL, &openResults)!=OSVR_RETURN_SUCCESS
			|| openResults.status==OSVR_OPEN_STATUS_FAILURE)
		{
			osvrDestroyRenderManager(srv->osvrRender);
			fprintf(stderr, "Failed to create an OpenGL render manager");
			exit(1);
		}
		
		osvrClientUpdate(srv->osvrCtx);

		// Setup render render-to-texture
		CHECKOSVRCALL(
				osvrRenderManagerGetDefaultRenderParams(&srv->renderParams),
				"Failed to get default render params"
			)

		OSVR_RenderInfoCollection renderInfoCollection;
		CHECKOSVRCALL(
				osvrRenderManagerGetRenderInfoCollection(srv->osvrRender, srv->renderParams, &renderInfoCollection),
				"Failed to get render info collection"
			)

		OSVR_RenderInfoCount numRenderInfo;
		CHECKOSVRCALL(
				osvrRenderManagerGetNumRenderInfoInCollection(renderInfoCollection, &numRenderInfo),
				"Failed to get number of render infos"
			)

		glGenFramebuffers(1, &srv->osvrFrameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, srv->osvrFrameBuffer);
		srv->osvrRenderBuffers = calloc(numRenderInfo, sizeof(OSVR_RenderBufferOpenGL));

		OSVR_RenderManagerRegisterBufferState registerBufferState;
		CHECKOSVRCALL(
				osvrRenderManagerStartRegisterRenderBuffers(&registerBufferState),
				"Failed start registering render buffers"
			)
		for(size_t i=0;i<numRenderInfo;i++)
		{
			OSVR_RenderInfoOpenGL renderInfo = {0};
			CHECKOSVRCALL(
					osvrRenderManagerGetRenderInfoFromCollectionOpenGL(renderInfoCollection, i, &renderInfo),
					"Could not get render info"
				)

			GLuint osvrColorBufferName = 0;
			GLuint osvrDepthBufferName = 0;
			int width = renderInfo.viewport.width;
			int height = renderInfo.viewport.height;

			CHECKOSVRCALL(
					osvrRenderManagerCreateColorBufferOpenGL(width, height, GL_RGBA, &osvrColorBufferName),
					"Could not create color buffer"
				)
			CHECKOSVRCALL(
					osvrRenderManagerCreateDepthBufferOpenGL(width, height, &osvrDepthBufferName),
					"Could not create depth buffer"
				)

			srv->osvrRenderBuffers[i].colorBufferName = osvrColorBufferName;
			srv->osvrRenderBuffers[i].depthStencilBufferName = osvrDepthBufferName;

			CHECKOSVRCALL(
					osvrRenderManagerRegisterRenderBufferOpenGL(registerBufferState, srv->osvrRenderBuffers[i]),
					"Failed to register render buffer"
				)
		}
		CHECKOSVRCALL(
				osvrRenderManagerReleaseRenderInfoCollection(renderInfoCollection),
				"Failed to release render info collection"
			)
		CHECKOSVRCALL(
				osvrRenderManagerFinishRegisterRenderBuffers(srv->osvrRender, registerBufferState, false),
				"Failed to release render info collection"
			)

	}
	fprintf(stderr, "Init complete\n");
}
void nuclear_server_run(nuclear_server *srv)
{
	struct wl_event_loop *event_loop = wl_display_get_event_loop(srv->display);
	int wayland_fd = wl_event_loop_get_fd(event_loop);

	// Create projection matrix
	mat4x4 projectionMatrix;
	mat4x4_perspective(projectionMatrix, (75.f)/(180.f/3.1415f), 16.0f/9.0f, 0.1f, 1000.f);

	while (1) {
		wl_event_loop_dispatch(event_loop, 0);
		backend_dispatch_nonblocking();
		wl_display_flush_clients(srv->display);
		if(srv->redraw) {

			if(srv->enableOSVR)
			{
				osvrClientUpdate(srv->osvrCtx);
				srv->redraw = 1;

				nuclear_universe *universe;
				wl_list_for_each_reverse (universe, &srv->universes, link) {
					nuclear_universe_osvrRender_render(universe);
				}
			}
			else
			{
				glClearColor(0.0f, 0.0f, 0.05f, 0.0f);
				glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

				srv->redraw = 0;
				nuclear_universe *universe;
				wl_list_for_each_reverse (universe, &srv->universes, link) {
					nuclear_universe_render(universe, projectionMatrix);
				}
				backend_swap_buffers();
			}

		}
		else {
			backend_wait_for_events(wayland_fd);
		}
		do_backend_render(srv);
	}
}
nuclear_client *nuclear_server_get_client(nuclear_server *srv, struct wl_client *_client)
{
	nuclear_client *nuclear_client;
	wl_list_for_each(nuclear_client, &srv->clients, link) {
		if(nuclear_client->client == _client)
			return nuclear_client;
	}
	nuclear_client = calloc((size_t)1, sizeof(nuclear_client));
	nuclear_client->client = _client;
	wl_list_insert(&srv->clients, &nuclear_client->link);
	return nuclear_client;
}
void nuclear_server_destroy(nuclear_server *srv)
{
	wl_display_destroy (srv->display);
}
