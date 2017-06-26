#ifndef __NUCLEAR_COMPOSITOR_NUCLEAR_UNIVERSE_INCLUDED__
#define __NUCLEAR_COMPOSITOR_NUCLEAR_UNIVERSE_INCLUDED__

typedef struct _nuclear_universe nuclear_universe;

#include "nuclear_server.h"
#include "nuclear_atom.h"
#define OSVR_RM_USE_OPENGLES20

#include <wayland-server.h>

struct _nuclear_universe {
	nuclear_server *server;
	struct wl_list atoms;

	struct wl_list next;
};

void nuclear_universe_init(nuclear_universe *unv, nuclear_server *srv);
void nuclear_universe_destroy(nuclear_universe *unv);

#endif
