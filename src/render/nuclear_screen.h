#ifndef __NUCLEAR_COMPOSITOR_RENDER_NUCLEAR_SCREEN_INCLUDED__
#define __NUCLEAR_COMPOSITOR_RENDER_NUCLEAR_SCREEN_INCLUDED__

typedef struct _nuclear_screen_renderer nuclear_screen_renderer;

#include "nuclear_renderer.h"
#include "nuclear_server.h"
#include "common/nuclear_universe.h"
#include "common/nuclear_atom.h"
#include "common/nuclear_level.h"
#include "common/nuclear_surface.h"

struct _nuclear_screen_renderer {
	mat4x4 projectionMatrix;
	float pitch,yaw;
};

bool nuclear_screen_init(nuclear_renderer *rnd, nuclear_server *srv);
bool nuclear_screen_render(nuclear_renderer *rnd);
bool nuclear_screen_update(nuclear_renderer *rnd);
bool nuclear_screen_render_universe(nuclear_renderer *rnd, nuclear_universe *unv);

void nuclear_screen_handle_resize_event(nuclear_renderer *rnd, int width, int height);
void nuclear_screen_handle_draw_event(nuclear_renderer *rnd);
void nuclear_screen_handle_mouse_motion_event(nuclear_renderer *rnd, int dx, int dy);
void nuclear_screen_handle_mouse_button_event(nuclear_renderer *rnd, int button, int state);
void nuclear_screen_handle_key_event(nuclear_renderer *rnd, int key, int state);
void nuclear_screen_handle_modifiers_changed(nuclear_renderer *rnd, struct modifier_state new_state);

#endif
