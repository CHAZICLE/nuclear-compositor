#ifndef __NUCLEAR_COMPOSITOR_NUCLEAR_SERVER_INCLUDED__
#define __NUCLEAR_COMPOSITOR_NUCLEAR_SERVER_INCLUDED__

typedef struct _nuclear_server nuclear_server;

#include <wayland-server.h>
#include "nuclear_universe.h"
#include "nuclear_server.h"
#include "nuclear_client.h"

struct _nuclear_server {
	nuclear_universe *active_universe;
	struct wl_list universes;
	struct wl_list clients;

	struct wl_display *display;

	int redraw;
};

void nuclear_server_init(nuclear_server *srv);
nuclear_client *nuclear_server_get_client(nuclear_server *srv, struct wl_client *_client);
void nuclear_server_run(nuclear_server *srv);
void nuclear_server_destroy(nuclear_server *srv);

#endif
