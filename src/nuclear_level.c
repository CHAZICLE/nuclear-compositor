#include "nuclear_utils.h"
#include "nuclear_surface.h"

#include "nuclear_level.h"


#define LEVEL_RESOLUTION_HORZ 18
#define LEVEL_RESOLUTION_VERT 36

GLuint levelVertexBuffer;
GLuint progId;
GLuint mvpId;

void nuclear_level_preinit()
{
	// Horizontal lines
	GLfloat data[LEVEL_RESOLUTION_HORZ*LEVEL_RESOLUTION_VERT*3];
	int c = 0;
	float vertSubd = 360.f/LEVEL_RESOLUTION_VERT;
	float horzSubd = 360.f/LEVEL_RESOLUTION_HORZ;
	for(int v=0;v<LEVEL_RESOLUTION_VERT;v++)
	{
		for(int h=0;h<LEVEL_RESOLUTION_HORZ;h++)
		{
			data[c*3+0] = cos(radians(v*vertSubd)) * sin(radians(h*horzSubd));
			data[c*3+1] = cos(radians(v*vertSubd)) * cos(radians(h*horzSubd));
			data[c*3+2] = sin(radians(v*vertSubd));
			c++;
		}
	}
	glGenBuffers(1, &levelVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, levelVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*LEVEL_RESOLUTION_HORZ*LEVEL_RESOLUTION_VERT*3, data, GL_STATIC_DRAW);

	progId = loadShaders("src/shaders/simple.vert", "src/shaders/simple.frag");
	mvpId = glGetUniformLocation(progId, "MVP");
}
void nuclear_level_init(nuclear_level *lvl, nuclear_atom *atm, float radius)
{
	lvl->atom = atm;
	lvl->radius = radius;

	wl_list_init(&lvl->surfaces);
}
void nuclear_level_render(nuclear_level *lvl, mat4x4 atomMatrix)
{
	glUseProgram(progId);

	mat4x4 levelMatrix;
	mat4x4_scale_aniso(levelMatrix, atomMatrix, lvl->radius, lvl->radius, lvl->radius);

	glUniformMatrix4fv(mvpId, 1, GL_FALSE, &levelMatrix[0][0]);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, levelVertexBuffer);
	glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);
	glDrawArrays(GL_LINES, 0, LEVEL_RESOLUTION_HORZ*LEVEL_RESOLUTION_VERT);

	//nuclear_surface *surface;
	//wl_list_for_each_reverse (surface, &lvl->surfaces, link)
	//{
	//	if (!surface->xdg_surface)
	//		continue;
	//	if (surface == lvl->moving_surface)
	//		texture_draw (&surface->texture, srv->pointer_x + surface->x, srv->pointer_y + surface->y);
	//	else
	//		texture_draw (&surface->texture, surface->x, surface->y);
	//	if (surface->frame_callback) {
	//		wl_callback_send_done (surface->frame_callback, backend_get_timestamp());
	//		wl_resource_destroy (surface->frame_callback);
	//		surface->frame_callback = NULL;
	//	}
	//}
	//// draw the cursor last
	//if (srv->cursor)
	//	texture_draw (&srv->cursor->texture, srv->pointer_x, srv->pointer_y);
}
void nuclear_level_destroy(nuclear_level *lvl)
{
	
}
