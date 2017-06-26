#ifndef __NUCLEAR_COMPOSITOR_NUCLEAR_CLIENT_INCLUDED__
#define __NUCLEAR_COMPOSITOR_NUCLEAR_CLIENT_INCLUDED__

#include <wayland-server.h>

struct _nuclear_client
{
	struct wl_client *client;
	struct wl_resource *pointer;
	struct wl_resource *keyboard;
	struct wl_list link;
};
typedef struct _nuclear_client nuclear_client;

#endif
