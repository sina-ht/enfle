/*
 * image_magnify.c -- image magnification
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Jun  8 00:04:54 2002.
 * $Id: image_magnify.c,v 1.8 2002/06/13 14:29:44 sian Exp $
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
#define REQUIRE_STRING_H
#include "compat.h"

#include "image.h"

#define INTEGER_ARITHMETIC
#define PRECISION 8
#define PRECISION2 (PRECISION * 2)
#define MIN_INT (1 << PRECISION)
#define DECIMAL_MASK (MIN_INT - 1)

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
#ifndef INTEGER_ARITHMETIC
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
#else
	unsigned char *dd = d;
	unsigned int dx, dy;
	int yt;

	yt = (h - 1) * w;
	for (y = 0; y < dh; y++) {
	  dy = ((y << PRECISION) * h / dh) & DECIMAL_MASK;
	  t = y * h / dh * w;

	  if (t < yt) {
	    for (x = 0; x < dw; x++) {
	      dx = ((x << PRECISION) * w / dw) & DECIMAL_MASK;
	      t3 = x * w / dw;

	      /* bilinear interpolation */
	      for (i = 0; i < 3; i++) 
		*dd++ = (
		  s[(t +      t3     ) * 3 + i] * (MIN_INT - dx) * (MIN_INT - dy) +
		  s[(t +     (t3 + 1)) * 3 + i] *            dx  * (MIN_INT - dy) +
		  s[(t + w +  t3     ) * 3 + i] * (MIN_INT - dx) *            dy  +
		  s[(t + w + (t3 + 1)) * 3 + i] *            dx  *            dy ) >> PRECISION2;
	    }
	  } else {
	    for (x = 0; x < dw; x++) {
	      dx = ((x << PRECISION) * w / dw) & DECIMAL_MASK;
	      t3 = x * w / dw;

	      for (i = 0; i < 3; i++) 
		*dd++ = (
		  s[(t     +  t3     ) * 3 + i] * (MIN_INT - dx) +
		  s[(t     + (t3 + 1)) * 3 + i] *            dx ) >> PRECISION;
	    }
	  }
	}
#endif
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
#ifndef INTEGER_ARITHMETIC
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
		  s[((t +      t3     ) << 2) + i] * (1.0 - dx) * (1.0 - dy) +
		  s[((t +     (t3 + 1)) << 2) + i] *        dx  * (1.0 - dy) +
		  s[((t + w +  t3     ) << 2) + i] * (1.0 - dx) *        dy  +
		  s[((t + w + (t3 + 1)) << 2) + i] *        dx  *        dy;
	      dd++;
	    } else {
	      for (i = 0; i < 3; i++) 
		*dd++ =
		  s[((t     +  t3     ) << 2) + i] * (1.0 - dx) +
		  s[((t     + (t3 + 1)) << 2) + i] *        dx;
	      dd++;
	    }
	  }
	}
#else
	unsigned char *dd = d;
	unsigned int dx, dy;
	int yt;

	yt = (h - 1) * w;
	for (y = 0; y < dh; y++) {
	  dy = ((y << PRECISION) * h / dh) & DECIMAL_MASK;
	  t = y * h / dh * w;


	  if (t < yt) {
	    for (x = 0; x < dw; x++) {
	      dx = ((x << PRECISION) * w / dw) & DECIMAL_MASK;
	      t3 = x * w / dw;

	      /* bilinear interpolation */
	      for (i = 0; i < 3; i++) 
		*dd++ = (
		  s[((t +      t3     ) << 2) + i] * (MIN_INT - dx) * (MIN_INT - dy) +
		  s[((t +     (t3 + 1)) << 2) + i] *            dx  * (MIN_INT - dy) +
		  s[((t + w +  t3     ) << 2) + i] * (MIN_INT - dx) *            dy  +
		  s[((t + w + (t3 + 1)) << 2) + i] *            dx  *            dy ) >> PRECISION2;
	      dd++;
	    }
	  } else {
	    for (x = 0; x < dw; x++) {
	      dx = ((x << PRECISION) * w / dw) & DECIMAL_MASK;
	      t3 = x * w / dw;

	      for (i = 0; i < 3; i++) 
		*dd++ = (
		  s[((t     +  t3     ) << 2) + i] * (MIN_INT - dx) +
		  s[((t     + (t3 + 1)) << 2) + i] *            dx ) >> PRECISION;
	      dd++;
	    }
	  }
	}
#endif
      } else {
	unsigned char *dd = d;
	for (y = 0; y < dh; y++) {
	  t = y * h / dh * w;
	  for (x = 0; x < dw; x++) {
	    t3 = x * w / dw + t;
	    for (i = 0; i < 3; i++)
	      *dd++ = s[(t3 << 2) + i];
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
	    d[((x + t) << 2) + i] = s[(t4 << 2) + i];
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
	    d[(t3 << 2) + i] = s[((x + t2) << 2) + i];
	}
      }
    else
      for (y = 0; y < h; y++) {
	t = y * dh / h * dw;
	for (x = 0; x < w; x++) {
	  t3 = x * dw / w + t;
	  for (i = 0; i < 3; i++)
	    d[(t3 << 2) + i] = *s++;
	}
      }
}

static void
canonicalize(Image *p, unsigned int shrinked_bpl)
{
  unsigned char *s = memory_ptr(p->image);
  unsigned int i;

  if (p->bytes_per_line <= shrinked_bpl)
    return;

  for (i = 1; i < p->height; i++)
    memmove(s + i * shrinked_bpl, s + i * p->bytes_per_line, shrinked_bpl);
  p->bytes_per_line = shrinked_bpl;
  memory_alloc(p->image, shrinked_bpl * p->height);
}

/* public interface */

int
image_magnify_main(Image *p, int dw, int dh, ImageInterpolateMethod method, int use_hw_scale)
{
  if (!use_hw_scale) {
    switch (p->type) {
    case _GRAY:
    case _INDEX:
      p->magnified.bytes_per_line = dw;
      canonicalize(p, p->width);
      break;
    case _RGB_WITH_BITMASK:
    case _BGR_WITH_BITMASK:
      p->magnified.bytes_per_line = dw << 1;
      canonicalize(p, p->width << 1);
      break;
    case _RGB24:
    case _BGR24:
      p->magnified.bytes_per_line = dw * 3;
      canonicalize(p, p->width * 3);
      break;
    case _RGBA32:
    case _ABGR32:
    case _ARGB32:
    case _BGRA32:
      p->magnified.bytes_per_line = dw << 2;
      canonicalize(p, p->width << 2);
      break;
    default:
      show_message_fnc("unsupported image type %s\n", image_type_to_string(p->type));
      return 0;
    }

    if (!p->magnified.image && ((p->magnified.image = memory_create()) == NULL)) {
      show_message_fnc("No enough memory\n");
      return 0;
    }

    if ((memory_alloc(p->magnified.image, p->magnified.bytes_per_line * dh)) == NULL) {
      show_message_fnc("No enough memory (%d bytes requested)\n", p->magnified.bytes_per_line * dh);
      return 0;
    }

    switch (p->type) {
    case _GRAY:
    case _INDEX:
      magnify_generic8(memory_ptr(p->magnified.image), memory_ptr(p->image), p->width, p->height, dw, dh);
      break;
    case _RGB_WITH_BITMASK:
    case _BGR_WITH_BITMASK:
      magnify_generic16((unsigned short *)memory_ptr(p->magnified.image), (unsigned short *)memory_ptr(p->image), p->width, p->height, dw, dh);
      break;
    case _RGB24:
    case _BGR24:
      magnify_generic24(memory_ptr(p->magnified.image), memory_ptr(p->image), p->width, p->height, dw, dh, method);
      break;
    case _RGBA32:
    case _ABGR32:
    case _ARGB32:
    case _BGRA32:
      magnify_generic32(memory_ptr(p->magnified.image), memory_ptr(p->image), p->width, p->height, dw, dh, method);
      break;
    default:
      show_message_fnc("unsupported image type %s\n", image_type_to_string(p->type));
      return 0;
    }
  }

  p->magnified.width  = dw;
  p->magnified.height = dh;
  p->if_magnified = 1;

  return 1;
}
