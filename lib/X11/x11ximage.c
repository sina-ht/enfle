/*
 * x11ximage.c -- X11 XImage related functions
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file if part of Enfle.
 *
 * Last Modified: Thu Dec 28 07:21:25 2000.
 * $Id: x11ximage.c,v 1.18 2000/12/27 23:29:29 sian Exp $
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

#include "x11ximage.h"

#ifdef WITH_NASM
void bgra32to16(unsigned char *dest, unsigned char *src, int width, int height);
#endif

static int convert(X11XImage *, Image *);
static void put(X11XImage *, Pixmap, GC, int, int, int, int, unsigned int, unsigned int);
static void destroy(X11XImage *);

static X11XImage template = {
  convert: convert,
  put: put,
  destroy: destroy
};

X11XImage *
x11ximage_create(X11 *x11)
{
  X11XImage *xi;

  if ((xi = calloc(1, sizeof(X11XImage))) == NULL)
    return NULL;
  memcpy(xi, &template, sizeof(X11XImage));
  xi->x11 = x11;

  return xi;
}

/* for internal use */

static void
destroy_ximage(X11XImage *xi)
{
  if (xi->ximage) {
    xi->ximage->data = NULL;
#ifdef USE_SHM
    if (xi->if_attached) {
      XShmDetach(x11_display(xi->x11), &xi->shminfo);
      xi->if_attached = 0;
      debug_message(__FUNCTION__ ": SHM detached\n");
    }
#endif
    x11_destroy_ximage(xi->ximage);
  }
}

/* methods */

static int
convert(X11XImage *xi, Image *p)
{
  int i, j;
  int bits_per_pixel = 0;
  int bytes_per_line_s;
#ifdef USE_SHM
  int to_be_attached = 0;
#endif
  unsigned char *dest = NULL, *dd, *s;
  XImage *ximage;
  Memory *to_be_rendered;
  unsigned int w, h;

  if (p->if_magnified) {
    to_be_rendered = p->magnified.image;
    bytes_per_line_s = p->magnified.bytes_per_line;
    w = p->magnified.width;
    h = p->magnified.height;
  } else {
    to_be_rendered = p->image;
    bytes_per_line_s = p->bytes_per_line;
    w = p->width;
    h = p->height;
  }

  p->rendered.width  = w;
  p->rendered.height = h;

  if (!xi->ximage ||
      xi->ximage->width != w || xi->ximage->height != h ||
      (memory_type(p->rendered.image) == _SHM && xi->shminfo.shmid != memory_shmid(p->rendered.image))) {
    destroy_ximage(xi);
    switch (memory_type(p->rendered.image)) {
    case _NORMAL:
      xi->ximage =
	x11_create_ximage(xi->x11, x11_visual(xi->x11), x11_depth(xi->x11),
			  memory_ptr(p->rendered.image), w, h, 8, 0);
      break;
    case _SHM:
#ifdef USE_SHM
      xi->ximage =
	XShmCreateImage(x11_display(xi->x11), x11_visual(xi->x11), x11_depth(xi->x11), ZPixmap, NULL,
			&xi->shminfo, w, h);
      to_be_attached = 1;
      XSync(x11_display(xi->x11), False);
#else
      fatal(3, "No SHM support. Should not be reached here.\n");
#endif
      break;
    default:
      return 0;
    }
    /* debug_message("ximage->bpl = %d\n", xi->ximage->bytes_per_line); */
  }

  ximage = xi->ximage;
  p->rendered.bytes_per_line = ximage->bytes_per_line;

#if 0
  debug_message("x order %s\n", ximage->byte_order == LSBFirst ? "LSB" : "MSB");
  debug_message("p type: %s p bpl: %d x bpl: %d\n", image_type_to_string(p->type), p->bits_per_pixel, ximage->bits_per_pixel);
#endif

  /* _GRAY -> _INDEX */
  if (p->type == _GRAY) {
    for (i = 0; i < p->ncolors; i++)
      p->colormap[i][0] = p->colormap[i][1] = p->colormap[i][2] = 255 * i / p->ncolors;
    p->type = _INDEX;
  }

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
	fatal(2, __FUNCTION__ ": No enough memory(alloc)\n");

      dd = dest = memory_ptr(p->rendered.image);
      s = memory_ptr(to_be_rendered);

      ximage->byte_order = LSBFirst;
      switch (p->type) {
      case _BGR24:
	for (j = 0; j < h; j++) {
	  dd = dest + j * ximage->bytes_per_line;
	  for (i = 0; i < w; i++) {
	    pix =
	      ((s[i * 3 + 2] & 0xf8) << 8) |
	      ((s[i * 3 + 1] & 0xfc) << 3) |
	      ((s[i * 3    ] & 0xf8) >> 3);
	    *dd++ = pix & 0xff;
	    *dd++ = pix >> 8;
	  }
	  s += bytes_per_line_s;
	}
	break;
      case _RGB24:
	for (j = 0; j < h; j++) {
	  dd = dest + j * ximage->bytes_per_line;
	  for (i = 0; i < w; i++) {
	    pix =
	      ((s[i * 3    ] & 0xf8) << 8) |
	      ((s[i * 3 + 1] & 0xfc) << 3) |
	      ((s[i * 3 + 2] & 0xf8) >> 3);
	    *dd++ = pix & 0xff;
	    *dd++ = pix >> 8;
	  }
	  s += bytes_per_line_s;
	}
	break;
      case _RGBA32:
	for (j = 0; j < h; j++) {
	  dd = dest + j * ximage->bytes_per_line;
	  for (i = 0; i < w; i++) {
	    pix =
	      ((s[i * 4    ] & 0xf8) << 8) |
	      ((s[i * 4 + 1] & 0xfc) << 3) |
	      ((s[i * 4 + 2] & 0xf8) >> 3);
	    *dd++ = pix & 0xff;
	    *dd++ = pix >> 8;
	  }
	  s += bytes_per_line_s;
	}
	break;
      case _ABGR32:
	for (j = 0; j < h; j++) {
	  dd = dest + j * ximage->bytes_per_line;
	  for (i = 0; i < w; i++) {
	    pix =
	      ((s[i * 4 + 3] & 0xf8) << 8) |
	      ((s[i * 4 + 2] & 0xfc) << 3) |
	      ((s[i * 4 + 1] & 0xf8) >> 3);
	    *dd++ = pix & 0xff;
	    *dd++ = pix >> 8;
	  }
	  s += bytes_per_line_s;
	}
	break;
      case _ARGB32:
	for (j = 0; j < h; j++) {
	  dd = dest + j * ximage->bytes_per_line;
	  for (i = 0; i < w; i++) {
	    pix =
	      ((s[i * 4 + 1] & 0xf8) << 8) |
	      ((s[i * 4 + 2] & 0xfc) << 3) |
	      ((s[i * 4 + 3] & 0xf8) >> 3);
	    *dd++ = pix & 0xff;
	    *dd++ = pix >> 8;
	  }
	  s += bytes_per_line_s;
	}
	break;
      case _BGRA32:
#ifdef WITH_NASM
	if (ximage->bytes_per_line == w * 2)
	  bgra32to16(dest, s, w, h);
	else {
#endif
	for (j = 0; j < h; j++) {
	  dd = dest + j * ximage->bytes_per_line;
	  for (i = 0; i < w; i++) {
	    pix =
	      ((s[i * 4 + 2] & 0xf8) << 8) |
	      ((s[i * 4 + 1] & 0xfc) << 3) |
	      ((s[i * 4 + 0] & 0xf8) >> 3);
	    *dd++ = pix & 0xff;
	    *dd++ = pix >> 8;
	  }
	  s += bytes_per_line_s;
	}
#ifdef WITH_NASM
	}
#endif
	break;
      case _INDEX:
	{
	  unsigned char *pal;

	  for (j = 0; j < h; j++) {
	    dd = dest + j * ximage->bytes_per_line;
	    for (i = 0; i < w; i++) {
	      pal = p->colormap[s[i]];
	      pix =
		((pal[0] & 0xf8) << 8) |
		((pal[1] & 0xfc) << 3) |
		((pal[2] & 0xf8) >> 3);
	      *dd++ = pix & 0xff;
	      *dd++ = pix >> 8;
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
      int i;

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
	fatal(2, __FUNCTION__ ": No enough memory(alloc)\n");

      dd = dest = memory_ptr(p->rendered.image);
      s = memory_ptr(to_be_rendered);
      ximage->byte_order = LSBFirst;
      switch (p->type) {
      case _INDEX:
	for (j = 0; j < h; j++) {
	  dd = dest + j * ximage->bytes_per_line;
	  for (i = 0; i < w; i++) {
	    *dd++ = p->colormap[s[i]][2];
	    *dd++ = p->colormap[s[i]][1];
	    *dd++ = p->colormap[s[i]][0];
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
      int i;

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
	fatal(2, __FUNCTION__ ": No enough memory(alloc)\n");

      dest = memory_ptr(p->rendered.image);
      switch (p->type) {
      case _RGBA32:
	ximage->byte_order = MSBFirst;
	memcpy(dest + 1, memory_ptr(to_be_rendered), memory_size(to_be_rendered) - 1);
	break;
      case _ABGR32:
	ximage->byte_order = LSBFirst;
	memcpy(dest, memory_ptr(to_be_rendered) + 1, memory_size(to_be_rendered) - 1);
	break;
      case _INDEX:
	if (memory_alloc(p->rendered.image, ximage->bytes_per_line * h) == NULL)
	  fatal(2, __FUNCTION__ ": No enough memory(alloc)\n");

	dest = memory_ptr(p->rendered.image);
	s = memory_ptr(to_be_rendered);
	dd = dest;
	ximage->byte_order = LSBFirst;
	for (j = 0; j < h; j++) {
	  dd = dest + j * ximage->bytes_per_line;
	  for (i = 0; i < w; i++) {
	    *dd++ = p->colormap[s[i]][2];
	    *dd++ = p->colormap[s[i]][1];
	    *dd++ = p->colormap[s[i]][0];
	    dd++;
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
    show_message(__FUNCTION__ ": ximage->bits_per_pixel = %d\n", ximage->bits_per_pixel);
    break;
  }

#ifdef USE_SHM
  if (to_be_attached) {
    xi->shminfo.shmid = memory_shmid(p->rendered.image);
    xi->shminfo.shmaddr = memory_ptr(p->rendered.image);
    xi->shminfo.readOnly = False;
    XShmAttach(x11_display(xi->x11), &xi->shminfo);
    xi->if_attached = 1;
    debug_message(__FUNCTION__": SHM attached\n");
  }
#endif

  ximage->data = dest;
  ximage->bits_per_pixel = bits_per_pixel;

  return 1;
}

static void
put(X11XImage *xi, Pixmap pix, GC gc, int sx, int sy, int dx, int dy, unsigned int w, unsigned int h)
{
#ifdef USE_SHM
  if (xi->if_attached) {
#ifdef USE_PTHREAD
    XShmPutImage(x11_display(xi->x11), pix, gc, xi->ximage, sx, sy, dx, dy, w, h, False);
    XSync(x11_display(xi->x11), False);
#else
    /* delayed sync, intentionally. */
    XSync(x11_display(xi->x11), False);
    XShmPutImage(x11_display(xi->x11), pix, gc, xi->ximage, sx, sy, dx, dy, w, h, False);
#endif
  } else
#endif
    XPutImage(x11_display(xi->x11), pix, gc, xi->ximage, sx, sy, dx, dy, w, h);
}

static void
destroy(X11XImage *xi)
{
  destroy_ximage(xi);
  free(xi);
}
