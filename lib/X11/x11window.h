/*
 * x11window.h -- X11 Window interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file if part of Enfle.
 *
 * Last Modified: Sun Oct 12 04:00:07 2003.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _X11WINDOW_H
#define _X11WINDOW_H

#include "x11.h"

typedef struct _x11window X11Window;

struct _x11window {
  X11 *x11;
  Window win;
  int mask;

  int (*set_event_mask)(X11Window *, int);
  int (*get_geometry)(X11Window *, unsigned int *, unsigned int *, unsigned int *, unsigned int *);
  void (*wait_mapped)(X11Window *);
  void (*destroy)(X11Window *);
};

#define x11window_x11(xw) (xw)->x11
#define x11window_win(xw) (xw)->win
#define x11window_mask(xw) (xw)->mask

#define x11window_set_event_mask(xw, m) (xw)->set_event_mask((xw), (m))
#define x11window_get_geometry(xw, xp, yp, wp, hp) (xw)->get_geometry((xw), (xp), (yp), (wp), (hp))
#define x11window_move(xw, x, y) x11_move_window(x11window_x11((xw)), x11window_win((xw)), (x), (y))
#define x11window_resize(xw, w, h) x11_resize_window(x11window_x11((xw)), x11window_win((xw)), (w), (h))
#define x11window_moveresize(xw, x, y, w, h) x11_moveresize_window(x11window_x11((xw)), x11window_win((xw)), (x), (y), (w), (h))
#define x11window_raise(xw) x11_raise_window(x11window_x11((xw)), x11window_win((xw)))
#define x11window_map(xw) x11_map_window(x11window_x11((xw)), x11window_win((xw)))
#define x11window_map_raised(xw) x11_map_raised(x11window_x11((xw)), x11window_win((xw)))
#define x11window_wait_mapped(xw) (xw)->wait_mapped((xw))
#define x11window_unmap(xw) x11_unmap_window(x11window_x11((xw)), x11window_win((xw)))
#define x11window_storename(xw, s) x11_storename(x11window_x11((xw)), x11window_win((xw)), s)
#define x11window_setwmname(xw, t) x11_setwmname(x11window_x11((xw)), x11window_win((xw)), t)

#define x11window_destroy(xw) (xw)->destroy((xw))

X11Window *x11window_create(X11 *, X11Window *, unsigned int, unsigned int);

#endif
