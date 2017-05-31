#ifndef __NUCLEAR_COMPOSITOR_NUCLEAR_COMPOSITOR_INCLUDED__
#define __NUCLEAR_COMPOSITOR_NUCLEAR_COMPOSITOR_INCLUDED__

#include <wayland-server.h>

void nuclear_impl_compositor_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id);
static struct wl_compositor_interface nuclear_impl_compositor_interface;
static void nuclear_impl_compositor_create_surface(struct wl_client *client,
		       struct wl_resource *resource,
		       uint32_t id);
static void nuclear_impl_compositor_create_region(struct wl_client *client,
		      struct wl_resource *resource,
		      uint32_t id);
static void nuclear_impl_compositor_delete_surface(struct wl_resource *resource);

static struct wl_surface_interface nuclear_impl_surface_interface;
static struct wl_region_interface nuclear_impl_region_interface;

#endif
