/*
 * x11window.h -- X11 Window interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file if part of Enfle.
 *
 * Last Modified: Mon Sep 18 07:19:44 2000.
 * $Id: x11window.h,v 1.1 2000/09/30 17:36:36 sian Exp $
 *
 * Enfle is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Enfle is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#ifndef _X11WINDOW_H
#define _X11WINDOW_H

#include "x11.h"

typedef struct _x11window X11Window;

struct _x11window {
  X11 *x11;
  Window win;

  void (*destroy)(X11Window *);
};

#define x11window_x11(xw) (xw)->x11
#define x11window_win(xw) (xw)->win

#define x11window_move(xw, x, y) x11_move_window(x11window_x11((xw)), x11window_win((xw)), (x), (y))
#define x11window_resize(xw, w, h) x11_resize_window(x11window_x11((xw)), x11window_win((xw)), (w), (h))
#define x11window_moveresize(xw, x, y, w, h) x11_moveresize(x11window_x11((xw)), x11window_win((xw)), (x), (y), (w), (h))
#define x11window_map(xw) x11_map_window(x11window_x11((xw)), x11window_win((xw)))
#define x11window_unmap(xw) x11_unmap_window(x11window_x11((xw)), x11window_win((xw)))
#define x11window_storename(xw, s) x11_storename(x11window_x11((xw)), x11window_win((xw)), s)

#define x11window_destroy(xw) (xw)->destroy((xw))

X11Window *x11window_create(X11 *, X11Window *, unsigned int, unsigned int);

#endif
