/*
 * x11ximage.c -- X11 XImage related functions
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file if part of Enfle.
 *
 * Last Modified: Thu Jun 13 23:31:31 2002.
 * $Id: x11ximage.c,v 1.46 2002/06/13 14:32:30 sian Exp $
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
#define REQUIRE_FATAL
#include "common.h"

#include "x11ximage.h"

void bgra32to16_generic(unsigned char *, unsigned char *, unsigned int, unsigned int, unsigned int, unsigned int);
#ifdef USE_MMX
void bgra32to16_maybe_mmx(unsigned char *, unsigned char *, unsigned int, unsigned int, unsigned int, unsigned int);
void bgra32to16_mmx(unsigned char *, unsigned char *, unsigned int, unsigned int);
#endif

static int convert(X11XImage *, Image *);
static void put(X11XImage *, Pixmap, GC, int, int, int, int, unsigned int, unsigned int);
static void put_scaled(X11XImage *, Pixmap, GC, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int);
static void destroy(X11XImage *);

static X11XImage template = {
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
convert(X11XImage *xi, Image *p)
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
  Memory *to_be_rendered;
  unsigned int w, h;

  if (p->if_magnified) {
    to_be_rendered = p->magnified.image;
    w = p->magnified.width;
    h = p->magnified.height;
  } else {
    to_be_rendered = p->image;
    w = p->width;
    h = p->height;
  }

  p->rendered.width  = w;
  p->rendered.height = h;

  switch (p->type) {
#ifdef USE_XV
  case _YUY2:
    //debug_message("Image provided in YUY2.\n");
    if (xi->x11->xv->capable_format & XV_YUY2_FLAG) {
      t = XV_YUY2;
      xi->use_xv = 1;
    } else {
      fatal(4, "Xv cannot display YUY2...\n");
    }
    break;
  case _YV12:
    //debug_message("Image provided in YV12.\n");
    if (xi->x11->xv->capable_format & XV_YV12_FLAG) {
      t = XV_YV12;
      xi->use_xv = 1;
    } else {
      fatal(4, "Xv cannot display YV12...\n");
    }
    break;
  case _I420:
    //debug_message("Image provided in I420.\n");
    if (xi->x11->xv->capable_format & XV_I420_FLAG) {
      t = XV_I420;
      xi->use_xv = 1;
    } else {
      fatal(4, "Xv cannot display I420...\n");
    }
    break;
  case _UYVY:
    //debug_message("Image provided in UYVY.\n");
    if (xi->x11->xv->capable_format & XV_UYVY_FLAG) {
      t = XV_UYVY;
      xi->use_xv = 1;
    } else {
      fatal(4, "Xv cannot display UYVY...\n");
    }
    break;
  case _RV15:
    //debug_message("Image provided in RV15.\n");
    if (xi->x11->xv->capable_format & XV_RV15_FLAG) {
      t = XV_RV15;
      xi->use_xv = 1;
    } else {
      fatal(4, "Xv cannot display RV15...\n");
    }
    break;
  case _RV16:
    //debug_message("Image provided in RV16.\n");
    if (xi->x11->xv->capable_format & XV_RV16_FLAG) {
      t = XV_RV16;
      xi->use_xv = 1;
    } else {
      fatal(4, "Xv cannot display RV16...\n");
    }
    break;
  case _RV24:
    debug_message("Image provided in RV24.\n");
    if (xi->x11->xv->capable_format & XV_RV24_FLAG) {
      t = XV_RV24;
      xi->use_xv = 1;
    } else {
      fatal(4, "Xv cannot display RV24...\n");
    }
    break;
  case _RV32:
    //debug_message("Image provided in RV32.\n");
    if (xi->x11->xv->capable_format & XV_RV32_FLAG) {
      t = XV_RV32;
      xi->use_xv = 1;
    } else {
      fatal(4, "Xv cannot display RV32...\n");
    }
    break;
#else
  case _YUY2:
  case _YV12:
  case _I420:
  case _UYVY:
  case _RV15:
  case _RV16:
  case _RV24:
  case _RV32:
    fatal(4, "Xv cannot display ...\n");
    break;
#endif
  default:
    xi->use_xv = 0;
    break;
  }

#ifdef USE_XV
  if (xi->use_xv && xi->xvimage) {
    if (xi->xvimage->width != (int)w || xi->xvimage->height != (int)h || p->type != xi->type
#ifdef USE_SHM
      || (memory_type(p->rendered.image) == _SHM && xi->shminfo->shmid != memory_shmid(p->rendered.image))
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
      || (memory_type(p->rendered.image) == _SHM && xi->shminfo->shmid != memory_shmid(p->rendered.image))
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
    switch (memory_type(p->rendered.image)) {
    case _NORMAL:
#ifdef USE_XV
      if (xi->use_xv) {
	if ((xi->xvimage = x11_xv_create_ximage(xi->x11, xi->x11->xv->image_port, xi->x11->xv->format_ids[t], memory_ptr(p->rendered.image), w, h)))
	  xi->format_num = t;
      }
      if (!xi->use_xv || xi->xvimage == NULL) {
#endif
	xi->ximage =
	  x11_create_ximage(xi->x11, x11_visual(xi->x11), x11_depth(xi->x11),
			    memory_ptr(p->rendered.image), w, h, 8, 0);
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
      fatal(3, "No SHM support. Should not be reached here.\n");
#endif /* USE_SHM */
      break;
    default:
      return 0;
    }
    /* debug_message("ximage->bpl = %d\n", xi->ximage->bytes_per_line); */
    x11_unlock(xi->x11);
  }

  if (!xi->use_xv) {
    ximage = xi->ximage;
    p->rendered.bytes_per_line = ximage->bytes_per_line;
#if 0
    debug_message("x order %s\n", ximage->byte_order == LSBFirst ? "LSB" : "MSB");
    debug_message("p type: %s p bpl: %d x bpl: %d\n", image_type_to_string(p->type), p->bits_per_pixel, ximage->bits_per_pixel);
#endif
  }

  /* _GRAY -> _INDEX */
  if (p->type == _GRAY) {
    for (i = 0; i < p->ncolors; i++)
      p->colormap[i][0] = p->colormap[i][1] = p->colormap[i][2] = 255 * i / (p->ncolors - 1);
    p->type = _INDEX;
  }

  /* _BITMAP_* -> _INDEX */
  if (p->type == _BITMAP_LSBFirst || p->type == _BITMAP_MSBFirst) {
    Memory *m;
    unsigned char *ss, *dd, b;
    int k, obpl, remain;

    m = memory_dup(p->image);
    ss = memory_ptr(m);
    obpl = p->bytes_per_line;
    p->bytes_per_line = p->width;
    p->depth = p->bits_per_pixel = 8;
    p->ncolors = 2;
    if ((dd = memory_alloc(p->image, p->bytes_per_line * p->height)) == NULL)
      fatal(2, "%s: No enough memory(alloc)\n", __FUNCTION__);
    p->colormap[0][0] = p->colormap[0][1] = p->colormap[0][2] = 255;
    p->colormap[1][0] = p->colormap[1][1] = p->colormap[1][2] = 0;
    remain = p->width & 7;

    if (p->type == _BITMAP_LSBFirst) {
      for (j = 0; j < p->height; j++) {
	for (i = 0; i < (p->width >> 3); i++) {
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
      for (j = 0; j < p->height; j++) {
	for (i = 0; i < (p->width >> 3); i++) {
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

  bytes_per_line_s = p->if_magnified ? p->magnified.bytes_per_line : p->bytes_per_line;

  if (!xi->use_xv) {
    switch (ximage->bits_per_pixel) {
    case 16:
      {
	unsigned short int pix;

	bits_per_pixel = 16;

	if (p->type == _RGB_WITH_BITMASK) {
	  ximage->byte_order = MSBFirst;
	  dest = memory_ptr(p->rendered.image);
	  break;
	} else if (p->type == _BGR_WITH_BITMASK) {
	  ximage->byte_order = LSBFirst;
	  dest = memory_ptr(p->rendered.image);
	  break;
	}

	if (memory_alloc(p->rendered.image, ximage->bytes_per_line * h) == NULL)
	  fatal(2, "%s: No enough memory(alloc)\n", __FUNCTION__);
	dest = memory_ptr(p->rendered.image);
	s = memory_ptr(to_be_rendered);

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
	  show_message("Cannot render image [type %s] so far.\n", image_type_to_string(p->type));
	  break;
	}
      }
      break;
    case 24:
      {
	bits_per_pixel = 24;

	if (p->type == _RGB24) {
	  ximage->byte_order = MSBFirst;
	  dest = memory_ptr(p->rendered.image);
	  break;
	} else if (p->type == _BGR24) {
	  ximage->byte_order = LSBFirst;
	  dest = memory_ptr(p->rendered.image);
	  break;
	}

	if (memory_alloc(p->rendered.image, ximage->bytes_per_line * h) == NULL)
	  fatal(2, "%s: No enough memory(alloc)\n", __FUNCTION__);

	dest = memory_ptr(p->rendered.image);
	s = memory_ptr(to_be_rendered);
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
	case _BGRA32:
	case _ARGB32:
	case _ABGR32:
	  show_message("render image [type %s] is not implemented yet.\n", image_type_to_string(p->type));
	  break;
	default:
	  show_message("Cannot render image [type %s] so far.\n", image_type_to_string(p->type));
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
	  dest = memory_ptr(p->rendered.image);
	  break;
	} else if (p->type == _BGR24) {
	  ximage->byte_order = LSBFirst;
	  bits_per_pixel = 24;
	  /* invalid */
	  ximage->bytes_per_line = bytes_per_line_s;
	  dest = memory_ptr(p->rendered.image);
	  break;
	} else if (p->type == _ARGB32) {
	  ximage->byte_order = MSBFirst;
	  dest = memory_ptr(p->rendered.image);
	  break;
	} else if (p->type == _BGRA32) {
	  ximage->byte_order = LSBFirst;
	  dest = memory_ptr(p->rendered.image);
	  break;
	}

	if (memory_alloc(p->rendered.image, ximage->bytes_per_line * h) == NULL)
	  fatal(2, "%s: No enough memory(alloc)\n", __FUNCTION__);

	dest = memory_ptr(p->rendered.image);
	switch (p->type) {
	case _RGBA32:
	  ximage->byte_order = MSBFirst;
	  memcpy(dest + 1, memory_ptr(to_be_rendered), memory_used(to_be_rendered) - 1);
	  break;
	case _ABGR32:
	  ximage->byte_order = LSBFirst;
	  memcpy(dest, memory_ptr(to_be_rendered) + 1, memory_used(to_be_rendered) - 1);
	  break;
	case _INDEX:
	  if (memory_alloc(p->rendered.image, ximage->bytes_per_line * h) == NULL)
	    fatal(2, "%s: No enough memory(alloc)\n", __FUNCTION__);

	  dest = memory_ptr(p->rendered.image);
	  s = memory_ptr(to_be_rendered);
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
	  show_message("Cannot render image [type %s] so far.\n", image_type_to_string(p->type));
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

    if (memory_alloc(p->rendered.image, xvimage->data_size) == NULL)
      fatal(2, "%s: No enough memory(alloc)\n", __FUNCTION__);
    if (xvimage->num_planes == 3) {
      if (xvimage->pitches[0] == (int)p->width &&
	  xvimage->pitches[1] == (int)p->width >> 1 &&
	  xvimage->pitches[2] == (int)p->width >> 1) {
	debug_message_fnc("XvImage:  pitch OK\n");
	if (xvimage->offsets[0] == 0 &&
	    xvimage->offsets[1] == (int)(p->width * p->height) &&
	    xvimage->offsets[2] == (int)(p->width * p->height + ((p->width * p->height) >> 2))) {
	  debug_message_fnc("XvImage:  offset OK\n");
	} else {
	  fatal(4, "%s: XvImage:  offset NG: %d %d %d <-> %d %d %d\n", __FUNCTION__,
		xvimage->offsets[0], xvimage->offsets[1], xvimage->offsets[2],
		0, p->width * p->height, p->width * p->height + ((p->width * p->height) >> 2));
	}
      } else {
	fatal(4, "%s: XvImage:  pitch NG: %d %d %d <-> %d %d %d\n", __FUNCTION__,
	      xvimage->pitches[0], xvimage->pitches[1], xvimage->pitches[2],
	      p->width, p->width >> 1, p->width >> 1);
      }
    } else if (xvimage->num_planes == 1) {
      if (xvimage->pitches[0] == (int)p->width << 1) {
	debug_message_fnc("XvImage:  pitch OK\n");
	if (xvimage->offsets[0] == 0) {
	  debug_message_fnc("XvImage:  offset OK\n");
	} else {
	  fatal(4, "%s: XvImage:  offset NG: %d <-> %d\n", __FUNCTION__, xvimage->offsets[0], 0);
	}
      } else {
	fatal(4, "%s: XvImage:  pitch NG: %d <-> %d\n", __FUNCTION__,
	      xvimage->pitches[0], p->width << 1);
      }
    } else {
      fatal(4, "%s: Unknown nplanes == %d\n", __FUNCTION__, xvimage->num_planes);
    }
#endif
#endif
  }

#ifdef USE_SHM
  if (to_be_attached) {
    xi->shminfo->shmid = memory_shmid(p->rendered.image);
    xi->shminfo->shmaddr = memory_ptr(p->rendered.image);
    xi->shminfo->readOnly = False;
    x11_lock(xi->x11);
    XShmAttach(x11_display(xi->x11), xi->shminfo);
    x11_unlock(xi->x11);
    xi->if_attached = 1;
  }
#endif

  if (xi->use_xv) {
#ifdef USE_XV
    xvimage->data = memory_ptr(p->rendered.image);
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
