#ifndef __NUCLEAR_COMPOSITOR_NUCLEAR_SERVER_INCLUDED__
#define __NUCLEAR_COMPOSITOR_NUCLEAR_SERVER_INCLUDED__

typedef struct _nuclear_server nuclear_server;

#include <wayland-server.h>
#include "common/nuclear_client.h"

#include "nuclear_server.h"

struct _nuclear_server {
	struct wl_list universes;
	struct wl_list clients;
	struct wl_list renderers;

	struct wl_display *display;
};

void nuclear_server_init(nuclear_server *srv, bool enableOSVR);
nuclear_client *nuclear_server_get_client(nuclear_server *srv, struct wl_client *_client);
void nuclear_server_run(nuclear_server *srv);
void nuclear_server_destroy(nuclear_server *srv);

#endif
