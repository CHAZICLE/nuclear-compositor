#include <wayland-server.h>
#include <stdio.h>
#include "main.h"
#include "nuclear_surface.h"

#include "nuclear_impl_compositor.h"

// wl compositor
void nuclear_impl_compositor_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
	struct wl_resource *resource = wl_resource_create (client, &wl_compositor_interface, 1, id);
	wl_resource_set_implementation (resource, &nuclear_impl_compositor_interface, NULL, NULL);
}
static struct wl_compositor_interface nuclear_impl_compositor_interface = {
	&nuclear_impl_compositor_create_surface,
	&nuclear_impl_compositor_create_region,
};
static void nuclear_impl_compositor_create_surface(struct wl_client *client,
		       struct wl_resource *resource,
		       uint32_t id)
{
	//printf("nuclear_compositor_create_surface\n");
	//nuclear_surface *surface = calloc(1, sizeof(nuclear_surface));
	//surface->surface = wl_resource_create (client, &wl_surface_interface, 3, id);
	//wl_resource_set_implementation (surface->surface, &nuclear_impl_surface_interface, surface, &nuclear_impl_compositor_delete_surface);
	//surface->client = get_client (client);
	//wl_list_insert (&surfaces, &surface->link);
}
static void nuclear_impl_compositor_create_region(struct wl_client *client,
		      struct wl_resource *resource,
		      uint32_t id)
{
	printf("nuclear_compositor_create_region\n");
	struct wl_resource *region = wl_resource_create (client, &wl_region_interface, 1, id);
	wl_resource_set_implementation (region, &nuclear_impl_region_interface, NULL, NULL);
}
static void nuclear_impl_compositor_delete_surface(struct wl_resource *resource)
{
	
}

// wl surface
static void nuclear_impl_surface_destroy(struct wl_client *client,
		struct wl_resource *resource)
{
	
	printf("nuclear_surface_destroy\n");
}
static void nuclear_impl_surface_attach(struct wl_client *client,
	       struct wl_resource *resource,
	       struct wl_resource *buffer,
	       int32_t x,
	       int32_t y)
{
	printf("nuclear_surface_attach\n");
	struct surface *surface = wl_resource_get_user_data (resource);
	//surface->buffer = buffer;
}
static void nuclear_impl_surface_damage(struct wl_client *client,
	       struct wl_resource *resource,
	       int32_t x,
	       int32_t y,
	       int32_t width,
	       int32_t height)
{
	printf("nuclear_surface_damage\n");
}
static void nuclear_impl_surface_frame(struct wl_client *client,
	      struct wl_resource *resource,
	      uint32_t callback)
{
	//printf("nuclear_surface_frame\n");
	//struct surface *surface = wl_resource_get_user_data (resource);
	//surface->frame_callback = wl_resource_create (client, &wl_callback_interface, 1, callback);
}
static void nuclear_impl_surface_set_opaque_region(struct wl_client *client,
			  struct wl_resource *resource,
			  struct wl_resource *region)
{
	printf("nuclear_surface_set_opaque_region\n");
}
static void nuclear_impl_surface_set_input_region(struct wl_client *client,
			 struct wl_resource *resource,
			 struct wl_resource *region)
{
	printf("nuclear_surface_set_input_region\n");
}
static void nuclear_impl_surface_commit(struct wl_client *client,
	       struct wl_resource *resource)
{
	//printf("nuclear_surface_commit\n");
	//struct surface *surface = wl_resource_get_user_data (resource);
	//EGLint texture_format;
	//if (eglQueryWaylandBufferWL (backend_get_egl_display(), surface->buffer, EGL_TEXTURE_FORMAT, &texture_format)) {
	//	EGLint width, height;
	//	eglQueryWaylandBufferWL (backend_get_egl_display(), surface->buffer, EGL_WIDTH, &width);
	//	eglQueryWaylandBufferWL (backend_get_egl_display(), surface->buffer, EGL_WIDTH, &height);
	//	EGLAttrib attribs = EGL_NONE;
	//	EGLImage image = eglCreateImage (backend_get_egl_display(), EGL_NO_CONTEXT, EGL_WAYLAND_BUFFER_WL, surface->buffer, &attribs);
	//	texture_delete (&surface->texture);
	//	texture_create_from_egl_image (&surface->texture, width, height, image);
	//	eglDestroyImage (backend_get_egl_display(), image);
	//}
	//else {
	//	struct wl_shm_buffer *shm_buffer = wl_shm_buffer_get (surface->buffer);
	//	uint32_t width = wl_shm_buffer_get_width (shm_buffer);
	//	uint32_t height = wl_shm_buffer_get_height (shm_buffer);
	//	void *data = wl_shm_buffer_get_data (shm_buffer);
	//	texture_delete (&surface->texture);
	//	texture_create (&surface->texture, width, height, data);
	//}
	//wl_buffer_send_release (surface->buffer);
	//redraw_needed = 1;
}
static void nuclear_impl_surface_set_buffer_transform(struct wl_client *client,
			     struct wl_resource *resource,
			     int32_t transform)
{
	printf("nuclear_surface_set_buffer_transform\n");
}
static void nuclear_impl_surface_set_buffer_scale(struct wl_client *client,
			 struct wl_resource *resource,
			 int32_t scale)
{
	printf("nuclear_surface_set_buffer_scale\n");
}
static void nuclear_impl_surface_damage_buffer(struct wl_client *client,
		      struct wl_resource *resource,
		      int32_t x,
		      int32_t y,
		      int32_t width,
		      int32_t height)
{
	printf("nuclear_surface_damage_buffer\n");
}
static struct wl_surface_interface nuclear_impl_surface_interface = {
	&nuclear_impl_surface_destroy,
	&nuclear_impl_surface_attach,
	&nuclear_impl_surface_damage,
	&nuclear_impl_surface_frame,
	&nuclear_impl_surface_set_opaque_region,
	&nuclear_impl_surface_set_input_region,
	&nuclear_impl_surface_commit,
	&nuclear_impl_surface_set_buffer_transform,
	&nuclear_impl_surface_set_buffer_scale,
	&nuclear_impl_surface_damage_buffer,
};
// wl region
static void nuclear_impl_region_destroy(struct wl_client *client,
		struct wl_resource *resource)
{
	printf("nuclear_region_destroy\n");
}
static void nuclear_impl_region_add(struct wl_client *client,
	    struct wl_resource *resource,
	    int32_t x,
	    int32_t y,
	    int32_t width,
	    int32_t height)
{
	printf("nuclear_region_add\n");
}
static void nuclear_impl_region_subtract(struct wl_client *client,
		 struct wl_resource *resource,
		 int32_t x,
		 int32_t y,
		 int32_t width,
		 int32_t height)
{
	printf("nuclear_region_subtract\n");
}
static struct wl_region_interface nuclear_impl_region_interface = {
	&nuclear_impl_region_destroy,
	&nuclear_impl_region_add,
	&nuclear_impl_region_subtract,
};

static void nuclear_impl_region_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
	//printf ("bind: compositor\n");
	//struct wl_resource *resource = wl_resource_create (client, &wl_compositor_interface, 1, id);
	//wl_resource_set_implementation(resource, &nuclear_compositor_interface, NULL, NULL);
}
// wl shell
// wl shell surface
// xdg surface
