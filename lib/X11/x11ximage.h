/*
 * x11ximage.h -- X11 XImage related functions header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file if part of Enfle.
 *
 * Last Modified: Sun Dec  3 15:22:14 2000.
 * $Id: x11ximage.h,v 1.2 2000/12/03 08:40:04 sian Exp $
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

#ifndef _X11XIMAGE_H
#define _X11XIMAGE_H

#include "image.h"
#include "x11.h"
#ifdef USE_SHM
#include <X11/extensions/XShm.h>
#endif

typedef struct _x11ximage X11XImage;
struct _x11ximage {
  X11 *x11;
  XImage *ximage;
#ifdef USE_SHM
  int if_attached;
  XShmSegmentInfo shminfo;
#endif

  int (*convert)(X11XImage *, Image *);
  void (*put)(X11XImage *, Pixmap, GC, int, int, int, int, unsigned int, unsigned int);
  void (*destroy)(X11XImage *);
};

#define x11ximage_convert(xi, p) (xi)->convert((xi), (p))
#define x11ximage_put(xi, pix, gc, sx, sy, dx, dy, w, h) (xi)->put((xi), (pix), (gc), (sx), (sy), (dx), (dy), (w), (h))
#define x11ximage_destroy(xi) (xi)->destroy((xi))

X11XImage *x11ximage_create(X11 *);

#endif
