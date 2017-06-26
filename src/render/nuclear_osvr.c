#include <stdio.h>
#include "nuclear_server.h"

#include "nuclear_osvr.h"

#ifdef NUCLEAR_OSVR

#define CHECKOSVRCALL(osvrCall, message) if((osvrCall)!=OSVR_RETURN_SUCCESS) { fprintf(stderr, "%s\n", message); print_glerror(message); osvrDestroyRenderManager(srnd->osvrRender); exit(1); }

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
static OSVR_CBool makeCurrentImpl(void* data, size_t display) {<
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

bool nuclear_osvr_init(nuclear_renderer *rnd, nuclear_server *srv)
{
	rnd->render = &nuclear_osvr_render_universe;

	// Set up renderer
	nuclear_osvr_renderer *srnd = malloc(sizeof(nuclear_osvr_renderer));
	rnd->data = srnd;

	long init_start = backend_get_timestamp();
	srnd->osvrCtx = osvrClientInit("local.nuclear_compositor", 0);
	while(osvrClientCheckStatus(srnd->osvrCtx)!=OSVR_RETURN_SUCCESS)
	{
		if(backend_get_timestamp()>init_start+5000)
		{
			return false;
		}
	}

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
	srnd->osvrLibrary.toolkit = toolkit;

	if(OSVR_RETURN_SUCCESS != osvrCreateRenderManagerOpenGL(srnd->osvrCtx, "OpenGL", srnd->osvrLibrary, &srnd->osvrRender, &srnd->osvrRenderGL))
	{
		fprintf(stderr, "Failed to create an OpenGL render manager\n");
		exit(1);
	}

	// Open the display
	OSVR_OpenResultsOpenGL openResults;
	if(osvrRenderManagerOpenDisplayOpenGL(srnd->osvrRenderGL, &openResults)!=OSVR_RETURN_SUCCESS
		|| openResults.status==OSVR_OPEN_STATUS_FAILURE)
	{
		osvrDestroyRenderManager(srnd->osvrRender);
		fprintf(stderr, "Failed to create an OpenGL render manager");
		exit(1);
	}
	
	osvrClientUpdate(srnd->osvrCtx);

	// Setup render render-to-texture
	CHECKOSVRCALL(
			osvrRenderManagerGetDefaultRenderParams(&srnd->renderParams),
			"Failed to get default render params"
		)

	OSVR_RenderInfoCollection renderInfoCollection;
	CHECKOSVRCALL(
			osvrRenderManagerGetRenderInfoCollection(srnd->osvrRender, srnd->renderParams, &renderInfoCollection),
			"Failed to get render info collection"
		)

	OSVR_RenderInfoCount numRenderInfo;
	CHECKOSVRCALL(
			osvrRenderManagerGetNumRenderInfoInCollection(renderInfoCollection, &numRenderInfo),
			"Failed to get number of render infos"
		)

	glGenFramebuffers(1, &srnd->osvrFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, srnd->osvrFrameBuffer);
	srnd->osvrRenderBuffers = calloc(numRenderInfo, sizeof(OSVR_RenderBufferOpenGL));

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

		srnd->osvrRenderBuffers[i].colorBufferName = osvrColorBufferName;
		srnd->osvrRenderBuffers[i].depthStencilBufferName = osvrDepthBufferName;

		CHECKOSVRCALL(
				osvrRenderManagerRegisterRenderBufferOpenGL(registerBufferState, srnd->osvrRenderBuffers[i]),
				"Failed to register render buffer"
			)
	}
	CHECKOSVRCALL(
			osvrRenderManagerReleaseRenderInfoCollection(renderInfoCollection),
			"Failed to release render info collection"
		)
	CHECKOSVRCALL(
			osvrRenderManagerFinishRegisterRenderBuffers(srnd->osvrRender, registerBufferState, false),
			"Failed to release render info collection"
		)

	return true;
}
void nuclear_osvr_render_server(nuclear_renderer *rnd, nuclear_server *srv)
{
	nuclear_osvr_renderer *srnd = (nuclear_osvr_renderer *)rnd->data;

	osvrClientUpdate(srnd->osvrCtx);
	rnd->redraw = 1;

	nuclear_osvr_render_universe(rnd, rnd->active_universe);
}
void nuclear_osvr_render_universe(nuclear_renderer *rnd, nuclear_universe *unv)
{
	nuclear_osvr_renderer *srnd = (nuclear_osvr_renderer *)rnd->data;
#define CHECKOSVRCALL(osvrCall, message) if((osvrCall)!=OSVR_RETURN_SUCCESS) { osvrDestroyRenderManager(srnd->osvrRender);fprintf(stderr, message); exit(1); }
	OSVR_RenderInfoCollection renderInfoCollection = {0};
	CHECKOSVRCALL(
			osvrRenderManagerGetRenderInfoCollection(srnd->osvrRender, srnd->renderParams, &renderInfoCollection),
			"Failed to get render info collection in main loop"
		)
	OSVR_RenderInfoCount numRenderInfo;
	CHECKOSVRCALL(
			osvrRenderManagerGetNumRenderInfoInCollection(renderInfoCollection, &numRenderInfo),
			"Failed to fetch renderInfoCount"
		)
	
	// Render into framebuffers
	for(size_t i=0;i<numRenderInfo;i++)
	{
		OSVR_RenderInfoOpenGL renderInfo = {0};
		osvrRenderManagerGetRenderInfoFromCollectionOpenGL(renderInfoCollection, i, &renderInfo);
		//render
		nuclear_universe_osvr_renderEye(unv, &renderInfo, i);
	}

	// Present the rendered
	OSVR_RenderManagerPresentState presentState;
	CHECKOSVRCALL(
			osvrRenderManagerStartPresentRenderBuffers(&presentState),
			"Could not start presenting render buffers"
		)
	OSVR_ViewportDescription fullView;
	fullView.left = fullView.lower = 0;
	fullView.width = fullView.height = 1;
	for(size_t i=0;i<numRenderInfo;i++)
	{
		OSVR_RenderInfoOpenGL renderInfo = {0};
		CHECKOSVRCALL(
				osvrRenderManagerGetRenderInfoFromCollectionOpenGL(renderInfoCollection, i, &renderInfo),
				"Could not fetch render info from collection"
			)
		CHECKOSVRCALL(
				osvrRenderManagerPresentRenderBufferOpenGL(presentState, srnd->osvrRenderBuffers[i], renderInfo, fullView),
				"Could not present render buffer"
			)
	}
	CHECKOSVRCALL(
			osvrRenderManagerFinishPresentRenderBuffers(srnd->osvrRender, presentState, srnd->renderParams, false),
			"Could not finish presenting render buffers"
		)
	osvrRenderManagerReleaseRenderInfoCollection(renderInfoCollection);
}
void nuclear_osvr_render_universe_osvrEye(nuclear_renderer *rnd, nuclear_universe *unv, OSVR_RenderInfoOpenGL *renderInfo, GLuint bufferName)
{
	nuclear_osvr_renderer *srnd = (nuclear_osvr_renderer *)rnd->data;
	nuclear_server *srv = unv->server;

	glBindFramebuffer(GL_FRAMEBUFFER, srnd->osvrFrameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, srnd->osvrRenderBuffers[bufferName].colorBufferName, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, srnd->osvrRenderBuffers[bufferName].depthStencilBufferName);

	//GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
	//glDrawBuffers(1, drawBuffers);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr, "Failed to do framebuffer shit");
		exit(1);
	}
	
	glViewport(0,
			0,
			renderInfo->viewport.width,
			renderInfo->viewport.height);

	double projectionMatrixDouble[16];
	double viewMatrixDouble[16];
	mat4x4 projectionMatrix = {0};
	mat4x4 viewMatrix = {0};
	mat4x4 viewProjectionMatrix = {0};

	renderInfo->projection.farClip = 1000.f;

	OSVR_Projection_to_OpenGL(projectionMatrixDouble, renderInfo->projection);
	OSVR_PoseState_to_OpenGL(viewMatrixDouble, renderInfo->pose);

	for(int i = 0; i < 16; i++)
	{
		*((float *)projectionMatrix+i) = (GLfloat)projectionMatrixDouble[i];
		*((float *)viewMatrix+i) = (GLfloat)viewMatrixDouble[i];
	}

	// Fix the OSVR axis
	mat4x4 rotationCorrectionMatrix;
	mat4x4_identity(rotationCorrectionMatrix);
	mat4x4_rotate_X(rotationCorrectionMatrix, rotationCorrectionMatrix, radians(-90.f));
	mat4x4_mul(viewMatrix, viewMatrix, rotationCorrectionMatrix);

	// Translate in world space
	mat4x4 worldTranslateMatrix;
	mat4x4_identity(worldTranslateMatrix);
	mat4x4_translate(worldTranslateMatrix, -rnd->position[0], -rnd->position[1], -rnd->position[2]);
	mat4x4_mul(viewMatrix, viewMatrix, worldTranslateMatrix);

	mat4x4_mul(viewProjectionMatrix, projectionMatrix, viewMatrix);

	glClearColor(0.f, 0.f, 0.f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Render all the atoms
	nuclear_atom *atom;
	wl_list_for_each_reverse(atom, &unv->atoms, next)
	{
		nuclear_atom_render(atom, projectionMatrix, viewProjectionMatrix);
	}
}

#endif
