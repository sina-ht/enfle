/*
 * x11window.c -- X11 Window interface
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file if part of Enfle.
 *
 * Last Modified: Wed Sep 20 00:46:08 2000.
 * $Id: x11window.c,v 1.1 2000/09/30 17:36:36 sian Exp $
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

static void destroy(X11Window *);

static X11Window template = {
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

static void
destroy(X11Window *xw)
{
  x11_destroy_window(xw->x11, xw->win);
  free(xw);
}
