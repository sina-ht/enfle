/*
 * x11.h -- X11 interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file if part of Enfle.
 *
 * Last Modified: Sat Jun 16 02:29:39 2001.
 * $Id: x11.h,v 1.10 2001/06/15 18:44:05 sian Exp $
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

#define X11_EXT_SHM (1 << 0)
#define X11_EXT_XV  (1 << 1)

#ifdef USE_XV
#define XV_YUY2_PACKED 0
#define XV_YUY2_PLANAR 1
#define XV_YV12_PACKED 2
#define XV_YV12_PLANAR 3
#define XV_I420_PACKED 4
#define XV_I420_PLANAR 5
#define XV_UYVY_PACKED 6
#define XV_UYVY_PLANAR 7
#define XV_FORMAT_MAX 8

#define XV_YUY2_PACKED_FLAG (1 << XV_YUY2_PACKED)
#define XV_YUY2_PLANAR_FLAG (1 << XV_YUY2_PLANAR)
#define XV_YV12_PACKED_FLAG (1 << XV_YV12_PACKED)
#define XV_YV12_PLANAR_FLAG (1 << XV_YV12_PLANAR)
#define XV_I420_PACKED_FLAG (1 << XV_I420_PACKED)
#define XV_I420_PLANAR_FLAG (1 << XV_I420_PLANAR)
#define XV_UYVY_PACKED_FLAG (1 << XV_UYVY_PACKED)
#define XV_UYVY_PLANAR_FLAG (1 << XV_UYVY_PLANAR)

typedef struct _x11_xv {
  unsigned int ver, rev, req_base, ev_base, err_base;
  int nadaptors;
  int image_port;
  unsigned int format_ids[XV_FORMAT_MAX];
  char prefer_msb[XV_FORMAT_MAX];
  int capable_format;
} X11Xv;
#endif

typedef struct _x11 X11;
struct _x11 {
  Display *disp;
  Window root;
  Visual *visual;
  Screen *sc;
  int screen;
  int depth;
  int bits_per_pixel;
  int prefer_msb;
  unsigned long white;
  unsigned long black;
  unsigned int extensions;
#ifdef USE_XV
  X11Xv xv;
#endif

  int (*open)(X11 *, char *);
  int (*close)(X11 *);
  void (*destroy)(X11 *);
};

#define x11_display(x) (x)->disp
#define x11_root(x) (x)->root
#define x11_visual(x) (x)->visual
#define x11_sc(x) (x)->sc
#define x11_screen(x) (x)->screen
#define x11_depth(x) (x)->depth
#define x11_bpp(x) (x)->bits_per_pixel
#define x11_prefer_msb(x) (x)->prefer_msb
#define x11_white(x) (x)->white
#define x11_black(x) (x)->black

#define x11_if_shm(x) ((x)->extensions & X11_EXT_SHM)
#define x11_if_xv(x) ((x)->extensions & X11_EXT_XV)

#define x11_open(x, d) (x)->open((x), (d))
#define x11_create_window(x, p, w, h) \
                    XCreateSimpleWindow(x11_display((x)), (p), 0, 0, \
					(w), (h), 0, x11_white((x)), x11_black((x)));
#define x11_move_window(x, win, xa, ya) XMoveWindow(x11_display((x)), (win), (xa), (ya))
#define x11_resize_window(x, win, w, h) XResizeWindow(x11_display((x)), (win), (w), (h))
#define x11_moveresize_window(x, win, xa, ya, w, h) XMoveResizeWindow(x11_display((x)), (win), (xa), (ya), (w), (h))
#define x11_raise_window(x, win) XRaiseWindow(x11_display(x), (win))
#define x11_map_window(x, win) XMapWindow(x11_display((x)), (win))
#define x11_map_raised(x, win) XMapRaised(x11_display((x)), (win))
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

#define x11_xv_create_ximage(x, p, i, d, w, h) XvCreateImage(x11_display((x)), (p), (i), (d), (w), (h))
#define x11_xv_shm_create_ximage(x, p, i, d, w, h, s) XvShmCreateImage(x11_display((x)), (p), (i), (d), (w), (h), (s))

X11 *x11_create(void);

#endif
