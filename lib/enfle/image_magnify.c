/*
 * image_magnify.c -- image magnification
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Sep 18 21:22:14 2000.
 * $Id: image_magnify.c,v 1.1 2000/09/30 17:36:36 sian Exp $
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

#include "common.h"

#include "image.h"

/* for internal use */

static void
magnify_generic8(unsigned char *d, unsigned char *s, int w, int h, int dw, int dh)
{
  int x, y, t, t2;

  if (dw >= w)
    if (dh >= h) {
      unsigned char *dd = d;
      for (y = 0; y < dh; y++) {
	t = y * h / dh * w;
	for (x = 0; x < dw; x++)
	  *dd++ = s[x * w / dw + t];
      }
    } else
      for (y = 0; y < h; y++) {
	t = y * dh / h * dw;
	t2 = y * w;
	for (x = 0; x < dw; x++)
	  d[x + t] = s[x * w / dw + t2];
      }
  else
    if (dh >= h)
      for (y = 0; y < dh; y++) {
	t = y * dw;
	t2 = y * h / dh * w;
	for (x = 0; x < w; x++)
	  d[x * dw / w + t] = s[x + t2];
      }
    else
      for (y = 0; y < h; y++) {
	t = y * dh / h * dw;
	for (x = 0; x < w; x++)
	  d[x * dw / w + t] = *s++;
      }
}

static void
magnify_generic16(unsigned short *d, unsigned short *s, int w, int h, int dw, int dh)
{
  int x, y, t, t2;

  if (dw >= w)
    if (dh >= h) {
      unsigned short *dd = d;
      for (y = 0; y < dh; y++) {
	t = y * h / dh * w;
	for (x = 0; x < dw; x++)
	  *dd++ = s[x * w / dw + t];
      }
    } else
      for (y = 0; y < h; y++) {
	t = y * dh / h * dw;
	t2 = y * w;
	for (x = 0; x < dw; x++)
	  d[x + t] = s[x * w / dw + t2];
      }
  else
    if (dh >= h)
      for (y = 0; y < dh; y++) {
	t = y * dw;
	t2 = y * h / dh * w;
	for (x = 0; x < w; x++)
	  d[x * dw / w + t] = s[x + t2];
      }
    else
      for (y = 0; y < h; y++) {
	t = y * dh / h * dw;
	for (x = 0; x < w; x++)
	  d[x * dw / w + t] = *s++;
      }
}

static void
magnify_generic24(unsigned char *d, unsigned char *s, int w, int h,
		  int dw, int dh, ImageInterpolateMethod method)
{
  int i, x, y, t, t2, t3, t4;

  if (dw >= w)
    if (dh >= h) {
      if (method == _BILINEAR) {
	unsigned char *dd = d;
	double dx, dy;
	int yt;

	yt = (h - 1) * w;
	for (y = 0; y < dh; y++) {
	  dy = (double)y * (double)h / (double)dh;
	  t = (int)dy;
	  dy -= t;
	  t = y * h / dh * w;
	  for (x = 0; x < dw; x++) {
	    dx = (double)x * (double)w / (double)dw;
	    t3 = (int)dx;
	    dx -= t3;

	    if (t < yt) {
	      /* bilinear interpolation */
	      for (i = 0; i < 3; i++) 
		*dd++ =
		  s[(t     +  t3     ) * 3 + i] * (1.0 - dx) * (1.0 - dy) +
		  s[(t     + (t3 + 1)) * 3 + i] *        dx  * (1.0 - dy) +
		  s[(t + w +  t3     ) * 3 + i] * (1.0 - dx) *        dy  +
		  s[(t + w + (t3 + 1)) * 3 + i] *        dx  *        dy;
	    } else {
	      for (i = 0; i < 3; i++) 
		*dd++ =
		  s[(t     +  t3     ) * 3 + i] * (1.0 - dx) +
		  s[(t     + (t3 + 1)) * 3 + i] *        dx;
	    }
	  }
	}
      } else {
	unsigned char *dd = d;
	for (y = 0; y < dh; y++) {
	  t = y * h / dh * w;
	  for (x = 0; x < dw; x++) {
	    t3 = x * w / dw + t;
	    for (i = 0; i < 3; i++)
	      *dd++ = s[t3 * 3 + i];
	  }
	}
      }
    } else
      for (y = 0; y < h; y++) {
	t = y * dh / h * dw;
	t2 = y * w;
	for (x = 0; x < dw; x++) {
	  t4 = x * w / dw + t2;
	  for (i = 0; i < 3; i++)
	    d[(x + t) * 3 + i] = s[t4 * 3 + i];
	}
      }
  else
    if (dh >= h)
      for (y = 0; y < dh; y++) {
	t = y * dw;
	t2 = y * h / dh * w;
	for (x = 0; x < w; x++) {
	  t3 = x * dw / w + t;
	  for (i = 0; i < 3; i++)
	    d[t3 * 3 + i] = s[(x + t2) * 3 + i];
	}
      }
    else
      for (y = 0; y < h; y++) {
	t = y * dh / h * dw;
	for (x = 0; x < w; x++) {
	  t3 = x * dw / w + t;
	  for (i = 0; i < 3; i++)
	    d[t3 * 3 + i] = *s++;
	}
      }
}

static void
magnify_generic32(unsigned char *d, unsigned char *s, int w, int h,
		  int dw, int dh, ImageInterpolateMethod method)
{
  int i, x, y, t, t2, t3, t4;

  if (dw >= w)
    if (dh >= h) {
      if (method == _BILINEAR) {
	unsigned char *dd = d;
	double dx, dy;
	int yt;

	yt = (h - 1) * w;
	for (y = 0; y < dh; y++) {
	  dy = (double)y * (double)h / (double)dh;
	  t = (int)dy;
	  dy -= t;
	  t = y * h / dh * w;
	  for (x = 0; x < dw; x++) {
	    dx = (double)x * (double)w / (double)dw;
	    t3 = (int)dx;
	    dx -= t3;

	    if (t < yt) {
	      /* bilinear interpolation */
	      for (i = 0; i < 3; i++) 
		*dd++ =
		  s[(t +      t3     ) * 4 + i] * (1.0 - dx) * (1.0 - dy) +
		  s[(t +     (t3 + 1)) * 4 + i] *        dx  * (1.0 - dy) +
		  s[(t + w +  t3     ) * 4 + i] * (1.0 - dx) *        dy  +
		  s[(t + w + (t3 + 1)) * 4 + i] *        dx  *        dy;
	    } else {
	      for (i = 0; i < 3; i++) 
		*dd++ =
		  s[(t     +  t3     ) * 4 + i] * (1.0 - dx) +
		  s[(t     + (t3 + 1)) * 4 + i] *        dx;
	    }
	  }
	}
      } else {
	unsigned char *dd = d;
	for (y = 0; y < dh; y++) {
	  t = y * h / dh * w;
	  for (x = 0; x < dw; x++) {
	    t3 = x * w / dw + t;
	    for (i = 0; i < 3; i++)
	      *dd++ = s[t3 * 4 + i];
	  }
	}
      }
    } else
      for (y = 0; y < h; y++) {
	t = y * dh / h * dw;
	t2 = y * w;
	for (x = 0; x < dw; x++) {
	  t4 = x * w / dw + t2;
	  for (i = 0; i < 3; i++)
	    d[(x + t) * 4 + i] = s[t4 * 4 + i];
	}
      }
  else
    if (dh >= h)
      for (y = 0; y < dh; y++) {
	t = y * dw;
	t2 = y * h / dh * w;
	for (x = 0; x < w; x++) {
	  t3 = x * dw / w + t;
	  for (i = 0; i < 3; i++)
	    d[t3 * 4 + i] = s[(x + t2) * 4 + i];
	}
      }
    else
      for (y = 0; y < h; y++) {
	t = y * dh / h * dw;
	for (x = 0; x < w; x++) {
	  t3 = x * dw / w + t;
	  for (i = 0; i < 3; i++)
	    d[t3 * 4 + i] = *s++;
	}
      }
}

/* public interface */

int
image_magnify_main(Image *p, int dw, int dh, ImageInterpolateMethod method)
{
  unsigned int image_size;
  unsigned char *image;

  switch (p->type) {
  case _GRAY:
  case _INDEX:
    image_size = dw * dh;
    break;
  case _RGB_WITH_BITMASK:
  case _BGR_WITH_BITMASK:
    image_size = dw * dh * 2;
    break;
  case _RGB24:
  case _BGR24:
    image_size = dw * dh * 3;
    break;
  case _RGBA32:
  case _ABGR32:
  case _ARGB32:
  case _BGRA32:
    image_size = dw * dh * 4;
    break;
  default:
    fprintf(stderr, "image_magnify: unsupported image type %s\n", image_type_to_string(p->type));
    return 0;
  }

  if ((image = malloc(image_size)) == NULL)
    return 0;

  switch (p->type) {
  case _GRAY:
  case _INDEX:
    magnify_generic8(image, p->image, p->width, p->height, dw, dh);
    break;
  case _RGB_WITH_BITMASK:
  case _BGR_WITH_BITMASK:
    magnify_generic16((unsigned short *)image, (unsigned short *)p->image, p->width, p->height, dw, dh);
    break;
  case _RGB24:
  case _BGR24:
    magnify_generic24(image, p->image, p->width, p->height, dw, dh, method);
    break;
  case _RGBA32:
  case _ABGR32:
  case _ARGB32:
  case _BGRA32:
    magnify_generic32(image, p->image, p->width, p->height, dw, dh, method);
    break;
  default:
    fprintf(stderr, "image_magnify: unsupported image type %s\n", image_type_to_string(p->type));
    return 0;
  }

  p->image = image;
  p->image_size = image_size;
  p->width = dw;
  p->height = dh;

  return 1;
}
