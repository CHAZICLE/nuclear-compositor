#include "main.h"
#include "nuclear_surface.h"

#include "backend-callbacks.h"

static int window_active = 1;

void handle_resize_event(nuclear_server *srv, int width, int height) {
	glViewport (0, 0, width, height);
	//glMatrixMode (GL_PROJECTION);
	//glLoadIdentity ();
	//glOrtho (0, width, height, 0, 1, -1);
	//glMatrixMode (GL_MODELVIEW);
	//
	//glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable (GL_BLEND);
}
void handle_draw_event (nuclear_server *srv) {
	srv->redraw = 1;
}
float vin = 100.f;
void handle_mouse_motion_event (nuclear_server *srv, int dx, int dy)
{
	fprintf(stderr, "handle_mouse_motion_event: (%d, %d)\n", dx, dy);
	if(window_active)
	{
		srv->active_universe->pitch -= dy/(180/3.14)/((float)backend_get_timestamp()/1000)*vin;
		srv->active_universe->yaw += dx/(180/3.14)/((float)backend_get_timestamp()/1000)*vin;
	}
	srv->redraw = 1;
	
//	// get surface under the pointer
//	nuclear_surface *next_pointer_surface = NULL;
//	nuclear_surface *s;
//	wl_list_for_each_reverse (s, &srv->surfaces, link) {
//		if (!s->xdg_surface) continue;
//		if (x > s->x && y > s->y && x < s->x + s->texture.width && y < s->y + s->texture.height)
//			next_pointer_surface = s;
//	}
//	// pointer enter and leave
//	if (next_pointer_surface != srv->pointer_surface) {
//		if (srv->pointer_surface && srv->pointer_surface->client->pointer)
//			wl_pointer_send_leave (srv->pointer_surface->client->pointer, 0, srv->pointer_surface->surface);
//		srv->pointer_surface = next_pointer_surface;
//		if (srv->pointer_surface && srv->pointer_surface->client->pointer)
//			wl_pointer_send_enter (srv->pointer_surface->client->pointer, 0, srv->pointer_surface->surface, x, y);
//	}
//	if (!srv->pointer_surface || !srv->pointer_surface->client->pointer) return;
//	wl_fixed_t surface_x = wl_fixed_from_double (x - srv->pointer_surface->x);
//	wl_fixed_t surface_y = wl_fixed_from_double (y - srv->pointer_surface->y);
//	wl_pointer_send_motion(srv->pointer_surface->client->pointer, backend_get_timestamp(), surface_x, surface_y);
}
void handle_mouse_button_event (nuclear_server *srv, int button, int state) {

	//if (srv->moving_surface && state == WL_POINTER_BUTTON_STATE_RELEASED) {
	//	srv->moving_surface->x = srv->pointer_x + srv->moving_surface->x;
	//	srv->moving_surface->y = srv->pointer_y + srv->moving_surface->y;
	//	srv->moving_surface = NULL;
	//}
	//if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
	//	if (srv->pointer_surface != srv->active_surface) {
	//		if (srv->active_surface) deactivate_surface (srv, srv->active_surface);
	//		srv->active_surface = srv->pointer_surface;
	//		if (srv->active_surface) activate_surface (srv, srv->active_surface);
	//	}
	//}
	//if (!srv->pointer_surface || !srv->pointer_surface->client->pointer) return;
	//wl_pointer_send_button (srv->pointer_surface->client->pointer, 0, backend_get_timestamp(), button, state);
}

#define SETUPKEY(k,ctl) do { \
		if(key==(k)) \
		{ \
			if(state==1) \
			{ \
				(ctl) = true; \
			} \
			else if(state==0) \
			{ \
				(ctl) = false; \
			} \
		} \
	} while(0);
bool ctl_left,ctl_right,ctl_up,ctl_down,ctl_forward,ctl_backward,ctl_speed;
void handle_key_event (nuclear_server *srv, int key, int state) {
	fprintf(stderr, "handle_key_event: key=%d state=%d\n", key, state);
	//if (!srv->active_surface || !srv->active_surface->client->keyboard) return;
	//wl_keyboard_send_key (srv->active_surface->client->keyboard, 0, backend_get_timestamp(), key, state);
	SETUPKEY(30, ctl_left)//left
	SETUPKEY(32, ctl_right)//right
	SETUPKEY(18, ctl_up)//down
	SETUPKEY(16, ctl_down)//up
	SETUPKEY(17, ctl_forward)//forward
	SETUPKEY(31, ctl_backward)//back
}
#define GET_forward vec3 forward = { \
			cos(unv->pitch) * sin(unv->yaw), \
			cos(unv->pitch) * cos(unv->yaw), \
			sin(unv->pitch) \
		};
#define GET_right vec3 right = { \
			sin(unv->yaw + 3.14f/2.0f), \
			cos(unv->yaw + 3.14f/2.0f), \
			0, \
		};
#define GET_up vec3 up; \
		GET_forward \
		GET_right \
		vec3_mul_cross(up, right, forward);
#define ADDCTL(ctl, op, vec) if(ctl_##ctl) \
	{ \
		GET_##vec \
		vec3_scale(vec, vec, vax); \
		vec3_##op(srv->active_universe->floating_position, srv->active_universe->floating_position, vec); \
		srv->redraw = 1; \
	}
float vax = 1.f;
void do_backend_render(nuclear_server *srv)
{
	nuclear_universe *unv = srv->active_universe;
	ADDCTL(right,add,right)
	ADDCTL(left,sub,right)
	ADDCTL(up,add,up)
	ADDCTL(down,sub,up)
	ADDCTL(forward,add,forward)
	ADDCTL(backward,sub,forward)
}
void handle_modifiers_changed(nuclear_server *srv, struct modifier_state new_state) {
	fprintf(stderr, "handle_modifiers_changed: group=%d, locked=%d, latched=%d, depressed=%d\n",
			new_state.group,
			new_state.locked,
			new_state.latched,
			new_state.depressed
			);
	if(new_state.depressed&1)
	{
		vax = 5.f;
	}
	else
	{
		vax = 1.f;
	}
	//puts("YO");
	//if (new_state.depressed == srv->modifier_state.depressed && new_state.latched == srv->modifier_state.latched && new_state.locked == srv->modifier_state.locked && new_state.group == srv->modifier_state.group)
	//	return;
	//puts("OXTO");
	//srv->modifier_state = new_state;
	//if (srv->active_surface && srv->active_surface->client->keyboard)
	//	wl_keyboard_send_modifiers (srv->active_surface->client->keyboard, 0, srv->modifier_state.depressed, srv->modifier_state.latched, srv->modifier_state.locked, srv->modifier_state.group);
}
struct backend_callbacks nuclear_backend_callbacks = {0, &handle_resize_event, &handle_draw_event, &handle_mouse_motion_event, &handle_mouse_button_event, &handle_key_event, &handle_modifiers_changed};
