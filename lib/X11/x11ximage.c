/*
 * x11ximage.c -- X11 XImage related functions
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file if part of Enfle.
 *
 * Last Modified: Sun Dec  3 19:58:15 2000.
 * $Id: x11ximage.c,v 1.9 2000/12/03 11:04:07 sian Exp $
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
      debug_message(__FUNCTION__": SHM detached\n");
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
  int bytes_per_line = 0, bits_per_pixel = 0;
  unsigned char *dest = NULL, *dd, *s;
  XImage *ximage;
  Memory *source = NULL;

  if (!xi->ximage || xi->ximage->width != p->width || xi->ximage->height != p->height) {
    destroy_ximage(xi);
    switch (memory_type(p->image)) {
    case _NORMAL:
      xi->ximage =
	x11_create_ximage(xi->x11, x11_visual(xi->x11), x11_depth(xi->x11), memory_ptr(p->image),
			  p->width, p->height, 8, 0);
      break;
    case _SHM:
#ifdef USE_SHM
      xi->ximage =
	XShmCreateImage(x11_display(xi->x11), x11_visual(xi->x11), x11_depth(xi->x11), ZPixmap, NULL,
			&xi->shminfo, p->width, p->height);
      xi->shminfo.shmid = memory_shmid(p->image);
      xi->shminfo.shmaddr = memory_ptr(p->image);
      xi->shminfo.readOnly = False;
      XShmAttach(x11_display(xi->x11), &xi->shminfo);
      xi->if_attached = 1;
      debug_message(__FUNCTION__": SHM attached\n");
#else
      show_message("No SHM support. Should not be reached here.\n");
      exit(-3);
#endif
      break;
    default:
      return 0;
    }
  }

  ximage = xi->ximage;

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
      bytes_per_line = p->width * 2;

      if (p->type == _RGB_WITH_BITMASK) {
	ximage->byte_order = MSBFirst;
	dest = memory_ptr(p->image);
	break;
      } else if (p->type == _BGR_WITH_BITMASK) {
	ximage->byte_order = LSBFirst;
	dest = memory_ptr(p->image);
	break;
      }

      if ((source = memory_dup(p->image)) == NULL) {
	show_message(__FUNCTION__ ": No enough memory(source)\n");
	exit(-2);
      }

      if (memory_alloc(p->image, p->width * p->height * 2) == NULL) {
	show_message(__FUNCTION__ ": No enough memory(alloc)\n");
	exit(-2);
      }

      dd = dest = memory_ptr(p->image);
      s = memory_ptr(source);
      ximage->byte_order = LSBFirst;
      switch (p->type) {
      case _BGR24:
	for (i = 0; i < p->width * p->height; i++) {
	  pix =
	    ((s[i * 3 + 2] & 0xf8) << 8) |
	    ((s[i * 3 + 1] & 0xfc) << 3) |
	    ((s[i * 3    ] & 0xf8) >> 3);
	  *dd++ = pix & 0xff;
	  *dd++ = pix >> 8;
	}
	break;
      case _RGB24:
	for (i = 0; i < p->width * p->height; i++) {
	  pix =
	    ((s[i * 3    ] & 0xf8) << 8) |
	    ((s[i * 3 + 1] & 0xfc) << 3) |
	    ((s[i * 3 + 2] & 0xf8) >> 3);
	  *dd++ = pix & 0xff;
	  *dd++ = pix >> 8;
	}
	break;
      case _RGBA32:
	for (i = 0; i < p->width * p->height; i++) {
	  pix =
	    ((s[i * 4    ] & 0xf8) << 8) |
	    ((s[i * 4 + 1] & 0xfc) << 3) |
	    ((s[i * 4 + 2] & 0xf8) >> 3);
	  *dd++ = pix & 0xff;
	  *dd++ = pix >> 8;
	}
	break;
      case _ABGR32:
	for (i = 0; i < p->width * p->height; i++) {
	  pix =
	    ((s[i * 4 + 3] & 0xf8) << 8) |
	    ((s[i * 4 + 2] & 0xfc) << 3) |
	    ((s[i * 4 + 1] & 0xf8) >> 3);
	  *dd++ = pix & 0xff;
	  *dd++ = pix >> 8;
	}
	break;
      case _ARGB32:
	for (i = 0; i < p->width * p->height; i++) {
	  pix =
	    ((s[i * 4 + 1] & 0xf8) << 8) |
	    ((s[i * 4 + 2] & 0xfc) << 3) |
	    ((s[i * 4 + 3] & 0xf8) >> 3);
	  *dd++ = pix & 0xff;
	  *dd++ = pix >> 8;
	}
	break;
      case _BGRA32:
#ifdef WITH_NASM
	bgra32to16(dest, s, p->width, p->height);
#else
	for (i = 0; i < p->width * p->height; i++) {
	  pix =
	    ((s[i * 4 + 2] & 0xf8) << 8) |
	    ((s[i * 4 + 1] & 0xfc) << 3) |
	    ((s[i * 4 + 0] & 0xf8) >> 3);
	  *dd++ = pix & 0xff;
	  *dd++ = pix >> 8;
	}
#endif
	break;
      case _INDEX:
	{
	  unsigned char *pal;

	  for (i = 0; i < p->bytes_per_line * p->height; i += p->bytes_per_line) {
	    for (j = 0; j < p->width; j++) {
	      pal = p->colormap[s[i + j]];
	      pix =
		((pal[0] & 0xf8) << 8) |
		((pal[1] & 0xfc) << 3) |
		((pal[2] & 0xf8) >> 3);
	      *dd++ = pix & 0xff;
	      *dd++ = pix >> 8;
	    }
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
      bytes_per_line = p->width * 3;

      if (p->type == _RGB24) {
	ximage->byte_order = MSBFirst;
	dest = memory_ptr(p->image);
	break;
      } else if (p->type == _BGR24) {
	ximage->byte_order = LSBFirst;
	dest = memory_ptr(p->image);
	break;
      }

      if ((source = memory_dup(p->image)) == NULL) {
	show_message(__FUNCTION__ ": No enough memory(source)\n");
	exit(-2);
      }

      if (memory_alloc(p->image, p->width * p->height * 3) == NULL) {
	show_message(__FUNCTION__ ": No enough memory(alloc)\n");
	exit(-2);
      }

      dd = dest = memory_ptr(p->image);
      s = memory_ptr(source);
      ximage->byte_order = LSBFirst;
      switch (p->type) {
      case _INDEX:
	for (i = 0; i < p->bytes_per_line * p->height; i += p->bytes_per_line) {
	  for (j = 0; j < p->width; j++) {
	    *dd++ = p->colormap[s[i + j]][2];
	    *dd++ = p->colormap[s[i + j]][1];
	    *dd++ = p->colormap[s[i + j]][0];
	  }
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

      bytes_per_line = p->bytes_per_line;
      bits_per_pixel = 32;
      /* Don't use switch() */
      if (p->type == _RGB24) {
	ximage->byte_order = MSBFirst;
	bits_per_pixel = 24;
	dest = memory_ptr(p->image);
	break;
      } else if (p->type == _BGR24) {
	ximage->byte_order = LSBFirst;
	bits_per_pixel = 24;
	dest = memory_ptr(p->image);
	break;
      } else if (p->type == _ARGB32) {
	ximage->byte_order = MSBFirst;
	dest = memory_ptr(p->image);
	break;
      } else if (p->type == _BGRA32) {
	ximage->byte_order = LSBFirst;
	dest = memory_ptr(p->image);
	break;
      }

      dest = memory_ptr(p->image);
      bytes_per_line = p->width << 2;
      switch (p->type) {
      case _RGBA32:
	ximage->byte_order = MSBFirst;
	memcpy(dest + 1, dest, memory_size(p->image) - 1);
	break;
      case _ABGR32:
	ximage->byte_order = LSBFirst;
	memcpy(dest, dest + 1, memory_size(p->image) - 1);
	break;
      case _INDEX:
	if (memory_alloc(p->image, p->width * p->height * 4) == NULL) {
	  show_message(__FUNCTION__ ": No enough memory(alloc)\n");
	  exit(-2);
	}

	if ((source = memory_dup(p->image)) == NULL) {
	  show_message(__FUNCTION__ ": No enough memory(source)\n");
	  exit(-2);
	}

	s = memory_ptr(source);
	dd = dest;
	ximage->byte_order = LSBFirst;
	for (i = 0; i < p->width * p->height; i++) {
	  *dd++ = p->colormap[s[i]][2];
	  *dd++ = p->colormap[s[i]][1];
	  *dd++ = p->colormap[s[i]][0];
	  dd++;
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

  if (source)
    memory_destroy(source);

  ximage->data = dest;
  ximage->bytes_per_line = bytes_per_line;
  ximage->bits_per_pixel = bits_per_pixel;

  return 1;
}

static void
put(X11XImage *xi, Pixmap pix, GC gc, int sx, int sy, int dx, int dy, unsigned int w, unsigned int h)
{
#ifdef USE_SHM
  if (xi->if_attached) {
    XSync(x11_display(xi->x11), False);
    XShmPutImage(x11_display(xi->x11), pix, gc, xi->ximage, sx, sy, dx, dy, w, h, False);
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
