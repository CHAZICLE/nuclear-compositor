#ifndef __BACKEND_H_INCLUDED__
#define __BACKEND_H_INCLUDED__

#include "render/nuclear_renderer.h"

void backend_init(nuclear_renderer *rnd);
void backend_setup_egl(nuclear_renderer *rnd);
void backend_make_current(nuclear_renderer *rnd);
void backend_swap_buffers(nuclear_renderer *rnd);
void backend_dispatch_nonblocking(nuclear_renderer *rnd);
void backend_wait_for_events(nuclear_renderer *rnd, int wayland_fd);
void backend_get_keymap(nuclear_renderer *rnd, int *fd, int *size);
long backend_get_timestamp(nuclear_renderer *rnd);

#endif
