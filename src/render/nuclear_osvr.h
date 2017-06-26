#ifndef __NUCLEAR_COMPOSITOR_OSVR_NUCLEAR_SERVER_OSVR_INCLUDED__
#define __NUCLEAR_COMPOSITOR_OSVR_NUCLEAR_SERVER_OSVR_INCLUDED__
#ifdef NUCLEAR_OSVR

typedef struct _nuclear_osvr_renderer nuclear_osvr_renderer;

#include "nuclear_server.h"

#define OSVR_RM_USE_OPENGLES20
#include <osvr/ClientKit/ClientKitC.h>
#include <osvr/ClientKit/DisplayC.h>
#include <osvr/RenderKit/RenderManagerOpenGLC.h>

struct _nuclear_osvr_renderer {
	OSVR_ClientContext osvrCtx;
	OSVR_GraphicsLibraryOpenGL osvrLibrary;
	OSVR_RenderManager osvrRender;
	OSVR_RenderManagerOpenGL osvrRenderGL;
	OSVR_RenderParams renderParams;
	GLuint osvrFrameBuffer;
	OSVR_RenderBufferOpenGL *osvrRenderBuffers;
};


bool nuclear_osvr_init(nuclear_renderer *rnd, nuclear_server *srv);
void nuclear_osvr_render_universe(nuclear_renderer *rnd, nuclear_universe *unv);
void nuclear_osvr_render_universe_osvrEye(nuclear_renderer *rnd, nuclear_universe *unv, OSVR_RenderInfoOpenGL *renderInfo, GLuint bufferName);

#endif
#endif
