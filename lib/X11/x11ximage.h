/*
 * x11ximage.h -- X11 XImage related functions header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file if part of Enfle.
 *
 * Last Modified: Fri Sep  7 18:22:50 2001.
 * $Id: x11ximage.h,v 1.5 2001/09/09 23:56:43 sian Exp $
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

#include "enfle/image.h"
#ifdef USE_SHM
#  include <X11/extensions/XShm.h>
#endif
#ifdef USE_XV
#  include <X11/extensions/Xvlib.h>
#endif
#include "x11.h"
#include "utils/cpucaps.h"

typedef struct _x11ximage X11XImage;
struct _x11ximage {
  X11 *x11;
  XImage *ximage;
  int use_xv;
#ifdef USE_XV
  XvImage *xvimage;
#endif
#ifdef USE_SHM
  int if_attached;
  XShmSegmentInfo shminfo;
#endif
  void (*bgra32to16)(unsigned char *, unsigned char *, int, int, int, int);

  int (*convert)(X11XImage *, Image *);
  void (*put)(X11XImage *, Pixmap, GC, int, int, int, int, unsigned int, unsigned int);
#ifdef USE_XV
  void (*put_scaled)(X11XImage *, Pixmap, GC, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int);
#endif
  void (*destroy)(X11XImage *);
};

#define x11ximage_convert(xi, p) (xi)->convert((xi), (p))
#define x11ximage_put(xi, pix, gc, sx, sy, dx, dy, w, h) (xi)->put((xi), (pix), (gc), (sx), (sy), (dx), (dy), (w), (h))
#ifdef USE_XV
#define x11ximage_put_scaled(xi, pix, gc, sx, sy, dx, dy, sw, sh, dw, dh) (xi)->put_scaled((xi), (pix), (gc), (sx), (sy), (dx), (dy), (sw), (sh), (dw), (dh))
#endif
#define x11ximage_destroy(xi) (xi)->destroy((xi))

X11XImage *x11ximage_create(X11 *);

#endif
