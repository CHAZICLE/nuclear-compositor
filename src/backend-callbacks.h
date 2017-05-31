#ifndef __NUCLEAR_COMPOSITOR_BACKEND_CALLBACKS__
#define __NUCLEAR_COMPOSITOR_BACKEND_CALLBACKS__

#include "main.h"
#include "nuclear_utils.h"
#include "backend.h"

void handle_resize_event (nuclear_server *srv, int width, int height);
void handle_draw_event (nuclear_server *srv);
void handle_mouse_motion_event (nuclear_server *srv, int x, int y);
void handle_mouse_button_event (nuclear_server *srv, int button, int state);
void handle_key_event (nuclear_server *srv, int key, int state);
void handle_modifiers_changed(nuclear_server *srv, struct modifier_state new_state);

void do_backend_render(nuclear_server *srv);

extern struct backend_callbacks nuclear_backend_callbacks;

#endif
