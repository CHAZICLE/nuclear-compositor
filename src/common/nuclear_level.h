#ifndef __NUCLEAR_COMPOSITOR_NUCLEAR_LEVEL_INCLUDED__
#define __NUCLEAR_COMPOSITOR_NUCLEAR_LEVEL_INCLUDED__

typedef struct _nuclear_level nuclear_level;

#include "nuclear_server.h"
#include "nuclear_universe.h"
#include "nuclear_atom.h"

struct _nuclear_level {
	nuclear_atom *atom;
	struct wl_list surfaces;

	float radius;

	struct wl_list next;
};

void nuclear_level_init(nuclear_level *lvl, nuclear_atom *atm, float radius);
void nuclear_level_destroy(nuclear_level *lvl);

#endif
