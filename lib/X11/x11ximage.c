/*
 * x11ximage.c -- X11 XImage related functions
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file if part of Enfle.
 *
 * Last Modified: Sat Nov 18 15:55:38 2000.
 * $Id: x11ximage.c,v 1.6 2000/11/20 12:52:23 sian Exp $
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

#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>

#include "common.h"

#include "x11ximage.h"

#ifdef WITH_NASM
void bgra32to16(unsigned char *dest, unsigned char *src, int width, int height);
#endif

void
x11ximage_convert_image(XImage *ximage, Image *p)
{
  int i, j;
  int bytes_per_line = 0, bits_per_pixel = 0;
  unsigned char *dest = NULL, *dd;

#if 0
  debug_message("x order %s\n", ximage->byte_order == LSBFirst ? "LSB" : "MSB");
  debug_message("p bpl: %d x bpl: %d\n", p->bits_per_pixel, ximage->bits_per_pixel);
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
	dest = p->image;
	break;
      } else if (p->type == _BGR_WITH_BITMASK) {
	ximage->byte_order = LSBFirst;
	dest = p->image;
	break;
      }

      if ((dest = calloc(2, p->width * p->height)) == NULL) {
	show_message("x11ximage_convert_image: No enough memory\n");
	exit(-2);
      }

      dd = dest;
      ximage->byte_order = LSBFirst;
      switch (p->type) {
      case _BGR24:
	for (i = 0; i < p->width * p->height; i++) {
	  pix =
	    ((p->image[i * 3 + 2] & 0xf8) << 8) |
	    ((p->image[i * 3 + 1] & 0xfc) << 3) |
	    ((p->image[i * 3    ] & 0xf8) >> 3);
	  *dd++ = pix & 0xff;
	  *dd++ = pix >> 8;
	}
	break;
      case _RGB24:
	for (i = 0; i < p->width * p->height; i++) {
	  pix =
	    ((p->image[i * 3    ] & 0xf8) << 8) |
	    ((p->image[i * 3 + 1] & 0xfc) << 3) |
	    ((p->image[i * 3 + 2] & 0xf8) >> 3);
	  *dd++ = pix & 0xff;
	  *dd++ = pix >> 8;
	}
	break;
      case _RGBA32:
	for (i = 0; i < p->width * p->height; i++) {
	  pix =
	    ((p->image[i * 4    ] & 0xf8) << 8) |
	    ((p->image[i * 4 + 1] & 0xfc) << 3) |
	    ((p->image[i * 4 + 2] & 0xf8) >> 3);
	  *dd++ = pix & 0xff;
	  *dd++ = pix >> 8;
	}
	break;
      case _ABGR32:
	for (i = 0; i < p->width * p->height; i++) {
	  pix =
	    ((p->image[i * 4 + 3] & 0xf8) << 8) |
	    ((p->image[i * 4 + 2] & 0xfc) << 3) |
	    ((p->image[i * 4 + 1] & 0xf8) >> 3);
	  *dd++ = pix & 0xff;
	  *dd++ = pix >> 8;
	}
	break;
      case _ARGB32:
	for (i = 0; i < p->width * p->height; i++) {
	  pix =
	    ((p->image[i * 4 + 1] & 0xf8) << 8) |
	    ((p->image[i * 4 + 2] & 0xfc) << 3) |
	    ((p->image[i * 4 + 3] & 0xf8) >> 3);
	  *dd++ = pix & 0xff;
	  *dd++ = pix >> 8;
	}
	break;
      case _BGRA32:
#ifdef WITH_NASM
	bgra32to16(dest, p->image, p->width, p->height);
#else
	for (i = 0; i < p->width * p->height; i++) {
	  pix =
	    ((p->image[i * 4 + 2] & 0xf8) << 8) |
	    ((p->image[i * 4 + 1] & 0xfc) << 3) |
	    ((p->image[i * 4 + 0] & 0xf8) >> 3);
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
	      pal = p->colormap[p->image[i + j]];
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
	dest = p->image;
	break;
      } else if (p->type == _BGR24) {
	ximage->byte_order = LSBFirst;
	dest = p->image;
	break;
      }

      if ((dest = calloc(3, p->width * p->height)) == NULL) {
	show_message("x11ximage_convert_image: No enough memory\n");
	exit(-2);
      }

      dd = dest;
      ximage->byte_order = LSBFirst;
      switch (p->type) {
      case _INDEX:
	for (i = 0; i < p->bytes_per_line * p->height; i += p->bytes_per_line) {
	  for (j = 0; j < p->width; j++) {
	    *dd++ = p->colormap[p->image[i + j]][2];
	    *dd++ = p->colormap[p->image[i + j]][1];
	    *dd++ = p->colormap[p->image[i + j]][0];
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
	dest = p->image;
	break;
      } else if (p->type == _BGR24) {
	ximage->byte_order = LSBFirst;
	bits_per_pixel = 24;
	dest = p->image;
	break;
      } else if (p->type == _ARGB32) {
	ximage->byte_order = MSBFirst;
	dest = p->image;
	break;
      } else if (p->type == _BGRA32) {
	ximage->byte_order = LSBFirst;
	dest = p->image;
	break;
      }

      if ((dest = calloc(4, p->width * p->height)) == NULL) {
	show_message("x11ximage_convert_image: No enough memory\n");
	exit(-2);
      }

      bytes_per_line = p->width * 4;
      switch (p->type) {
      case _RGBA32:
	ximage->byte_order = MSBFirst;
	memcpy(dest + 1, p->image, p->image_size - 1);
	break;
      case _ABGR32:
	ximage->byte_order = LSBFirst;
	memcpy(dest, p->image + 1, p->image_size - 1);
	break;
      case _INDEX:
	dd = dest;
	ximage->byte_order = LSBFirst;
	for (i = 0; i < p->width * p->height; i++) {
	  *dd++ = p->colormap[p->image[i]][2];
	  *dd++ = p->colormap[p->image[i]][1];
	  *dd++ = p->colormap[p->image[i]][0];
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

  ximage->data = dest;
  ximage->bytes_per_line = bytes_per_line;
  ximage->bits_per_pixel = bits_per_pixel;

  return;
}
