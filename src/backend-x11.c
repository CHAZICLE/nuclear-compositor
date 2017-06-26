#define _GNU_SOURCE
#include "backend.h"
#include <wayland-server.h>
#include <X11/Xlib.h>
#include <linux/input.h>
#include <X11/Xlib-xcb.h>
#include <xkbcommon/xkbcommon-x11.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <poll.h>
#include "nuclear_utils.h"
#include <X11/Xcursor/Xcursor.h>
#include <stdio.h>
#include "render/nuclear_renderer.h"

typedef struct _backend_x11 backend_x11;

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

struct _backend_x11 {
	Display *x_display;
	Window window;

	Cursor cursor;

	xcb_connection_t *xcb_connection;
	int32_t keyboard_device_id;
	struct xkb_keymap *keymap;
	struct xkb_state *state;
};

void backend_init(nuclear_renderer *rnd)
{
	backend_x11 *bkx11 = malloc(sizeof(backend_x11));
	rnd->backend_data = bkx11;
	bkx11->x_display = XOpenDisplay(NULL);
	
	bkx11->xcb_connection = XGetXCBConnection(bkx11->x_display);
	struct xkb_context *context = xkb_context_new (XKB_CONTEXT_NO_FLAGS);
	xkb_x11_setup_xkb_extension(bkx11->xcb_connection, XKB_X11_MIN_MAJOR_XKB_VERSION, XKB_X11_MIN_MINOR_XKB_VERSION, 0, NULL, NULL, NULL, NULL);
	bkx11->keyboard_device_id = xkb_x11_get_core_keyboard_device_id(bkx11->xcb_connection);
	bkx11->keymap = xkb_x11_keymap_new_from_device (context, bkx11->xcb_connection, bkx11->keyboard_device_id, XKB_KEYMAP_COMPILE_NO_FLAGS);
	bkx11->state = xkb_x11_state_new_from_device(bkx11->keymap, bkx11->xcb_connection, bkx11->keyboard_device_id);

	rnd->display = eglGetDisplay(bkx11->x_display);
	backend_setup_egl(rnd);
}
void backend_setup_egl(nuclear_renderer *rnd)
{
	backend_x11 *bkx11 = (backend_x11 *)rnd->backend_data;

	// Init EGL
	print_eglversion(EGL_NO_DISPLAY);
	EGLint major,minor;
	if(eglInitialize(rnd->display, &major, &minor)!=EGL_TRUE)
	{
		print_eglerror("EGL init failed");
		exit(1);
	}
	print_eglversion(rnd->display);

	// Setup EGL
	EGLint egl_attribs[] = {
		EGL_RED_SIZE, 1,
		EGL_GREEN_SIZE, 1,
		EGL_BLUE_SIZE, 1,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
		EGL_NONE
	};
	EGLConfig egl_config;
	EGLint num_configs_returned;
	if(eglChooseConfig(rnd->display, egl_attribs, &egl_config, 1, &num_configs_returned)!=GL_TRUE)
	{
		print_eglerror("eglChooseConfig failed");
		exit(1);
	}

	// Get the visual from the EGL config
	EGLint visual_id;
	if(eglGetConfigAttrib(rnd->display, egl_config, EGL_NATIVE_VISUAL_ID, &visual_id)!=GL_TRUE)
	{
		print_eglerror("eglGetConfigAttrib failed");
		exit(1);
	}
	XVisualInfo visual_template;
	visual_template.visualid = visual_id;
	int num_visuals_returned;
	XVisualInfo *visual = XGetVisualInfo (bkx11->x_display, VisualIDMask, &visual_template, &num_visuals_returned);
	
	// Create a window
	XSetWindowAttributes window_attributes;
	window_attributes.event_mask = ExposureMask | StructureNotifyMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | EnterWindowMask | LeaveWindowMask | FocusChangeMask;
	window_attributes.colormap = XCreateColormap(bkx11->x_display, RootWindow(bkx11->x_display, DefaultScreen(bkx11->x_display)), visual->visual, AllocNone);
	bkx11->window = XCreateWindow (
		bkx11->x_display,
		RootWindow(bkx11->x_display, DefaultScreen(bkx11->x_display)),
		0, 0,
		WINDOW_WIDTH, WINDOW_HEIGHT,
		0, // border width
		visual->depth, // depth
		InputOutput, // class
		visual->visual, // visual
		CWEventMask|CWColormap, // attribute mask
		&window_attributes // attributes
	);

	static const EGLint eglContextAttribs[] = {
		//EGL_CONTEXT_MAJOR_VERSION, 3,
		//EGL_CONTEXT_MINOR_VERSION, 1,
		EGL_CONTEXT_MAJOR_VERSION, 2,
		EGL_CONTEXT_MINOR_VERSION, 0,
		EGL_NONE
	};
	
	// EGL context and surface
	if(eglBindAPI(EGL_OPENGL_ES_API)==EGL_FALSE)
	{
		print_eglerror("Failed to bind api");
		exit(1);
	}
	rnd->context = eglCreateContext(rnd->display, egl_config, EGL_NO_CONTEXT, eglContextAttribs);
	if(rnd->context==EGL_NO_CONTEXT)
	{
		print_eglerror("eglCreateContext failed");
		exit(1);
	}
	rnd->surface = eglCreateWindowSurface(rnd->display, egl_config, bkx11->window, NULL);
	if(rnd->surface==EGL_NO_SURFACE)
	{
		print_eglerror("eglCreateWindowSurface failed");
		exit(1);
	}
	backend_make_current(rnd);

	print_glversion();

	XFree(visual);
	XMapWindow(bkx11->x_display, bkx11->window);

	XcursorImage* native = XcursorImageCreate(16, 16);
	native->xhot = 0;
	native->yhot = 0;
	memset(native->pixels, 16*16*4, sizeof(unsigned char));
	bkx11->cursor = XcursorImageLoadCursor(bkx11->x_display, native);
	XcursorImageDestroy(native);

	XDefineCursor(bkx11->x_display, bkx11->window, bkx11->cursor);
	XFlush(bkx11->x_display);
}
void backend_make_current(nuclear_renderer *rnd)
{
	if(eglMakeCurrent(rnd->display, rnd->surface, rnd->surface, rnd->context)!=GL_TRUE)
	{
		print_eglerror("eglMakeCurrent failed");
		exit(1);
	}
}
void backend_swap_buffers(nuclear_renderer *rnd)
{
	eglSwapBuffers(rnd->display, rnd->surface);
}
void update_modifiers(nuclear_renderer *rnd)
{
	backend_x11 *bkx11 = (backend_x11 *)rnd->backend_data;
	struct modifier_state modifier_state;
	modifier_state.depressed = xkb_state_serialize_mods(bkx11->state, XKB_STATE_MODS_DEPRESSED);
	modifier_state.latched = xkb_state_serialize_mods(bkx11->state, XKB_STATE_MODS_LATCHED);
	modifier_state.locked = xkb_state_serialize_mods(bkx11->state, XKB_STATE_MODS_LOCKED);
	modifier_state.group = xkb_state_serialize_layout(bkx11->state, XKB_STATE_LAYOUT_EFFECTIVE);
	rnd->backend_handle_modifiers_event(rnd, modifier_state);
}

int window_width,window_height;

void backend_dispatch_nonblocking(nuclear_renderer *rnd)
{
	backend_x11 *bkx11 = (backend_x11 *)rnd->backend_data;
	XEvent event;
	while (XPending(bkx11->x_display))
	{
		XNextEvent(bkx11->x_display, &event);
		if(event.type==ConfigureNotify)
		{
			window_width = event.xconfigure.width;
			window_height = event.xconfigure.height;
			XWarpPointer(bkx11->x_display, 0, bkx11->window, 0,0, 0, 0, window_width/2, window_height/2);
			rnd->backend_handle_resize_event(rnd, event.xconfigure.width, event.xconfigure.height);
		}
		else if(event.type==Expose)
		{
			rnd->backend_handle_draw_event(rnd);
		}
		else if (event.type == MotionNotify)
		{
			int dx = event.xbutton.x-window_width/2;
			int dy = event.xbutton.y-window_height/2;
			if(dx!=0 || dy!=0)
			{
				rnd->backend_handle_mouse_motion_event(rnd, dx, dy);
				XWarpPointer(bkx11->x_display, 0, bkx11->window, 0,0, 0, 0, window_width/2, window_height/2);
			}
		}
		else if (event.type == ButtonPress)
		{
			if (event.xbutton.button == Button1)
				rnd->backend_handle_mouse_button_event(rnd, BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
			else if (event.xbutton.button == Button2)
				rnd->backend_handle_mouse_button_event(rnd, BTN_MIDDLE, WL_POINTER_BUTTON_STATE_PRESSED);
			else if (event.xbutton.button == Button3)
				rnd->backend_handle_mouse_button_event(rnd, BTN_RIGHT, WL_POINTER_BUTTON_STATE_PRESSED);
		}
		else if (event.type == ButtonRelease)
		{
			if (event.xbutton.button == Button1)
				rnd->backend_handle_mouse_button_event(rnd, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);
			else if (event.xbutton.button == Button2)
				rnd->backend_handle_mouse_button_event(rnd, BTN_MIDDLE, WL_POINTER_BUTTON_STATE_RELEASED);
			else if (event.xbutton.button == Button3)
				rnd->backend_handle_mouse_button_event(rnd, BTN_RIGHT, WL_POINTER_BUTTON_STATE_RELEASED);
		}
		else if (event.type == KeyPress)
		{
			rnd->backend_handle_key_event(rnd, event.xkey.keycode - 8, WL_KEYBOARD_KEY_STATE_PRESSED);
			xkb_state_update_key(bkx11->state, event.xkey.keycode, XKB_KEY_DOWN);
			update_modifiers(rnd);
		}
		else if (event.type == KeyRelease)
		{
			if (XEventsQueued(bkx11->x_display, QueuedAfterReading))
			{
				XEvent next;
				XPeekEvent(bkx11->x_display, &next);
				if (next.type == KeyPress && next.xkey.window == event.xkey.window && next.xkey.keycode == event.xkey.keycode && (next.xkey.time-event.xkey.time) < 20)
				{
					return;
				}
			}
			rnd->backend_handle_key_event(rnd, event.xkey.keycode - 8, WL_KEYBOARD_KEY_STATE_RELEASED);
			xkb_state_update_key(bkx11->state, event.xkey.keycode, XKB_KEY_UP);
			update_modifiers(rnd);
		}
		else if (event.type == FocusIn)
		{
			xkb_state_unref(bkx11->state);
			bkx11->state = xkb_x11_state_new_from_device(bkx11->keymap, bkx11->xcb_connection, bkx11->keyboard_device_id);
			update_modifiers(rnd);
		}
	}
}
void backend_wait_for_events(nuclear_renderer *rnd, int wayland_fd)
{
	backend_x11 *bkx11 = (backend_x11 *)rnd->backend_data;
	struct pollfd fds[2] = {{ConnectionNumber(bkx11->x_display), POLLIN}, {wayland_fd, POLLIN}};
	poll (fds, 2, -1);
}
void backend_get_keymap(nuclear_renderer *rnd, int *fd, int *size)
{
	backend_x11 *bkx11 = (backend_x11 *)rnd->backend_data;
	char *string = xkb_keymap_get_as_string(bkx11->keymap, XKB_KEYMAP_FORMAT_TEXT_V1);
	*size = strlen (string) + 1;
	char *xdg_runtime_dir = getenv ("XDG_RUNTIME_DIR");
	*fd = open (xdg_runtime_dir, O_TMPFILE|O_RDWR|O_EXCL, 0600);
	ftruncate (*fd, *size);
	char *map = mmap (NULL, *size, PROT_READ|PROT_WRITE, MAP_SHARED, *fd, 0);
	strcpy (map, string);
	munmap (map, *size);
	free (string);
}
long backend_get_timestamp(nuclear_renderer *rnd)
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return t.tv_sec * 1000 + t.tv_nsec / 1000000;
}
