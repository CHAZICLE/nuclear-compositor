#ifndef __NUCLEAR_COMPOSITOR_NUCLEAR_SURFACE_INCLUDED__
#define __NUCLEAR_COMPOSITOR_NUCLEAR_SURFACE_INCLUDED__

typedef struct _nuclear_surface nuclear_surface;

#include <wayland-server.h>
#include "nuclear_server.h"
#include "nuclear_universe.h"
#include "nuclear_atom.h"
#include "nuclear_level.h"

#include "nuclear_client.h"

struct _nuclear_surface {
	nuclear_server *server;
	nuclear_universe *universe;
	nuclear_atom *atom;
	nuclear_level *level;

	nuclear_client *client;

	int level_pitch,level_yaw;
	int x,y,z;

	struct wl_resource *surface;
	struct wl_resource *xdg_surface;
	struct wl_resource *buffer;
	struct wl_resource *frame_callback;
	//struct texture texture;

	struct wl_list link;
};

void nuclear_surface_init(nuclear_surface *lvl);
void nuclear_surface_render(nuclear_surface *lvl);
void nuclear_surface_destroy(nuclear_surface *lvl);

#endif
