#ifndef __NUCLEAR_COMPOSITOR_NUCLEAR_COMPOSITOR_INCLUDED__
#define __NUCLEAR_COMPOSITOR_NUCLEAR_COMPOSITOR_INCLUDED__

#include <wayland-server.h>

void nuclear_impl_compositor_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id);
void nuclear_impl_region_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id);

#endif
