/*
 * x11ximage.c -- X11 XImage related functions
 * (C)Copyright 2000, 2002 by Hiroshi Takekawa
 * This file if part of Enfle.
 *
 * Last Modified: Sat Feb 21 15:51:16 2004.
 * $Id: x11ximage.c,v 1.50 2004/02/21 07:49:36 sian Exp $
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

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "x11ximage.h"

void bgra32to16_generic(unsigned char *, unsigned char *, unsigned int, unsigned int, unsigned int, unsigned int);
#ifdef USE_MMX
void bgra32to16_maybe_mmx(unsigned char *, unsigned char *, unsigned int, unsigned int, unsigned int, unsigned int);
void bgra32to16_mmx(unsigned char *, unsigned char *, unsigned int, unsigned int);
#endif

static int is_hw_scalable(X11XImage *, Image *, int *);
static int convert(X11XImage *, Image *, int, int);
static void put(X11XImage *, Pixmap, GC, int, int, int, int, unsigned int, unsigned int);
static void put_scaled(X11XImage *, Pixmap, GC, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int);
static void destroy(X11XImage *);

static X11XImage template = {
  is_hw_scalable: is_hw_scalable,
  convert: convert,
  put: put,
  put_scaled: put_scaled,
  destroy: destroy
};

X11XImage *
x11ximage_create(X11 *x11)
{
  X11XImage *xi;
  CPUCaps cpucaps;

  if ((xi = calloc(1, sizeof(X11XImage))) == NULL)
    return NULL;
  memcpy(xi, &template, sizeof(X11XImage));
  xi->x11 = x11;
  xi->ximage = NULL;
  xi->xvimage = NULL;
#ifdef USE_SHM
  if ((xi->shminfo = calloc(1, sizeof(XShmSegmentInfo))) == NULL) {
    free(xi);
    return NULL;
  }
#endif
  cpucaps = cpucaps_get();
#ifdef USE_MMX
  if (cpucaps_is_mmx(cpucaps)) {
    //debug_message_fnc("MMX is available.\n");
    xi->bgra32to16 = bgra32to16_maybe_mmx;
  } else
#endif
    xi->bgra32to16 = bgra32to16_generic;

  return xi;
}

/* for internal use */

static void
destroy_ximage(X11XImage *xi)
{
  x11_lock(xi->x11);
  if (xi->use_xv) {
    if (xi->xvimage) {
#ifdef USE_SHM
      if (xi->if_attached) {
	XSync(x11_display(xi->x11), False);
	XShmDetach(x11_display(xi->x11), xi->shminfo);
	xi->if_attached = 0;
	//debug_message_fnc("SHM detached\n");
      }
#endif
      XFree(xi->xvimage);
      xi->xvimage = NULL;
    }
  } else {
    if (xi->ximage) {
      xi->ximage->data = NULL;
#ifdef USE_SHM
      if (xi->if_attached) {
	XSync(x11_display(xi->x11), False);
	XShmDetach(x11_display(xi->x11), xi->shminfo);
	xi->if_attached = 0;
	//debug_message_fnc("SHM detached\n");
      }
#endif
      x11_destroy_ximage(xi->ximage);
      xi->ximage = NULL;
    }
  }
  x11_unlock(xi->x11);
}

void
bgra32to16_generic(unsigned char *dest, unsigned char *s, unsigned int w, unsigned int h, unsigned int bpl, unsigned int bpl_s)
{
  unsigned int i, j, pix;
  unsigned char *d;

  for (j = 0; j < h; j++) {
    d = dest + j * bpl;
    for (i = 0; i < w; i++) {
      pix =
	((s[i * 4 + 2] & 0xf8) << 8) |
	((s[i * 4 + 1] & 0xfc) << 3) |
	((s[i * 4 + 0] & 0xf8) >> 3);
      *d++ = pix & 0xff;
      *d++ = pix >> 8;
    }
    s += bpl_s;
  }
}

#ifdef USE_MMX
void
bgra32to16_maybe_mmx(unsigned char *dest, unsigned char *s, unsigned int w, unsigned int h, unsigned int bpl, unsigned int bpl_s)
{
  if (bpl == w * 2)
    bgra32to16_mmx(dest, s, w, h);
  else
    bgra32to16_generic(dest, s, w, h, bpl, bpl_s);
}
#endif

/* methods */

static int
is_hw_scalable(X11XImage *xi, Image *p, int *t_return)
{
  int t;

#ifdef USE_XV
  switch (p->type) {
  case _YUY2:
    t = XV_YUY2;
    break;
  case _YV12:
    t = XV_YV12;
    break;
  case _I420:
    t = XV_I420;
    break;
  case _UYVY:
    t = XV_UYVY;
    break;
#if 0
  case _RGB24:
    t = XV_RV24;
    break;
  case _RGBA32:
    t = XV_RV32;
    break;
#endif
  default:
    t = XV_OTHER;
    break;
  }
  if (t_return)
    *t_return = t;
  return XV_ABLE_TO_RENDER(xi->x11->xv, t);
#else
  return 0;
#endif
}

static int
convert(X11XImage *xi, Image *p, int src, int dst)
{
  unsigned int i, j;
  int bits_per_pixel = 0;
  int bytes_per_line_s;
  int create_ximage = 0;
#ifdef USE_SHM
  int to_be_attached = 0;
#endif
  unsigned char *dest = NULL, *d, *s;
  XImage *ximage = NULL;
#ifdef USE_XV
  XvImage *xvimage = NULL;
  int t = 0;
#endif
  Memory *src_img, *dst_img;
  unsigned int w, h;

  src_img = image_image_by_index(p, src);
  w = image_width_by_index(p, src);
  h = image_height_by_index(p, src);
  dst_img = image_image_by_index(p, dst);

  xi->use_xv = is_hw_scalable(xi, p, &t);

#ifdef USE_XV
  if (xi->use_xv && xi->xvimage) {
    if (xi->xvimage->width != (int)w || xi->xvimage->height != (int)h || p->type != xi->type
#ifdef USE_SHM
	|| (memory_type(dst_img) == _SHM && xi->shminfo->shmid != memory_shmid(dst_img))
#endif
	) {
      xi->type = p->type;
      destroy_ximage(xi);
      create_ximage = 1;
    }
  } else
#endif
  if (!xi->use_xv && xi->ximage) {
    if (xi->ximage->width != (int)w || xi->ximage->height != (int)h || p->type != xi->type
#ifdef USE_SHM
	|| (memory_type(dst_img) == _SHM && xi->shminfo->shmid != memory_shmid(dst_img))
#endif
	) {
      xi->type = p->type;
      destroy_ximage(xi);
      create_ximage = 1;
    }
  } else {
    create_ximage = 1;
    xi->type = p->type;
  }

#if 0
  if (create_ximage)
    debug_message_fnc("(%d, %d) use_xv %d ximage %p xvimage %p\n", w, h, xi->use_xv, xi->ximage, xi->xvimage);
#endif

  if (create_ximage) {
    x11_lock(xi->x11);
    switch (memory_type(dst_img)) {
    case _NORMAL:
#ifdef USE_XV
      if (xi->use_xv) {
	if ((xi->xvimage = x11_xv_create_ximage(xi->x11, xi->x11->xv->image_port, xi->x11->xv->format_ids[t], memory_ptr(dst_img), w, h)))
	  xi->format_num = t;
      }
      if (!xi->use_xv || xi->xvimage == NULL) {
#endif
	xi->ximage =
	  x11_create_ximage(xi->x11, x11_visual(xi->x11), x11_depth(xi->x11),
			    memory_ptr(dst_img), w, h, 8, 0);
#ifdef USE_XV
      }
#endif
      break;
    case _SHM:
#ifdef USE_SHM
#ifdef USE_XV
      if (xi->use_xv) {
	debug_message_fnc("port %d  id %X  (%d, %d)\n", xi->x11->xv->image_port, xi->x11->xv->format_ids[t], w, h);

	if ((xi->xvimage = x11_xv_shm_create_ximage(xi->x11, xi->x11->xv->image_port, xi->x11->xv->format_ids[t], NULL, w, h, xi->shminfo)))
	  xi->format_num = t;
      }
      if (!xi->use_xv || xi->xvimage == NULL) {
#endif /* USE_XV */
	xi->ximage =
	  XShmCreateImage(x11_display(xi->x11), x11_visual(xi->x11), x11_depth(xi->x11), ZPixmap, NULL,
			  xi->shminfo, w, h);
#ifdef USE_XV
      }
#endif
      to_be_attached = 1;
      XSync(x11_display(xi->x11), False);
#else /* USE_SHM */
      fatal("No SHM support. Should not be reached here.\n");
#endif /* USE_SHM */
      break;
    default:
      return 0;
    }
    //debug_message_fnc("ximage->bpl = %d\n", xi->ximage->bytes_per_line);
    x11_unlock(xi->x11);
  }

  if (!xi->use_xv) {
    ximage = xi->ximage;
    image_bpl_by_index(p, dst) = ximage->bytes_per_line;
#if 0
    debug_message("x order %s\n", ximage->byte_order == LSBFirst ? "LSB" : "MSB");
    debug_message("p type: %s p bpp: %d x bpp: %d bpl: %d\n", image_type_to_string(p->type), p->bits_per_pixel, ximage->bits_per_pixel, ximage->bytes_per_line);
#endif
  }

  /* _GRAY -> _INDEX */
  if (p->type == _GRAY) {
    debug_message_fnc("_GRAY -> _INDEX\n");
    for (i = 0; i < p->ncolors; i++)
      p->colormap[i][0] = p->colormap[i][1] = p->colormap[i][2] = 255 * i / (p->ncolors - 1);
    p->type = _INDEX;
  }

  /* _BITMAP_* -> _INDEX */
  if (p->type == _BITMAP_LSBFirst || p->type == _BITMAP_MSBFirst) {
    Memory *m;
    unsigned char *ss, *dd, b;
    int k, obpl, remain;

    debug_message_fnc("_BITMAP -> _INDEX\n");

    m = memory_dup(src_img);
    ss = memory_ptr(m);
    obpl = image_bpl_by_index(p, src);
    image_bpl_by_index(p, src) = image_width_by_index(p, src);
    p->depth = p->bits_per_pixel = 8;
    p->ncolors = 2;
    if ((dd = memory_alloc(src_img, image_bpl_by_index(p, src) * h)) == NULL)
      fatal("%s: No enough memory(alloc)\n", __FUNCTION__);
    p->colormap[0][0] = p->colormap[0][1] = p->colormap[0][2] = 255;
    p->colormap[1][0] = p->colormap[1][1] = p->colormap[1][2] = 0;
    remain = w & 7;

    if (p->type == _BITMAP_LSBFirst) {
      for (j = 0; j < h; j++) {
	for (i = 0; i < (w >> 3); i++) {
	  b = ss[i];
	  for (k = 8; k > 0; k--) {
	    *dd++ = b & 1;
	    b >>= 1;
	  }
	}
	if (remain) {
	  b = ss[i];
	  for (k = remain; k > 0; k--) {
	    *dd++ = b & 1;
	    b >>= 1;
	  }
	}
	ss += obpl;
      }
    } else {
      for (j = 0; j < h; j++) {
	for (i = 0; i < (w >> 3); i++) {
	  b = ss[i];
	  for (k = 8; k > 0; k--) {
	    *dd++ = (b & 0x80) ? 1 : 0;
	    b <<= 1;
	  }
	}
	if (remain) {
	  b = ss[i];
	  for (k = remain; k > 0; k--) {
	    *dd++ = (b & 0x80) ? 1 : 0;
	    b <<= 1;
	  }
	}
	ss += obpl;
      }
    }
    p->type = _INDEX;
    memory_destroy(m);
  }

  bytes_per_line_s = image_bpl_by_index(p, src);

  if (!xi->use_xv) {
    switch (ximage->bits_per_pixel) {
    case 16:
      {
	unsigned short int pix;

	bits_per_pixel = 16;

	if (p->type == _RGB_WITH_BITMASK) {
	  ximage->byte_order = MSBFirst;
	  dest = memory_ptr(dst_img);
	  break;
	} else if (p->type == _BGR_WITH_BITMASK) {
	  ximage->byte_order = LSBFirst;
	  dest = memory_ptr(dst_img);
	  break;
	}

	if (memory_alloc(dst_img, ximage->bytes_per_line * h) == NULL)
	  fatal("%s: No enough memory(alloc)\n", __FUNCTION__);
	dest = memory_ptr(dst_img);
	s = memory_ptr(src_img);

	ximage->byte_order = LSBFirst;
	switch (p->type) {
	case _BGR24:
	  for (j = 0; j < h; j++) {
	    d = dest + j * ximage->bytes_per_line;
	    for (i = 0; i < w; i++) {
	      pix =
		((s[i * 3 + 2] & 0xf8) << 8) |
		((s[i * 3 + 1] & 0xfc) << 3) |
		((s[i * 3    ] & 0xf8) >> 3);
	      *d++ = pix & 0xff;
	      *d++ = pix >> 8;
	    }
	    s += bytes_per_line_s;
	  }
	  break;
	case _RGB24:
	  for (j = 0; j < h; j++) {
	    d = dest + j * ximage->bytes_per_line;
	    for (i = 0; i < w; i++) {
	      pix =
		((s[i * 3    ] & 0xf8) << 8) |
		((s[i * 3 + 1] & 0xfc) << 3) |
		((s[i * 3 + 2] & 0xf8) >> 3);
	      *d++ = pix & 0xff;
	      *d++ = pix >> 8;
	    }
	    s += bytes_per_line_s;
	  }
	  break;
	case _RGBA32:
	  for (j = 0; j < h; j++) {
	    d = dest + j * ximage->bytes_per_line;
	    for (i = 0; i < w; i++) {
	      pix =
		((s[i * 4    ] & 0xf8) << 8) |
		((s[i * 4 + 1] & 0xfc) << 3) |
		((s[i * 4 + 2] & 0xf8) >> 3);
	      *d++ = pix & 0xff;
	      *d++ = pix >> 8;
	    }
	    s += bytes_per_line_s;
	  }
	  break;
	case _ABGR32:
	  for (j = 0; j < h; j++) {
	    d = dest + j * ximage->bytes_per_line;
	    for (i = 0; i < w; i++) {
	      pix =
		((s[i * 4 + 3] & 0xf8) << 8) |
		((s[i * 4 + 2] & 0xfc) << 3) |
		((s[i * 4 + 1] & 0xf8) >> 3);
	      *d++ = pix & 0xff;
	      *d++ = pix >> 8;
	    }
	    s += bytes_per_line_s;
	  }
	  break;
	case _ARGB32:
	  for (j = 0; j < h; j++) {
	    d = dest + j * ximage->bytes_per_line;
	    for (i = 0; i < w; i++) {
	      pix =
		((s[i * 4 + 1] & 0xf8) << 8) |
		((s[i * 4 + 2] & 0xfc) << 3) |
		((s[i * 4 + 3] & 0xf8) >> 3);
	      *d++ = pix & 0xff;
	      *d++ = pix >> 8;
	    }
	    s += bytes_per_line_s;
	  }
	  break;
	case _BGRA32:
	  xi->bgra32to16(dest, s, w, h, ximage->bytes_per_line, bytes_per_line_s);
	  break;
	case _INDEX:
	  {
	    unsigned char *pal;

	    for (j = 0; j < h; j++) {
	      d = dest + j * ximage->bytes_per_line;
	      for (i = 0; i < w; i++) {
		pal = p->colormap[s[i]];
		pix =
		  ((pal[0] & 0xf8) << 8) |
		  ((pal[1] & 0xfc) << 3) |
		  ((pal[2] & 0xf8) >> 3);
		*d++ = pix & 0xff;
		*d++ = pix >> 8;
	      }
	      s += bytes_per_line_s;
	    }
	  }
	  break;
	default:
	  show_message("Cannot render image [type %s] so far. (bpp: %d)\n", image_type_to_string(p->type), ximage->bits_per_pixel);
	  break;
	}
      }
      break;
    case 24:
      {
	bits_per_pixel = 24;

	if (p->type == _RGB24) {
	  ximage->byte_order = MSBFirst;
	  dest = memory_ptr(dst_img);
	  break;
	} else if (p->type == _BGR24) {
	  ximage->byte_order = LSBFirst;
	  dest = memory_ptr(dst_img);
	  break;
	}

	if (memory_alloc(dst_img, ximage->bytes_per_line * h) == NULL)
	  fatal("%s: No enough memory(alloc)\n", __FUNCTION__);

	dest = memory_ptr(dst_img);
	s = memory_ptr(src_img);
	ximage->byte_order = LSBFirst;
	switch (p->type) {
	case _INDEX:
	  for (j = 0; j < h; j++) {
	    d = dest + j * ximage->bytes_per_line;
	    for (i = 0; i < w; i++) {
	      *d++ = p->colormap[s[i]][2];
	      *d++ = p->colormap[s[i]][1];
	      *d++ = p->colormap[s[i]][0];
	    }
	    s += bytes_per_line_s;
	  }
	  break;
	case _RGBA32:
	  for (j = 0; j < h; j++) {
	    d = dest + j * ximage->bytes_per_line;
	    for (i = 0; i < w; i++) {
	      *d++ = s[i * 4 + 2];
	      *d++ = s[i * 4 + 1];
	      *d++ = s[i * 4 + 0];
	    }
	    s += bytes_per_line_s;
	  }
	  break;
	case _BGRA32:
	  for (j = 0; j < h; j++) {
	    d = dest + j * ximage->bytes_per_line;
	    for (i = 0; i < w; i++) {
	      *d++ = s[i * 4 + 0];
	      *d++ = s[i * 4 + 1];
	      *d++ = s[i * 4 + 2];
	    }
	    s += bytes_per_line_s;
	  }
	  break;
	case _ARGB32:
	case _ABGR32:
	  show_message("render image [type %s] is not implemented yet. (bpp: %d)\n", image_type_to_string(p->type), ximage->bits_per_pixel);
	  break;
	default:
	  show_message("Cannot render image [type %s] so far. (bpp: %d)\n", image_type_to_string(p->type), ximage->bits_per_pixel);
	  break;
	}
      }
      break;
    case 32:
      {
	bits_per_pixel = 32;

	/* Don't use switch() */
	if (p->type == _RGB24) {
	  ximage->byte_order = MSBFirst;
	  bits_per_pixel = 24;
	  /* invalid */
	  ximage->bytes_per_line = bytes_per_line_s;
	  dest = memory_ptr(dst_img);
	  break;
	} else if (p->type == _BGR24) {
	  ximage->byte_order = LSBFirst;
	  bits_per_pixel = 24;
	  /* invalid */
	  ximage->bytes_per_line = bytes_per_line_s;
	  dest = memory_ptr(dst_img);
	  break;
	} else if (p->type == _ARGB32) {
	  ximage->byte_order = MSBFirst;
	  dest = memory_ptr(dst_img);
	  break;
	} else if (p->type == _BGRA32) {
	  ximage->byte_order = LSBFirst;
	  dest = memory_ptr(dst_img);
	  break;
	}

	if (memory_alloc(dst_img, ximage->bytes_per_line * h) == NULL)
	  fatal("%s: No enough memory(alloc)\n", __FUNCTION__);

	dest = memory_ptr(dst_img);
	switch (p->type) {
	case _RGBA32:
	  ximage->byte_order = MSBFirst;
	  memcpy(dest + 1, memory_ptr(src_img), memory_used(src_img) - 1);
	  break;
	case _ABGR32:
	  ximage->byte_order = LSBFirst;
	  memcpy(dest, memory_ptr(src_img) + 1, memory_used(src_img) - 1);
	  break;
	case _INDEX:
	  if (memory_alloc(dst_img, ximage->bytes_per_line * h) == NULL)
	    fatal("%s: No enough memory(alloc)\n", __FUNCTION__);

	  dest = memory_ptr(dst_img);
	  s = memory_ptr(src_img);
	  ximage->byte_order = LSBFirst;
	  for (j = 0; j < h; j++) {
	    d = dest + j * ximage->bytes_per_line;
	    for (i = 0; i < w; i++) {
	      *d++ = p->colormap[s[i]][2];
	      *d++ = p->colormap[s[i]][1];
	      *d++ = p->colormap[s[i]][0];
	      d++;
	    }
	    s += bytes_per_line_s;
	  }
	  break;
	default:
	  show_message("Cannot render image [type %s] so far. (bpp: %d)\n", image_type_to_string(p->type), ximage->bits_per_pixel);
	  break;
	}
      }
      break;
    default:
      show_message_fnc("ximage->bits_per_pixel = %d\n", ximage->bits_per_pixel);
      break;
    }
  } else {
#if defined(USE_XV)
    xvimage = xi->xvimage;
    i = 0;
#if 0
    debug_message_fnc("XvImage: id %04X (%d x %d) %d bytes, %d plane, %d bpl\n",
		  xvimage->id, xvimage->width, xvimage->height,
		  xvimage->data_size, xvimage->num_planes, xi->x11->xv->bits_per_pixel[xi->format_num]);
    debug_message_fnc("XvImage:  pitch/offset: ");
    for (i = 0; i < (unsigned int)xvimage->num_planes; i++)
      debug_message("%d/%d ", xvimage->pitches[i], xvimage->offsets[i]);
    debug_message("\n");

    if (memory_alloc(dst_img, xvimage->data_size) == NULL)
      fatal("%s: No enough memory(alloc)\n", __FUNCTION__);
    if (xvimage->num_planes == 3) {
      if (xvimage->pitches[0] == (int)w &&
	  xvimage->pitches[1] == (int)w >> 1 &&
	  xvimage->pitches[2] == (int)w >> 1) {
	debug_message_fnc("XvImage:  pitch OK\n");
	if (xvimage->offsets[0] == 0 &&
	    xvimage->offsets[1] == (int)(w * h) &&
	    xvimage->offsets[2] == (int)(w * h + ((w * h) >> 2))) {
	  debug_message_fnc("XvImage:  offset OK\n");
	} else {
	  fatal("%s: XvImage:  offset NG: %d %d %d <-> %d %d %d\n", __FUNCTION__,
		xvimage->offsets[0], xvimage->offsets[1], xvimage->offsets[2],
		0, w * h, w * h + ((w * h) >> 2));
	}
      } else {
	fatal("%s: XvImage:  pitch NG: %d %d %d <-> %d %d %d\n", __FUNCTION__,
	      xvimage->pitches[0], xvimage->pitches[1], xvimage->pitches[2],
	      w, w >> 1, w >> 1);
      }
    } else if (xvimage->num_planes == 1) {
      if (xvimage->pitches[0] == (int)w << 1) {
	debug_message_fnc("XvImage:  pitch OK\n");
	if (xvimage->offsets[0] == 0) {
	  debug_message_fnc("XvImage:  offset OK\n");
	} else {
	  fatal("%s: XvImage:  offset NG: %d <-> %d\n", __FUNCTION__, xvimage->offsets[0], 0);
	}
      } else {
	fatal("%s: XvImage:  pitch NG: %d <-> %d\n", __FUNCTION__,
	      xvimage->pitches[0], w << 1);
      }
    } else {
      fatal("%s: Unknown nplanes == %d\n", __FUNCTION__, xvimage->num_planes);
    }
#endif
#endif
  }

#ifdef USE_SHM
  xi->if_attached = 0;
  if (to_be_attached) {
    xi->shminfo->shmid = memory_shmid(dst_img);
    xi->shminfo->shmaddr = memory_ptr(dst_img);
    xi->shminfo->readOnly = False;
    x11_lock(xi->x11);
    XShmAttach(x11_display(xi->x11), xi->shminfo);
    x11_unlock(xi->x11);
    xi->if_attached = 1;
  }
#endif

  if (xi->use_xv) {
#ifdef USE_XV
    xvimage->data = memory_ptr(dst_img);
#endif
  } else {
    ximage->data = (char *)dest;
    ximage->bits_per_pixel = bits_per_pixel;
  }

  //debug_message_fnc("data %p bpp %d bpl %d\n", ximage->data, ximage->bits_per_pixel, ximage->bytes_per_line);

  return 1;
}

static void
put(X11XImage *xi, Pixmap pix, GC gc, int sx, int sy, int dx, int dy, unsigned int w, unsigned int h)
{
  x11_lock(xi->x11);
#ifdef USE_SHM
  if (xi->if_attached) {
#ifdef USE_PTHREAD
    if (xi->use_xv) {
#ifdef USE_XV
      if (xi->xvimage) {
	if (xi->xvimage->width >= (int)w && xi->xvimage->height >= (int)h) {
	  XvShmPutImage(x11_display(xi->x11), xi->x11->xv->image_port, pix, gc, xi->xvimage, sx, sy, w, h, dx, dy, w, h, False);
	  XSync(x11_display(xi->x11), False);
	}
      }
#endif /* USE_XV */
    } else {
      if (xi->ximage) {
	if (xi->ximage->width >= (int)w && xi->ximage->height >= (int)h) {
	  XShmPutImage(x11_display(xi->x11), pix, gc, xi->ximage, sx, sy, dx, dy, w, h, False);
	  XSync(x11_display(xi->x11), False);
	}
      }
    }
#else /* USE_PTHREAD */
    /* delayed sync, intentionally. */
    if (x11->use_xv) {
#ifdef USE_XV
      if (xi->xvimage) {
	if (xi->xvimage->width >= (int)w && xi->xvimage->height >= (int)h) {
	  XSync(x11_display(xi->x11), False);
	  XvShmPutImage(x11_display(xi->x11), xi->x11->xv->image_port, pix, gc, xi->xvimage, sx, sy, w, h, dx, dy, w, h, Faluse);
	}
      }
#endif
    } else {
      if (xi->ximage) {
	if (xi->ximage->width >= (int)w && xi->ximage->height >= (int)h) {
	  XSync(x11_display(xi->x11), False);
	  XShmPutImage(x11_display(xi->x11), pix, gc, xi->ximage, sx, sy, dx, dy, w, h, False);
	}
      }
    }
#endif /* USE_PTHREAD */
  } else {
#endif /* USE_SHM */
    if (xi->use_xv) {
#ifdef USE_XV
      if (xi->xvimage)
	if (xi->xvimage->width >= (int)w && xi->xvimage->height >= (int)h) {
	  XvPutImage(x11_display(xi->x11), xi->x11->xv->image_port, pix, gc, xi->xvimage, sx, sy, w, h, dx, dy, w, h);
	}
#endif
    } else {
      if (xi->ximage)
	if (xi->ximage->width >= (int)w && xi->ximage->height >= (int)h) {
	  XPutImage(x11_display(xi->x11), pix, gc, xi->ximage, sx, sy, dx, dy, w, h);
	}
    }
#ifdef USE_SHM
  }
#endif
  x11_unlock(xi->x11);
}

static void
put_scaled(X11XImage *xi, Pixmap pix, GC gc, int sx, int sy, int dx, int dy, unsigned int sw, unsigned int sh, unsigned int dw, unsigned int dh)
{
  if (!xi->use_xv) {
    show_message_fnc("Needs XvImage, not XImage.\n");
    return;
  }

  x11_lock(xi->x11);
#ifdef USE_XV
#ifdef USE_SHM
  if (xi->if_attached) {
#ifdef USE_PTHREAD
    if (xi->xvimage) {
      if (xi->xvimage->width >= (int)sw && xi->xvimage->height >= (int)sh) {
	XvShmPutImage(x11_display(xi->x11), xi->x11->xv->image_port, pix, gc, xi->xvimage, sx, sy, sw, sh, dx, dy, dw, dh, False);
	XSync(x11_display(xi->x11), False);
      }
    }
#else /* USE_PTHREAD */
    /* delayed sync, intentionally. */
    if (xi->xvimage) {
      if (xi->xvimage->width >= (int)sw && xi->xvimage->height >= (int)sh) {
	XSync(x11_display(xi->x11), False);
	XvShmPutImage(x11_display(xi->x11), xi->x11->xv->image_port, pix, gc, xi->xvimage, sx, sy, sw, sh, dx, dy, dw, dh, False);
      }
    }
#endif /* USE_PTHREAD */
  } else {
#endif /* USE_SHM */
    if (xi->xvimage) {
      if (xi->xvimage->width >= (int)sw && xi->xvimage->height >= (int)sh) {
	XvPutImage(x11_display(xi->x11), xi->x11->xv->image_port, pix, gc, xi->xvimage, sx, sy, sw, sh, dx, dy, dw, dh);
      } else {
	show_message_fnc("width %d <=> %d sw, height %d <=> %d sh\n", xi->xvimage->width, sw, xi->xvimage->height, sh);
      }
    }
#ifdef USE_SHM
  }
#endif
#endif
  x11_unlock(xi->x11);
}

static void
destroy(X11XImage *xi)
{
  destroy_ximage(xi);
#ifdef USE_SHM
  if (xi->shminfo)
    free(xi->shminfo);
#endif
  free(xi);
}
