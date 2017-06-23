#ifndef __BACKEND_H_INCLUDED__
#define __BACKEND_H_INCLUDED__

#include "main.h"
#include "nuclear_utils.h"

struct backend_callbacks
{
	nuclear_server *srv;
	void (*resize) (nuclear_server *srv, int width, int height);
	void (*draw) (nuclear_server *srv);
	void (*mouse_motion) (nuclear_server *srv, int x, int y);
	void (*mouse_button) (nuclear_server *srv, int button, int state);
	void (*key) (nuclear_server *srv, int key, int state);
	void (*modifiers) (nuclear_server *srv, struct modifier_state modifier_state);
	void (*render) (nuclear_server *srv);
};

struct backend_egl_state {
	EGLDisplay display;
	EGLContext context;
	EGLSurface surface;
};

void backend_init(struct backend_callbacks *callbacks, nuclear_server *srv);
void backend_setup_egl(struct backend_egl_state *egl_state);
struct backend_egl_state *backend_get_egl();
void backend_make_current();
void backend_swap_buffers();
void backend_dispatch_nonblocking ();
void backend_wait_for_events (int wayland_fd);
void backend_get_keymap(int *fd, int *size);
long backend_get_timestamp();

#endif
