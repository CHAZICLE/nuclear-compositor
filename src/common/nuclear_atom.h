#ifndef __NUCLEAR_COMPOSITOR_NUCLEAR_ATOM_INCLUDED__
#define __NUCLEAR_COMPOSITOR_NUCLEAR_ATOM_INCLUDED__

typedef struct _nuclear_atom nuclear_atom;

#include "nuclear_universe.h"
#include "nuclear_atom.h"
#include "linmath.h"

struct _nuclear_atom {
	nuclear_universe *universe;
	struct wl_list levels;

	vec3 position;

	struct wl_list next;
};

void nuclear_atom_init(nuclear_atom *atm, nuclear_universe *unv, int x, int y, int z);
void nuclear_atom_destroy(nuclear_atom *atm);

#endif
