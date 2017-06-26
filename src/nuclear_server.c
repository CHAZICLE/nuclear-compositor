#include "backend.h"
#include "nuclear_utils.h"
#include "wayland/nuclear_impl_compositor.h"
#include "nuclear_seat.h"
#include "common/nuclear_universe.h"
#include <stdlib.h>
#include "render/nuclear_osvr.h"
#include "render/nuclear_screen.h"
#include "render/nuclear_renderer.h"

#include "nuclear_server.h"

void nuclear_server_init(nuclear_server *srv, bool enableOSVR)
{
	wl_list_init(&srv->universes);
	wl_list_init(&srv->clients);
	wl_list_init(&srv->renderers);

	// Create default renderer
	nuclear_renderer *rnd = malloc(sizeof(nuclear_renderer));

	// Setup the EGL context
	backend_init(rnd);

	// Setup the wayland server
	srv->display = wl_display_create();
	wl_display_add_socket_auto(srv->display);
	wl_global_create(srv->display, &wl_compositor_interface, 3, NULL, &nuclear_impl_compositor_bind);
	wl_global_create(srv->display, &wl_seat_interface, 1, NULL, &nuclear_seat_bind);
	//wl_global_create(display, &wl_shell_interface, 1, NULL, &shell_bind);
	//wl_global_create(display, &xdg_shell_interface, 1, NULL, &xdg_shell_bind);
	eglBindWaylandDisplayWL(rnd->display, srv->display);
	wl_display_init_shm(srv->display);

	nuclear_renderer_glinit();

	// Create the universe (heh)
	nuclear_universe *unv = malloc(sizeof(nuclear_universe));
	nuclear_universe_init(unv, srv);
	rnd->universe = unv;
	wl_list_insert(&srv->universes, &unv->next);

	// Init renderer
#ifdef NUCLEAR_OSVR
	if(enableOSVR)
		nuclear_osvr_init(rnd, srv);
	else
#endif
		nuclear_screen_init(rnd, srv);
	wl_list_insert(&srv->renderers, &rnd->next);

	fprintf(stderr, "Init complete\n");
}
void nuclear_server_run(nuclear_server *srv)
{
	struct wl_event_loop *event_loop = wl_display_get_event_loop(srv->display);
	int wayland_fd = wl_event_loop_get_fd(event_loop);

	bool redraw;
	while(true)
	{
		nuclear_renderer *rnd;
		redraw = false;
		wl_list_for_each(rnd, &srv->renderers, next)
		{
			wl_display_flush_clients(srv->display);
			backend_dispatch_nonblocking(rnd);
			wl_event_loop_dispatch(event_loop, 0);
			rnd->update(rnd);
			if(rnd->render(rnd))
				redraw = true;
			if(!redraw)
			{
				backend_wait_for_events(rnd, wayland_fd);
			}
		}
	}
}
nuclear_client *nuclear_server_get_client(nuclear_server *srv, struct wl_client *_client)
{
	nuclear_client *nuclear_client;
	wl_list_for_each(nuclear_client, &srv->clients, link)
	{
		if(nuclear_client->client == _client)
			return nuclear_client;
	}
	nuclear_client = calloc((size_t)1, sizeof(nuclear_client));
	nuclear_client->client = _client;
	wl_list_insert(&srv->clients, &nuclear_client->link);
	return nuclear_client;
}
void nuclear_server_destroy(nuclear_server *srv)
{
	wl_display_destroy (srv->display);
}
