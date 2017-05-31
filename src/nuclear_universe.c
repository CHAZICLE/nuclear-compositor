#include "nuclear_atom.h"
#include <stdlib.h>

#include "nuclear_universe.h"

void nuclear_universe_preinit()
{
	nuclear_atom_preinit();
}
void nuclear_universe_init(nuclear_universe *unv, nuclear_server *srv)
{
	unv->server = srv;
	wl_list_init(&unv->atoms);
	unv->floating_position[0] = 0;
	unv->floating_position[1] = 0;
	unv->floating_position[2] = 0;

	nuclear_atom *atoms = malloc(sizeof(nuclear_atom)*3);
	for(int i=0;i<3;i++)
	{
		nuclear_atom_init(&atoms[i], unv, 0.f, 100.f*(i+2), 0.f);
		wl_list_insert(&unv->atoms, &atoms[i].link);
	}
	unv->active_atom = &atoms[0];
}
void nuclear_universe_render(nuclear_universe *unv, mat4x4 projectionMatrix)
{
	// Create view matrix
	mat4x4 viewMatrix;
	vec3 direction = {
		cos(unv->pitch) * sin(unv->yaw),
		cos(unv->pitch) * cos(unv->yaw),
		sin(unv->pitch)
	};
	vec3 right = {
		sin(unv->yaw + 3.14f/2.0f),
		cos(unv->yaw + 3.14f/2.0f),
		0,
	};
	vec3 up;
	vec3_mul_cross(up, right, direction);
	vec3 lookAt;
	vec3_add(lookAt, unv->floating_position, direction);
	mat4x4_look_at(viewMatrix, unv->floating_position, lookAt, up);

	mat4x4 viewProjectionMatrix;
	mat4x4_mul(viewProjectionMatrix, projectionMatrix, viewMatrix);

	nuclear_atom *atom;
	wl_list_for_each_reverse (atom, &unv->atoms, link) {
		nuclear_atom_render(atom, viewProjectionMatrix);
	}
}
void nuclear_universe_destroy(nuclear_universe *unv)
{
	nuclear_atom *atom;
	wl_list_for_each_reverse(atom, &unv->atoms, link) {
		nuclear_atom_destroy(atom);
	}
}
