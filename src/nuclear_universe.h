#ifndef __NUCLEAR_COMPOSITOR_NUCLEAR_UNIVERSE_INCLUDED__
#define __NUCLEAR_COMPOSITOR_NUCLEAR_UNIVERSE_INCLUDED__

typedef struct _nuclear_universe nuclear_universe;

#include "nuclear_server.h"
#include "nuclear_atom.h"

#include <wayland-server.h>

struct _nuclear_universe {
	nuclear_server *server;
	nuclear_atom *active_atom;
	vec3 floating_position;
	struct wl_list atoms;

	float yaw; // 0 to 360
	float pitch; // -90 to 90

	struct wl_list link;
};

void nuclear_universe_preinit();
void nuclear_universe_init(nuclear_universe *unv, nuclear_server *srv);
void nuclear_universe_render(nuclear_universe *unv, mat4x4 projectionMatrix);
void nuclear_universe_destroy(nuclear_universe *unv);

#endif
