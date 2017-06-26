#include <stdlib.h>
#include "backend.h"
#include "nuclear_utils.h"

#include "nuclear_screen.h"

static float translate_multiplier = 1.f;
static float ROTATION_MULTIPLIER = 5000.f;

bool nuclear_screen_init(nuclear_renderer *rnd, nuclear_server *srv)
{
	rnd->backend_handle_resize_event = &nuclear_screen_handle_resize_event;
	rnd->backend_handle_draw_event = &nuclear_screen_handle_draw_event;
	rnd->backend_handle_mouse_motion_event = &nuclear_screen_handle_mouse_motion_event;
	rnd->backend_handle_mouse_button_event = &nuclear_screen_handle_mouse_button_event;
	rnd->backend_handle_key_event = &nuclear_screen_handle_key_event;
	rnd->backend_handle_modifiers_event = &nuclear_screen_handle_modifiers_changed;

	rnd->render = &nuclear_screen_render;
	rnd->update = &nuclear_screen_update;

	// Set up renderer
	nuclear_screen_renderer *srnd = malloc(sizeof(nuclear_screen_renderer));
	mat4x4_identity(srnd->projectionMatrix);
	mat4x4_perspective(srnd->projectionMatrix, (75.f)/(180.f/3.1415f), 16.0f/9.0f, 0.1f, 1000.f);
	srnd->pitch = 0.f;
	srnd->yaw = 0.f;
	rnd->data = srnd;

	return true;
}
static bool ctl_left,ctl_right,ctl_up,ctl_down,ctl_forward,ctl_backward;
#define NCS_GET_forward vec3 forward = { \
			cos(srnd->pitch) * sin(srnd->yaw), \
			cos(srnd->pitch) * cos(srnd->yaw), \
			sin(srnd->pitch) \
		};
#define NCS_GET_right vec3 right = { \
			sin(srnd->yaw + 3.14f/2.0f), \
			cos(srnd->yaw + 3.14f/2.0f), \
			0, \
		};
#define NCS_GET_up vec3 up; \
		NCS_GET_forward \
		NCS_GET_right \
		vec3_mul_cross(up, right, forward);
#define NCS_ADDCTL(ctl, op, vec) if(ctl_##ctl) \
	{ \
		NCS_GET_##vec \
		vec3_scale(vec, vec, translate_multiplier); \
		vec3_##op(rnd->position, rnd->position, vec); \
	}
bool nuclear_screen_render(nuclear_renderer *rnd)
{
	return nuclear_screen_render_universe(rnd, rnd->universe);
}
bool nuclear_screen_update(nuclear_renderer *rnd)
{
	nuclear_screen_renderer *srnd = (nuclear_screen_renderer *)rnd->data;

	NCS_ADDCTL(right,add,right)
	NCS_ADDCTL(left,sub,right)
	NCS_ADDCTL(up,add,up)
	NCS_ADDCTL(down,sub,up)
	NCS_ADDCTL(forward,add,forward)
	NCS_ADDCTL(backward,sub,forward)

	return true;
}
bool nuclear_screen_render_universe(nuclear_renderer *rnd, nuclear_universe *unv)
{
	nuclear_screen_renderer *srnd = (nuclear_screen_renderer *)rnd->data;
	glClearColor(0.0f, 0.0f, 0.05f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// Create view matrix
	mat4x4 viewMatrix;
	mat4x4_identity(viewMatrix);
	vec3 direction = {
		cos(srnd->pitch) * sin(srnd->yaw),
		cos(srnd->pitch) * cos(srnd->yaw),
		sin(srnd->pitch)
	};
	vec3 right = {
		sin(srnd->yaw + 3.14f/2.0f),
		cos(srnd->yaw + 3.14f/2.0f),
		0,
	};
	vec3 up;
	vec3_mul_cross(up, right, direction);

	vec3 lookAt;
	vec3_add(lookAt, rnd->position, direction);
	mat4x4_look_at(viewMatrix, rnd->position, lookAt, up);

	nuclear_renderer_render_universe(rnd, unv, srnd->projectionMatrix, viewMatrix);

	backend_swap_buffers(rnd);
	return true;
}


void nuclear_screen_handle_resize_event(nuclear_renderer *rnd, int width, int height)
{
	fprintf(stderr, "%s: (%d, %d)\n", __func__, width, height);
	nuclear_screen_renderer *srnd = (nuclear_screen_renderer *)rnd->data;

	glViewport(0, 0, width, height);
	mat4x4_perspective(srnd->projectionMatrix, radians(75.f), (float)width/(float)height, 0.1f, 1000.f);
}
void nuclear_screen_handle_draw_event(nuclear_renderer *rnd)
{
	fprintf(stderr, "%s\n", __func__);
}
void nuclear_screen_handle_mouse_motion_event(nuclear_renderer *rnd, int dx, int dy)
{
	fprintf(stderr, "%s: (%d, %d)\n", __func__, dx, dy);
	nuclear_screen_renderer *srnd = (nuclear_screen_renderer *)rnd->data;

	srnd->pitch -= dy/(180/3.14)/((float)backend_get_timestamp(rnd)/1000)*ROTATION_MULTIPLIER;
	srnd->yaw += dx/(180/3.14)/((float)backend_get_timestamp(rnd)/1000)*ROTATION_MULTIPLIER;
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
void nuclear_screen_handle_mouse_button_event(nuclear_renderer *rnd, int button, int state)
{
	fprintf(stderr, "%s: (%d, %d)\n", __func__, button, state);
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
void nuclear_screen_handle_key_event(nuclear_renderer *rnd, int key, int state)
{
	fprintf(stderr, "%s: (%d, %d)\n", __func__, key, state);
	//if (!srv->active_surface || !srv->active_surface->client->keyboard) return;
	//wl_keyboard_send_key (srv->active_surface->client->keyboard, 0, backend_get_timestamp(), key, state);
	SETUPKEY(30, ctl_left)//left
	SETUPKEY(32, ctl_right)//right
	SETUPKEY(18, ctl_up)//down
	SETUPKEY(16, ctl_down)//up
	SETUPKEY(17, ctl_forward)//forward
	SETUPKEY(31, ctl_backward)//back
}
void nuclear_screen_handle_modifiers_changed(nuclear_renderer *rnd, struct modifier_state new_state)
{
	fprintf(stderr, "handle_modifiers_changed: group=%d, locked=%d, latched=%d, depressed=%d\n",
			new_state.group,
			new_state.locked,
			new_state.latched,
			new_state.depressed
			);
	if(new_state.depressed&1)
	{
		translate_multiplier = 5.f;
	}
	else
	{
		translate_multiplier = 1.f;
	}
	//if (new_state.depressed == srv->modifier_state.depressed && new_state.latched == srv->modifier_state.latched && new_state.locked == srv->modifier_state.locked && new_state.group == srv->modifier_state.group)
	//	return;
	//puts("OXTO");
	//srv->modifier_state = new_state;
	//if (srv->active_surface && srv->active_surface->client->keyboard)
	//	wl_keyboard_send_modifiers (srv->active_surface->client->keyboard, 0, srv->modifier_state.depressed, srv->modifier_state.latched, srv->modifier_state.locked, srv->modifier_state.group);
}
