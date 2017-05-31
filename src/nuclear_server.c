#include "backend.h"
#include "backend-callbacks.h"
#include "nuclear_impl_compositor.h"
#include "nuclear_seat.h"
#include "nuclear_universe.h"
#include <stdlib.h>

#include "nuclear_server.h"

void nuclear_server_init(nuclear_server *srv)
{
	backend_init(&nuclear_backend_callbacks, srv);

	srv->display = wl_display_create();
	wl_display_add_socket_auto(srv->display);
	wl_global_create(srv->display, &wl_compositor_interface, 3, NULL, &nuclear_impl_compositor_bind);
	wl_global_create(srv->display, &wl_seat_interface, 1, NULL, &nuclear_seat_bind);
	//wl_global_create(display, &wl_shell_interface, 1, NULL, &shell_bind);
	//wl_global_create(display, &xdg_shell_interface, 1, NULL, &xdg_shell_bind);
	eglBindWaylandDisplayWL (backend_get_egl_display(), srv->display);
	wl_display_init_shm(srv->display);
	wl_list_init(&srv->universes);
	wl_list_init(&srv->clients);

	srv->redraw = 1;

	srv->active_universe = malloc(sizeof(nuclear_universe));
	nuclear_universe_init(srv->active_universe, srv);
	wl_list_insert(&srv->universes, &srv->active_universe->link);

	nuclear_universe_preinit();
}
void nuclear_server_run(nuclear_server *srv)
{
	struct wl_event_loop *event_loop = wl_display_get_event_loop(srv->display);
	int wayland_fd = wl_event_loop_get_fd(event_loop);

	// Create projection matrix
	mat4x4 projectionMatrix;
	mat4x4_perspective(projectionMatrix, (75.f)/(180.f/3.1415f), 16.0f/9.0f, 0.1f, 1000.f);

	while (1) {
		wl_event_loop_dispatch(event_loop, 0);
		backend_dispatch_nonblocking();
		wl_display_flush_clients(srv->display);
		if(srv->redraw) {
			glClearColor(0.0f, 0.0f, 0.05f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
			srv->redraw = 0;

			nuclear_universe *universe;
			wl_list_for_each_reverse (universe, &srv->universes, link) {
				nuclear_universe_render(universe, projectionMatrix);
			}


			backend_swap_buffers();
		}
		else {
			backend_wait_for_events(wayland_fd);
		}
		do_backend_render(srv);
	}
}
nuclear_client *nuclear_server_get_client(nuclear_server *srv, struct wl_client *_client)
{
	nuclear_client *nuclear_client;
	wl_list_for_each(nuclear_client, &srv->clients, link) {
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
