#ifndef __NUCLEAR_COMPOSITOR_NUCLEAR_SERVER_INCLUDED__
#define __NUCLEAR_COMPOSITOR_NUCLEAR_SERVER_INCLUDED__

typedef struct _nuclear_server nuclear_server;

#include <wayland-server.h>
#include "nuclear_universe.h"
#include "nuclear_server.h"
#include "nuclear_client.h"
#define OSVR_RM_USE_OPENGLES20
#include <osvr/ClientKit/ClientKitC.h>
#include <osvr/ClientKit/DisplayC.h>
#include <osvr/RenderKit/RenderManagerOpenGLC.h>

struct _nuclear_server {
	nuclear_universe *active_universe;
	struct wl_list universes;
	struct wl_list clients;

	struct wl_display *display;

	bool enableOSVR;
	OSVR_ClientContext osvrCtx;
	OSVR_GraphicsLibraryOpenGL osvrLibrary;
	OSVR_RenderManager osvrRender;
	OSVR_RenderManagerOpenGL osvrRenderGL;
	OSVR_RenderParams renderParams;
	GLuint osvrFrameBuffer;
	OSVR_RenderBufferOpenGL *osvrRenderBuffers;
	int redraw;
};

void nuclear_server_init(nuclear_server *srv, bool enableOSVR);
nuclear_client *nuclear_server_get_client(nuclear_server *srv, struct wl_client *_client);
void nuclear_server_run(nuclear_server *srv);
void nuclear_server_destroy(nuclear_server *srv);

#endif
