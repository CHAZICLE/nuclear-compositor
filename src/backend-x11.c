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
#include <stdio.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

static struct {
	Window window;
	EGLContext egl_context;
	EGLSurface egl_surface;
	EGLDisplay egl_display;
} window;
static Display *x_display;
static struct backend_callbacks callbacks;
static xcb_connection_t *xcb_connection;
static int32_t keyboard_device_id;
static struct xkb_keymap *keymap;
static struct xkb_state *state;

static void create_egl_window(void)
{
	print_eglversion(EGL_NO_DISPLAY);

	//window.egl_display = eglGetDisplay(x_display);
	window.egl_display = eglGetPlatformDisplay(EGL_PLATFORM_X11_EXT, x_display, NULL);
	if(window.egl_display==EGL_NO_DISPLAY)
	{
		fprintf(stderr, "NO DISPLAY\n");
		exit(1);
	}
	EGLint major,minor;
	if(eglInitialize(window.egl_display, &major, &minor)!=EGL_TRUE)
	{
		print_eglerror("EGL init failed");
		exit(1);
	}
	print_eglversion(window.egl_display);

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
	if(eglChooseConfig(window.egl_display, egl_attribs, &egl_config, 1, &num_configs_returned)!=GL_TRUE)
	{
		print_eglerror("eglChooseConfig failed");
		exit(1);
	}

	// Get the visual from the EGL config
	EGLint visual_id;
	if(eglGetConfigAttrib(window.egl_display, egl_config, EGL_NATIVE_VISUAL_ID, &visual_id)!=GL_TRUE)
	{
		print_eglerror("eglGetConfigAttrib failed");
		exit(1);
	}
	XVisualInfo visual_template;
	visual_template.visualid = visual_id;
	int num_visuals_returned;
	XVisualInfo *visual = XGetVisualInfo (x_display, VisualIDMask, &visual_template, &num_visuals_returned);
	
	// Create a window
	XSetWindowAttributes window_attributes;
	window_attributes.event_mask = ExposureMask | StructureNotifyMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | EnterWindowMask | LeaveWindowMask | FocusChangeMask;
	window_attributes.colormap = XCreateColormap (x_display, RootWindow(x_display,DefaultScreen(x_display)), visual->visual, AllocNone);
	window.window = XCreateWindow (
		x_display,
		RootWindow(x_display, DefaultScreen(x_display)),
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
	window.egl_context = eglCreateContext(window.egl_display, egl_config, EGL_NO_CONTEXT, eglContextAttribs);
	if(window.egl_context==EGL_NO_CONTEXT)
	{
		print_eglerror("eglCreateContext failed");
		exit(1);
	}
	window.egl_surface = eglCreateWindowSurface(window.egl_display, egl_config, window.window, NULL);
	if(window.egl_surface==EGL_NO_SURFACE)
	{
		print_eglerror("eglCreateWindowSurface failed");
		exit(1);
	}
	if(eglMakeCurrent(window.egl_display, window.egl_surface, window.egl_surface, window.egl_context)!=GL_TRUE)
	{
		print_eglerror("eglMakeCurrent failed");
		exit(1);
	}

	XFree(visual);
	XMapWindow (x_display, window.window);

	XDefineCursor(x_display, window.window, 0);

	print_glversion();
}

void backend_init (struct backend_callbacks *_callbacks, nuclear_server *srv) {
	callbacks = *_callbacks;
	callbacks.srv = srv;
	x_display = XOpenDisplay (NULL);
	
	xcb_connection = XGetXCBConnection (x_display);
	struct xkb_context *context = xkb_context_new (XKB_CONTEXT_NO_FLAGS);
	xkb_x11_setup_xkb_extension (xcb_connection, XKB_X11_MIN_MAJOR_XKB_VERSION, XKB_X11_MIN_MINOR_XKB_VERSION, 0, NULL, NULL, NULL, NULL);
	keyboard_device_id = xkb_x11_get_core_keyboard_device_id (xcb_connection);
	keymap = xkb_x11_keymap_new_from_device (context, xcb_connection, keyboard_device_id, XKB_KEYMAP_COMPILE_NO_FLAGS);
	state = xkb_x11_state_new_from_device (keymap, xcb_connection, keyboard_device_id);
	
	create_egl_window();
}

EGLDisplay backend_get_egl_display (void)
{
	return window.egl_display;
}

void backend_swap_buffers (void)
{
	eglSwapBuffers(window.egl_display, window.egl_surface);
}

static void update_modifiers (void) {
	struct modifier_state modifier_state;
	modifier_state.depressed = xkb_state_serialize_mods (state, XKB_STATE_MODS_DEPRESSED);
	modifier_state.latched = xkb_state_serialize_mods (state, XKB_STATE_MODS_LATCHED);
	modifier_state.locked = xkb_state_serialize_mods (state, XKB_STATE_MODS_LOCKED);
	modifier_state.group = xkb_state_serialize_layout (state, XKB_STATE_LAYOUT_EFFECTIVE);
	callbacks.modifiers (callbacks.srv, modifier_state);
}

int window_width,window_height;

void backend_dispatch_nonblocking (void) {
	XEvent event;
	while (XPending(x_display)) {
		XNextEvent (x_display, &event);
		if (event.type == ConfigureNotify) {
			window_width = event.xconfigure.width;
			window_height = event.xconfigure.height;
			callbacks.resize (callbacks.srv, event.xconfigure.width, event.xconfigure.height);
		}
		else if (event.type == Expose) {
			callbacks.draw (callbacks.srv);
		}
		else if (event.type == MotionNotify) {
			int dx = event.xbutton.x-window_width/2;
			int dy = event.xbutton.y-window_height/2;
			if(dx!=0 || dy!=0)
			{
				callbacks.mouse_motion (callbacks.srv, dx, dy);
				XWarpPointer(x_display, 0, window.window, 0,0, 0, 0, window_width/2, window_height/2);
			}
		}
		else if (event.type == ButtonPress) {
			if (event.xbutton.button == Button1)
				callbacks.mouse_button (callbacks.srv, BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
			else if (event.xbutton.button == Button2)
				callbacks.mouse_button (callbacks.srv, BTN_MIDDLE, WL_POINTER_BUTTON_STATE_PRESSED);
			else if (event.xbutton.button == Button3)
				callbacks.mouse_button (callbacks.srv, BTN_RIGHT, WL_POINTER_BUTTON_STATE_PRESSED);
		}
		else if (event.type == ButtonRelease) {
			if (event.xbutton.button == Button1)
				callbacks.mouse_button (callbacks.srv, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);
			else if (event.xbutton.button == Button2)
				callbacks.mouse_button (callbacks.srv, BTN_MIDDLE, WL_POINTER_BUTTON_STATE_RELEASED);
			else if (event.xbutton.button == Button3)
				callbacks.mouse_button (callbacks.srv, BTN_RIGHT, WL_POINTER_BUTTON_STATE_RELEASED);
		}
		else if (event.type == KeyPress) {
			callbacks.key (callbacks.srv, event.xkey.keycode - 8, WL_KEYBOARD_KEY_STATE_PRESSED);
			xkb_state_update_key (state, event.xkey.keycode, XKB_KEY_DOWN);
			update_modifiers ();
		}
		else if (event.type == KeyRelease) {

			if (XEventsQueued(x_display, QueuedAfterReading))
			{
				XEvent next;
				XPeekEvent(x_display, &next);
				if (next.type == KeyPress && next.xkey.window == event.xkey.window && next.xkey.keycode == event.xkey.keycode && (next.xkey.time-event.xkey.time) < 20)
				{
					return;
				}
			}

			callbacks.key (callbacks.srv, event.xkey.keycode - 8, WL_KEYBOARD_KEY_STATE_RELEASED);
			xkb_state_update_key (state, event.xkey.keycode, XKB_KEY_UP);
			update_modifiers ();
		}
		else if (event.type == FocusIn) {
			xkb_state_unref (state);
			state = xkb_x11_state_new_from_device (keymap, xcb_connection, keyboard_device_id);
			update_modifiers ();
		}
	}
}

void backend_wait_for_events (int wayland_fd) {
	struct pollfd fds[2] = {{ConnectionNumber(x_display), POLLIN}, {wayland_fd, POLLIN}};
	poll (fds, 2, -1);
}

void backend_get_keymap (int *fd, int *size) {
	char *string = xkb_keymap_get_as_string (keymap, XKB_KEYMAP_FORMAT_TEXT_V1);
	*size = strlen (string) + 1;
	char *xdg_runtime_dir = getenv ("XDG_RUNTIME_DIR");
	*fd = open (xdg_runtime_dir, O_TMPFILE|O_RDWR|O_EXCL, 0600);
	ftruncate (*fd, *size);
	char *map = mmap (NULL, *size, PROT_READ|PROT_WRITE, MAP_SHARED, *fd, 0);
	strcpy (map, string);
	munmap (map, *size);
	free (string);
}

long backend_get_timestamp (void) {
	struct timespec t;
	clock_gettime (CLOCK_MONOTONIC, &t);
	return t.tv_sec * 1000 + t.tv_nsec / 1000000;
}
