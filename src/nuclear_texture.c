#include "main.h"
#include "nuclear_utils.h"

#include "nuclear_texture.h"


void nuclear_texture_create_from_data(nuclear_texture *texture, int width, int height, void *data)
{
	texture->width = width;
	texture->height = height;
	texture->channels = 4;
	glGenTextures(1, &texture->identifier);
	glBindTexture(GL_TEXTURE_2D, texture->identifier);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glBindTexture(GL_TEXTURE_2D, 0);
}
void nuclear_texture_create_from_egl_image(nuclear_texture *texture, int width, int height, EGLImage image)
{
	texture->width = width;
	texture->height = height;
	texture->channels = -1;
	glGenTextures (1, &texture->identifier);
	glBindTexture (GL_TEXTURE_2D, texture->identifier);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	c_glEGLImageTargetTexture2DOES (GL_TEXTURE_2D, image);
	glBindTexture (GL_TEXTURE_2D, 0);
}
void nuclear_texture_create_from_file(nuclear_texture *texture, const char *filename)
{
	//texture->identifier = SOIL_create_OGL_texture(
	//		SOIL_load_image(
	//				filename,
	//				&texture->width,
	//				&texture->height,
	//				&texture->channels,
	//				SOIL_LOAD_AUTO
	//			),
	//		texture->width,
	//		texture->height,
	//		texture->channels,
	//		SOIL_CREATE_NEW_ID,
	//		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	//	);
}
void nuclear_texture_draw(nuclear_texture *texture, int x, int y)
{
	if (!texture->identifier) return;
	
	GLfloat vertices[] = {
		x, y,
		x+texture->width, y,
		x+texture->width, y+texture->height,
		x, y+texture->height
	};
	GLint tex_coords[] = {
		0, 0,
		1, 0,
		1, 1,
		0, 1
	};
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture->identifier);
	//glVertexPointer(2, GL_FLOAT, 0, vertices);
	//glTexCoordPointer(2, GL_FLOAT, 0, tex_coords);
	
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	
	glBindTexture(GL_TEXTURE_2D, 0);
}
void nuclear_texture_delete(nuclear_texture* texture)
{
	if (texture->identifier)
		glDeleteTextures (1, &texture->identifier);
}
