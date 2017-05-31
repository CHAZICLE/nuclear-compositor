#ifndef __NUCLEAR_COMPOSITOR_NUCLEAR_TEXTURE_INCLUDED__
#define __NUCLEAR_COMPOSITOR_NUCLEAR_TEXTURE_INCLUDED__

#include "nuclear_utils.h"

struct _nuclear_texture {
	GLuint identifier;
	int width, height, channels;
};
typedef struct _nuclear_texture nuclear_texture;

void nuclear_texture_create_from_data(nuclear_texture *texture, int width, int height, void *data);
void nuclear_texture_create_from_egl_image(nuclear_texture *texture, int width, int height, EGLImage image);
void nuclear_texture_create_from_file(nuclear_texture *texture, const char *filename);
void nuclear_texture_render(nuclear_texture *texture, int x, int y);
void nuclear_texture_delete(nuclear_texture* texture);

#endif
