/*
 * x11ximage.c -- X11 XImage related functions
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file if part of Enfle.
 *
 * Last Modified: Sat Oct 14 05:37:19 2000.
 * $Id: x11ximage.c,v 1.2 2000/10/15 07:50:42 sian Exp $
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
      unsigned char *dest, *dd;
      unsigned short int pix;

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
      free(p->image);
      p->image = dest;
      p->type = _BGR_WITH_BITMASK;
      p->depth = 16;
      p->bits_per_pixel = 16;
      p->bytes_per_line = p->width * 2;
      p->image_size = p->bytes_per_line * p->height;
#if 0
      /* XXX: TODO: use this info. to convert pixel written in above... */
      p->red_mask   = 0xf800;
      p->green_mask = 0x07e0;
      p->blue_mask  = 0x001f;
#endif
    }
    break;
  case 24:
    {
      unsigned char *dest;
      unsigned char *dd;
      int i;

      if (p->type == _RGB24) {
	ximage->byte_order = MSBFirst;
	break;
      } else if (p->type == _BGR24) {
	ximage->byte_order = LSBFirst;
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
      default:
	show_message("Cannot render image [type %s] so far.\n", image_type_to_string(p->type));
	break;
      }
      free(p->image);
      p->image = dest;
      p->type = _BGR24;
      p->depth = 24;
      p->bits_per_pixel = 24;
      p->bytes_per_line = p->width * 3;
      p->image_size = p->bytes_per_line * p->height;
#if 0
      p->red_mask   = 0xff0000;
      p->green_mask = 0x00ff00;
      p->blue_mask  = 0x0000ff;
#endif
    }
    break;
  case 32:
    {
      unsigned char *dest;
      unsigned char *dd;
      int i;

      if (p->type == _RGBA32 || p->type == _ARGB32 || p->type == _RGB24) {
	ximage->byte_order = MSBFirst;
	if (p->type == _RGBA32)
	  memcpy(p->image + 1, p->image, p->image_size - 1);
	break;
      } else if (p->type == _ABGR32 || p->type == _BGRA32 || p->type == _BGR24) {
	ximage->byte_order = LSBFirst;
	if (p->type == _ABGR32)
	  memcpy(p->image, p->image + 1, p->image_size - 1);
	break;
      }

      if ((dest = calloc(4, p->width * p->height)) == NULL) {
	show_message("x11ximage_convert_image: No enough memory\n");
	exit(-2);
      }

      dd = dest;
      ximage->byte_order = LSBFirst;
      switch (p->type) {
      case _INDEX:
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
      free(p->image);
      p->image = dest;
      p->type = _ABGR32;
      p->depth = 24;
      p->bits_per_pixel = 32;
      p->bytes_per_line = p->width * 4;
      p->image_size = p->bytes_per_line * p->height;
#if 0
      p->red_mask   = 0x00ff0000;
      p->green_mask = 0x0000ff00;
      p->blue_mask  = 0x000000ff;
#endif
    }
    break;
  default:
    show_message("ximage->bits_per_pixel = %d\n", ximage->bits_per_pixel);
    break;
  }

  ximage->data = p->image;
  ximage->bytes_per_line = p->bytes_per_line;
  ximage->bits_per_pixel = p->bits_per_pixel;

  return;
}
