#include "nuclear_utils.h"
#include "utils/icosphere.h"
#include "common/nuclear_atom.h"

#include "nuclear_renderer.h"

static GLuint levelVertexBuffer;
static GLuint levelTriangleIndexBuffer;
static GLuint levelLineIndexBuffer;
static GLuint progId;
static GLuint projectionMatrixId,viewMatrixId,modelMatrixId;
static struct icosphere *levelSphere;

void nuclear_renderer_glinit()
{
	levelSphere = create_icosphere(2);

	glGenBuffers(1, &levelVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, levelVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*levelSphere->vertices*3, levelSphere->vertexPositionBuffer, GL_STATIC_DRAW);

	glGenBuffers(1, &levelTriangleIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, levelTriangleIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*levelSphere->triangles*3, levelSphere->triangleIndexBuffer, GL_STATIC_DRAW);

	glGenBuffers(1, &levelLineIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, levelLineIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*levelSphere->lines*2, levelSphere->lineIndexBuffer, GL_STATIC_DRAW);

	progId = loadShaders("src/shaders/simple.vert", "src/shaders/simple.frag");
	projectionMatrixId = glGetUniformLocation(progId, "P");
	viewMatrixId = glGetUniformLocation(progId, "V");
	modelMatrixId = glGetUniformLocation(progId, "M");
}
void nuclear_renderer_render_universe(nuclear_renderer *rnd, nuclear_universe *unv, mat4x4 projectionMatrix, mat4x4 viewMatrix)
{
	// Render all the atoms
	nuclear_atom *atm;
	wl_list_for_each_reverse(atm, &unv->atoms, next)
	{
		nuclear_renderer_render_atom(rnd, atm, projectionMatrix, viewMatrix);
	}
}
void nuclear_renderer_render_atom(nuclear_renderer *rnd, nuclear_atom *atm, mat4x4 projectionMatrix, mat4x4 viewMatrix)
{
	mat4x4 modelMatrix;
	mat4x4_identity(modelMatrix);
	mat4x4_translate(modelMatrix, atm->position[0], atm->position[1], atm->position[2]);

	// Render all the levels
	nuclear_level *level;
	wl_list_for_each(level, &atm->levels, next)
	{
		nuclear_renderer_render_level(rnd, level, projectionMatrix, viewMatrix, modelMatrix);
	}
}
void nuclear_renderer_render_level(nuclear_renderer *rnd, nuclear_level *lvl, mat4x4 projectionMatrix, mat4x4 viewMatrix, mat4x4 modelMatrix)
{
	glUseProgram(progId);

	mat4x4_scale_aniso(modelMatrix, modelMatrix, lvl->radius, lvl->radius, lvl->radius);

	glUniformMatrix4fv(projectionMatrixId, 1, GL_FALSE, &projectionMatrix[0][0]);
	glUniformMatrix4fv(viewMatrixId, 1, GL_FALSE, &viewMatrix[0][0]);
	glUniformMatrix4fv(modelMatrixId, 1, GL_FALSE, &modelMatrix[0][0]);

	glBindBuffer(GL_ARRAY_BUFFER, levelVertexBuffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, levelTriangleIndexBuffer);
	glDrawElements(GL_TRIANGLES, levelSphere->triangles*3, GL_UNSIGNED_INT, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, levelLineIndexBuffer);
	glDrawElements(GL_LINES, levelSphere->lines*2, GL_UNSIGNED_INT, 0);

	glDisable(GL_BLEND);

	// draw the cursor last
	//if (srv->cursor)
	//	texture_draw (&srv->cursor->texture, srv->pointer_x, srv->pointer_y);

	// Render all surfaces
	nuclear_surface *srf;
	wl_list_for_each(srf, &lvl->surfaces, next)
	{
		nuclear_renderer_render_surface(rnd, srf, projectionMatrix, viewMatrix, modelMatrix);
	}
}
void nuclear_renderer_render_surface(nuclear_renderer *rnd, nuclear_surface *srf, mat4x4 projectionMatrix, mat4x4 viewMatrix, mat4x4 modelMatrix)
{
	//if (!surface->xdg_surface)
	//	continue;
	//if (surface == lvl->moving_surface)
	//	texture_draw (&surface->texture, srv->pointer_x + surface->x, srv->pointer_y + surface->y);
	//else
	//	texture_draw (&surface->texture, surface->x, surface->y);
	//if (surface->frame_callback) {
	//	wl_callback_send_done (surface->frame_callback, backend_get_timestamp());
	//	wl_resource_destroy (surface->frame_callback);
	//	surface->frame_callback = NULL;
	//}
}
