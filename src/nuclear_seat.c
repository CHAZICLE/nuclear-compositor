#include <stdlib.h>
#include "nuclear_surface.h"
#include "main.h"

// wl_pointer
void nuclear_pointer_set_cursor(struct wl_client *client,
	   struct wl_resource *resource,
	   uint32_t serial,
	   struct wl_resource *surface,
	   int32_t hotspot_x,
	   int32_t hotspot_y)
{
	nuclear_surface *ns = wl_resource_get_user_data (surface);
	//srv->cursor = ns;
}

void nuclear_pointer_release(struct wl_client *client,
	struct wl_resource *resource)
{
	
}

static struct wl_pointer_interface nuclear_pointer_interface = {
	&nuclear_pointer_set_cursor,
	&nuclear_pointer_release
};

// wl_keyboard
void nuclear_keyboard_release(struct wl_client *client,
		struct wl_resource *resource)
{

}

static struct wl_keyboard_interface nuclear_keyboard_interface = {
	&nuclear_keyboard_release
};


// seat
void nuclear_seat_get_pointer(struct wl_client *client,
	    struct wl_resource *resource,
	    uint32_t id)
{
	struct wl_resource *pointer = wl_resource_create (client, &wl_pointer_interface, 1, id);
	wl_resource_set_implementation (pointer, &nuclear_pointer_interface, NULL, NULL);
	//get_client(client)->pointer = pointer;
}

void nuclear_seat_get_keyboard(struct wl_client *client,
	     struct wl_resource *resource,
	     uint32_t id)
{
	struct wl_resource *keyboard = wl_resource_create (client, &wl_keyboard_interface, 1, id);
	//wl_resource_set_implementation (keyboard, &keyboard_interface, NULL, NULL);
	//get_client(client)->keyboard = keyboard;
	//int fd, size;
	//backend_get_keymap (&fd, &size);
	//wl_keyboard_send_keymap (keyboard, WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1, fd, size);
}

void nuclear_seat_get_touch(struct wl_client *client,
	  struct wl_resource *resource,
	  uint32_t id)
{
	
}

void nuclear_seat_release(struct wl_client *client,
	struct wl_resource *resource)
{
	
}

static struct wl_seat_interface nuclear_seat_interface = {
	&nuclear_seat_get_pointer,
	&nuclear_seat_get_keyboard,
	&nuclear_seat_get_touch,
	&nuclear_seat_release
};

void nuclear_seat_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id) {
	struct wl_resource *seat = wl_resource_create (client, &wl_seat_interface, 1, id);
	wl_resource_set_implementation (seat, &nuclear_seat_interface, NULL, NULL);
	wl_seat_send_capabilities(seat, WL_SEAT_CAPABILITY_POINTER|WL_SEAT_CAPABILITY_KEYBOARD);
}
