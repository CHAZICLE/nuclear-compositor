#ifndef __NUCLEAR_COMPOSITOR_RENDER_NUCLEAR_RENDERER_INCLUDED__
#define __NUCLEAR_COMPOSITOR_RENDER_NUCLEAR_RENDERER_INCLUDED__

typedef struct _nuclear_renderer nuclear_renderer;

#include "nuclear_utils.h"
#include "common/nuclear_universe.h"
#include "common/nuclear_atom.h"
#include "common/nuclear_level.h"
#include "common/nuclear_surface.h"

struct _nuclear_renderer {

	// Backend
	EGLDisplay display;
	EGLContext context;
	EGLSurface surface;
	void *backend_data;

	void (*backend_handle_resize_event)(nuclear_renderer *rnd, int width, int height);
	void (*backend_handle_draw_event)(nuclear_renderer *rnd);
	void (*backend_handle_mouse_motion_event)(nuclear_renderer *rnd, int x, int y);
	void (*backend_handle_mouse_button_event)(nuclear_renderer *rnd, int button, int state);
	void (*backend_handle_key_event)(nuclear_renderer *rnd, int key, int state);
	void (*backend_handle_modifiers_event)(nuclear_renderer *rnd, struct modifier_state modifier_state);

	// Renderer
	void *data;
	nuclear_universe *universe;
	vec3 position;

	nuclear_atom *active_atom;
	nuclear_level *active_level;
	nuclear_surface *active_surface;

	bool redraw;

	bool (*render)(nuclear_renderer *rnd);
	bool (*update)(nuclear_renderer *rnd);

	struct wl_list next;
};

void nuclear_renderer_glinit();
void nuclear_renderer_render_universe(nuclear_renderer *render, nuclear_universe *unv, mat4x4 projectMatrix, mat4x4 viewMatrix);
void nuclear_renderer_render_atom(nuclear_renderer *render, nuclear_atom *atm, mat4x4 projectMatrix, mat4x4 viewMatrix);
void nuclear_renderer_render_level(nuclear_renderer *rnd, nuclear_level *lvl, mat4x4 projectMatrix, mat4x4 viewMatrix, mat4x4 modelMatrix);
void nuclear_renderer_render_surface(nuclear_renderer *rnd, nuclear_surface *srf, mat4x4 projectionMatrix, mat4x4 viewMatrix, mat4x4 modelMatrix);

#endif
