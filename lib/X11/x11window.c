/*
 * x11window.c -- X11 Window interface
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file if part of Enfle.
 *
 * Last Modified: Sat Oct 21 01:52:15 2000.
 * $Id: x11window.c,v 1.2 2000/10/20 18:13:06 sian Exp $
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

#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "common.h"

#include "x11window.h"

static int set_event_mask(X11Window *, int);
static int get_position(X11Window *, unsigned int *, unsigned int *);
static void destroy(X11Window *);

static X11Window template = {
  set_event_mask: set_event_mask,
  get_position: get_position,
  destroy: destroy
};

X11Window *
x11window_create(X11 *x11, X11Window *parent, unsigned int width, unsigned height)
{
  X11Window *xw;

  if ((xw = calloc(1, sizeof(X11Window))) == NULL)
    return NULL;
  memcpy(xw, &template, sizeof(X11Window));

  xw->x11 = x11;
  xw->win = x11_create_window(xw->x11, parent ? x11window_win(parent) : x11_root(x11),
			      width, height);

  return xw;
}

/* methods */

static int
set_event_mask(X11Window *xw, int mask)
{
  X11 *x11;

  x11 = x11window_x11(xw);
  XSelectInput(x11_display(x11), x11window_win(xw), mask);
  xw->mask = mask;

  return 1;
}

static int
get_position(X11Window *xw, unsigned int *x_return, unsigned int *y_return)
{
  X11 *x11;
  Window root, parent, *child;
  int x, y, px, py, nc;
  unsigned int width, height, bw, depth;

  x11 = x11window_x11(xw);
  if (!XGetGeometry(x11_display(x11), x11window_win(xw), &root, &x, &y, &width, &height, &bw, &depth))
    return 0;
  if (!XQueryTree(x11_display(x11), x11window_win(xw), &root, &parent, &child, &nc))
    return 0;
  if (child != NULL)
    XFree(child);

  while (root != parent) {
    if (!XGetGeometry(x11_display(x11), parent, &root, &px, &py, &width, &height, &bw, &depth))
      return 0;
    x += px + bw;
    y += py + bw;
    if (!XQueryTree(x11_display(x11), parent, &root, &parent, &child, &nc))
      return 0;
    if (child != NULL)
      XFree(child);
  }

  *x_return = x;
  *y_return = y;

  return 1;
}

static void
destroy(X11Window *xw)
{
  x11_destroy_window(xw->x11, xw->win);
  free(xw);
}
