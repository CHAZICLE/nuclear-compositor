#include "nuclear_level.h"
#include <string.h>
#include "backend.h"
#include "nuclear_utils.h"
#include <stdlib.h>

#include "nuclear_atom.h"

void nuclear_atom_preinit()
{
	nuclear_level_preinit();
}
void nuclear_atom_init(nuclear_atom *atm, nuclear_universe *unv, int x, int y, int z)
{
	atm->universe = unv;
	atm->position[0] = x;
	atm->position[1] = y;
	atm->position[2] = z;

	wl_list_init(&atm->levels);
	nuclear_level *level = malloc(sizeof(nuclear_level)*3);
	for(int i=0;i<1;i++)
	{
		nuclear_level_init(&level[i], atm, 20.f+5.f*(i+1));
		wl_list_insert(&atm->levels, &level[i].link);
	}
}
void nuclear_atom_render(nuclear_atom *atm, mat4x4 viewProjectionMatrix)
{
	mat4x4 modelMatrix;
	mat4x4_translate(modelMatrix, atm->position[0], atm->position[1], atm->position[2]);
	mat4x4 modelViewProjectionMatrix;
	mat4x4_mul(modelViewProjectionMatrix, viewProjectionMatrix, modelMatrix);

	//// Render all the levels
	nuclear_level *level;
	wl_list_for_each_reverse (level, &atm->levels, link) {
		nuclear_level_render(level, modelViewProjectionMatrix);
	}
}
// void deactivate_surface (nuclear_server *srv, nuclear_surface *surface) {
// 	if (surface->client->keyboard) wl_keyboard_send_leave (surface->client->keyboard, 0, surface->surface);
// 	struct wl_array state_array;
// 	wl_array_init (&state_array);
// 	xdg_surface_send_configure (surface->xdg_surface, 0, 0, &state_array, 0);
// }
// void activate_surface (nuclear_server *srv, nuclear_surface *surface) {
// 	wl_list_remove (&surface->link);
// 	wl_list_insert (&srv->surfaces, &surface->link);
// 	struct wl_array array;
// 	wl_array_init (&array);
// 	if (surface->client->keyboard) {
// 		wl_keyboard_send_enter (surface->client->keyboard, 0, surface->surface, &array);
// 		wl_keyboard_send_modifiers (surface->client->keyboard, 0, srv->modifier_state.depressed, srv->modifier_state.latched, srv->modifier_state.locked, srv->modifier_state.group);
// 	}
// 	int32_t *states = wl_array_add (&array, sizeof(int32_t));
// 	states[0] = XDG_SURFACE_STATE_ACTIVATED;
// 	xdg_surface_send_configure (surface->xdg_surface, 0, 0, &array, 0);
// }
// static void delete_surface (nuclear_server *srv, struct wl_resource *resource) {
// 	nuclear_surface *surface = wl_resource_get_user_data (resource);
// 	wl_list_remove (&surface->link);
// 	if (surface == srv->active_surface)
// 		srv->active_surface = NULL;
// 	if (surface == srv->pointer_surface)
// 		srv->pointer_surface = NULL;
// 	free (surface);
// 	srv->redraw_needed = 1;
// }
void nuclear_atom_destroy(nuclear_atom *atm)
{
	
}
