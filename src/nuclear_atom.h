#ifndef __NUCLEAR_COMPOSITOR_NUCLEAR_ATOM_INCLUDED__
#define __NUCLEAR_COMPOSITOR_NUCLEAR_ATOM_INCLUDED__

typedef struct _nuclear_atom nuclear_atom;

#include <wayland-server.h>
#include "linmath.h"

#include "nuclear_server.h"
#include "nuclear_universe.h"
#include "nuclear_surface.h"

struct _nuclear_atom {
	nuclear_universe *universe;
	struct wl_list levels;
	nuclear_surface *active_surface;

	//struct modifier_state modifier_state;
	
	vec3 position;

	struct wl_list link;
};

void nuclear_atom_preinit();
void nuclear_atom_init(nuclear_atom *atm, nuclear_universe *unv, int x, int y, int z);
void nuclear_atom_render(nuclear_atom *atm, mat4x4 viewProjectionMatrix);
void nuclear_atom_destroy(nuclear_atom *atm);

#endif
