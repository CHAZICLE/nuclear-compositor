#include "nuclear_atom.h"
#include <stdlib.h>
#include <osvr/ClientKit/ClientKitC.h>
#include <osvr/ClientKit/DisplayC.h>
#include <stdio.h>
#include "nuclear_utils.h"
#include <string.h>
#include <math.h>

#include "nuclear_universe.h"

void nuclear_universe_preinit()
{
	nuclear_atom_preinit();
}
void nuclear_universe_init(nuclear_universe *unv, nuclear_server *srv)
{
	unv->server = srv;
	unv->yaw = 0.f;
	unv->pitch = 0.f;
	wl_list_init(&unv->atoms);
	unv->floating_position[0] = 0;
	unv->floating_position[1] = 0;
	unv->floating_position[2] = 0;

	nuclear_atom *atoms = malloc(sizeof(nuclear_atom)*3);
	for(int i=0;i<3;i++)
	{
		nuclear_atom_init(&atoms[i], unv, 0.f, 100.f*(i+2), 0.f);
		wl_list_insert(&unv->atoms, &atoms[i].link);
	}
	atoms[2].position[0] = 100.f;
	unv->active_atom = &atoms[0];
}
void nuclear_universe_osvrRender_render(nuclear_universe *unv)
{
	//fprintf(stderr, "Render universe\n");
	nuclear_server *srv = unv->server;
#define CHECKOSVRCALL(osvrCall, message) if((osvrCall)!=OSVR_RETURN_SUCCESS) { osvrDestroyRenderManager(srv->osvrRender);fprintf(stderr, message); exit(1); }
	OSVR_RenderInfoCollection renderInfoCollection = {0};
	CHECKOSVRCALL(
			osvrRenderManagerGetRenderInfoCollection(srv->osvrRender, srv->renderParams, &renderInfoCollection),
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
		nuclear_universe_osvrRender_renderEye(unv, &renderInfo, i);
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
				osvrRenderManagerPresentRenderBufferOpenGL(presentState, srv->osvrRenderBuffers[i], renderInfo, fullView),
				"Could not present render buffer"
			)
	}
	CHECKOSVRCALL(
			osvrRenderManagerFinishPresentRenderBuffers(srv->osvrRender, presentState, srv->renderParams, false),
			"Could not finish presenting render buffers"
		)
	osvrRenderManagerReleaseRenderInfoCollection(renderInfoCollection);
}
void nuclear_universe_osvrRender_renderEye(nuclear_universe *unv, OSVR_RenderInfoOpenGL *renderInfo, GLuint bufferName)
{
	//fprintf(stderr, "Render cell %d\n", bufferName);
	nuclear_server *srv = unv->server;

	glBindFramebuffer(GL_FRAMEBUFFER, srv->osvrFrameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, srv->osvrRenderBuffers[bufferName].colorBufferName, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, srv->osvrRenderBuffers[bufferName].depthStencilBufferName);

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
	mat4x4_translate(worldTranslateMatrix, -unv->floating_position[0], -unv->floating_position[1], -unv->floating_position[2]);
	mat4x4_mul(viewMatrix, viewMatrix, worldTranslateMatrix);

	mat4x4_mul(viewProjectionMatrix, projectionMatrix, viewMatrix);

	glClearColor(0.f, 0.f, 0.f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Render all the atoms
	nuclear_atom *atom;
	wl_list_for_each_reverse (atom, &unv->atoms, link) {
		nuclear_atom_render(atom, viewProjectionMatrix);
	}
}
void nuclear_universe_render(nuclear_universe *unv, mat4x4 projectionMatrix)
{
	// Create view matrix
	mat4x4 viewMatrix;
	vec3 direction = {
		cos(unv->pitch) * sin(unv->yaw),
		cos(unv->pitch) * cos(unv->yaw),
		sin(unv->pitch)
	};
	vec3 right = {
		sin(unv->yaw + 3.14f/2.0f),
		cos(unv->yaw + 3.14f/2.0f),
		0,
	};
	vec3 up;
	vec3_mul_cross(up, right, direction);
	vec3 lookAt;
	vec3_add(lookAt, unv->floating_position, direction);
	mat4x4_look_at(viewMatrix, unv->floating_position, lookAt, up);

	mat4x4 viewProjectionMatrix;
	mat4x4_mul(viewProjectionMatrix, projectionMatrix, viewMatrix);

	nuclear_atom *atom;
	wl_list_for_each_reverse (atom, &unv->atoms, link) {
		nuclear_atom_render(atom, viewProjectionMatrix);
	}
}
void nuclear_universe_destroy(nuclear_universe *unv)
{
	nuclear_atom *atom;
	wl_list_for_each_reverse(atom, &unv->atoms, link) {
		nuclear_atom_destroy(atom);
	}
}
