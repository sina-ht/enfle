/*
 * x11.h -- X11 interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file if part of Enfle.
 *
 * Last Modified: Thu Oct 12 18:58:06 2000.
 * $Id: x11.h,v 1.3 2000/10/12 15:44:28 sian Exp $
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

#ifndef _ENFLE_X11_H
#define _ENFLE_X11_H

typedef struct _x11 X11;
struct _x11 {
  Display *disp;
  Window root;
  Visual *visual;
  int screen;
  int depth;
  unsigned long white;
  unsigned long black;

  int (*open)(X11 *, char *);
  int (*close)(X11 *);
  void (*destroy)(X11 *);
};

#define x11_display(x) (x)->disp
#define x11_root(x) (x)->root
#define x11_visual(x) (x)->visual
#define x11_screen(x) (x)->screen
#define x11_depth(x) (x)->depth
#define x11_white(x) (x)->white
#define x11_black(x) (x)->black

#define x11_open(x, d) (x)->open((x), (d))
#define x11_create_window(x, p, w, h) \
                    XCreateSimpleWindow(x11_display((x)), (p), 0, 0, \
					(w), (h), 0, x11_white((x)), x11_black((x)));
#define x11_move_window(x, win, xa, ya) XResizeWindow(x11_display((x)), (win), (xa), (ya))
#define x11_resize_window(x, win, w, h) XResizeWindow(x11_display((x)), (win), (w), (h))
#define x11_moveresize_window(x, win, xa, ya, w, h) XResizeWindow(x11_display((x)), (win), (xa), (ya), (w), (h))
#define x11_map_window(x, win) XMapWindow(x11_display((x)), (win))
#define x11_unmap_window(x, win) XUnmapWindow(x11_display((x)), (win))
#define x11_storename(x, win, s) XStoreName(x11_display((x)), (win), (s))
#define x11_destroy_window(x, win) XDestroyWindow(x11_display((x)), (win))
#define x11_create_ximage(x, v, dep, d, w, h, p, bpl) \
                    XCreateImage(x11_display((x)), (v), (dep), ZPixmap, \
                                 0, (d), (w), (h), (p), (bpl))
#define x11_destroy_ximage(xi) XDestroyImage((xi))
#define x11_create_pixmap(x, win, w, h, d) \
                       XCreatePixmap(x11_display((x)), (win), (w), (h), (d))
#define x11_free_pixmap(x, pix) \
                       XFreePixmap(x11_display((x)), (pix))
#define x11_create_gc(x, p, a1, a2) XCreateGC(x11_display((x)), (p), (a1), (a2))
#define x11_free_gc(x, gc) XFreeGC(x11_display((x)), (gc))
#define x11_close(x) (x)->close((x))
#define x11_destroy(x) (x)->destroy((x))

X11 *x11_create(void);

#endif
