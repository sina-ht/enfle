/*
 * x11.c -- X11 interface
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file if part of Enfle.
 *
 * Last Modified: Sun Oct 15 21:30:13 2000.
 * $Id: x11.c,v 1.2 2000/10/15 12:30:30 sian Exp $
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

#ifdef DEBUG
#  include <stdio.h>
#endif

#include "x11.h"

static int open(X11 *, char *);
static int close(X11 *);
static void destroy(X11 *);

static X11 template = {
  open: open,
  close: close,
  destroy: destroy
};

X11 *
x11_create(void)
{
  X11 *x11;

  if ((x11 = calloc(1, sizeof(X11))) == NULL)
    return NULL;
  memcpy(x11, &template, sizeof(X11));

  return x11;
}

/* methods */

static int
open(X11 *x11, char *dispname)
{
  XVisualInfo *xvi, xvit;
  XPixmapFormatValues *xpfv;
  int i, count;

  if ((x11->disp = XOpenDisplay(dispname)) == NULL)
    return 0;

  x11->root = DefaultRootWindow(x11_display(x11));
  x11->screen = DefaultScreen(x11_display(x11));
  x11->white = WhitePixel(x11_display(x11), x11_screen(x11));
  x11->black = BlackPixel(x11_display(x11), x11_screen(x11));

  xvit.screen = x11_screen(x11);
  xvit.depth = 24;
  xvit.class = TrueColor;
  xvi = XGetVisualInfo(x11_display(x11), VisualScreenMask | VisualDepthMask | VisualClassMask,
		       &xvit, &count);
  if (count) {
    x11->visual = xvi[0].visual;
    x11->depth = 24;

    debug_message("x11: open: Depth 24 TrueColor visual available\n");

    XFree(xvi);
  } else {
    x11->visual = DefaultVisual(x11_display(x11), x11_screen(x11));
    x11->depth = DefaultDepth(x11_display(x11), x11_screen(x11));

    debug_message("x11: open: using default visual\n");
  }

  xpfv = XListPixmapFormats(x11_display(x11), &count);
  if (x11->depth == 24) {
    x11->bits_per_pixel = 0;
    for (i = 0; i < count; i++) {
      if (xpfv[i].depth == 24 && x11->bits_per_pixel < xpfv[i].bits_per_pixel)
	x11->bits_per_pixel = xpfv[i].bits_per_pixel;
    }
  } else {
    for (i = 0; i < count; i++) {
      if (xpfv[i].depth == x11->depth) {
	x11->bits_per_pixel = xpfv[i].bits_per_pixel;
	break;
      }
    }
  }
  XFree(xpfv);

#ifdef DEBUG
  debug_message("x11: open: bits_per_pixel = %d\n", x11_bpp(x11));
  if (x11_bpp(x11) == 32)
    debug_message("You have 32bpp ZPixmap.\n");
#endif

  return 1;
}

static int
close(X11 *x11)
{
  XCloseDisplay(x11_display(x11));
  x11_display(x11) = NULL;

  return 1;
}

static void
destroy(X11 *x11)
{
  if (x11_display(x11))
    close(x11);
  free(x11);
}
