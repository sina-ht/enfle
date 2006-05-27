/*
 * image_magnify.c -- image magnification
 * (C)Copyright 2000-2006 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat May 27 17:38:30 2006.
 * $Id: image_magnify.c,v 1.13 2006/05/27 08:41:27 sian Exp $
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
  int i, x, y, t, t3;

  if (method == _NOINTERPOLATE ||
      (method == _BILINEAR && ((dw >= w && dh < h) || (dw < w && dh >= h)))) {
    for (y = 0; y < dh; y++) {
      t = y * h / dh * w;
      for (x = 0; x < dw; x++) {
	t3 = x * w / dw + t;
	for (i = 0; i < 3; i++)
	  *d++ = s[t3 * 3 + i];
      }
    }
    return;
  }

  /* method == _BILINEAR */
  if (dw >= w /* && dh >= h */) {
#if !defined(INTEGER_ARITHMETIC)
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
	    *d++ =
	      s[(t     +  t3     ) * 3 + i] * (1.0 - dx) * (1.0 - dy) +
	      s[(t     + (t3 + 1)) * 3 + i] *        dx  * (1.0 - dy) +
	      s[(t + w +  t3     ) * 3 + i] * (1.0 - dx) *        dy  +
	      s[(t + w + (t3 + 1)) * 3 + i] *        dx  *        dy;
	} else {
	  for (i = 0; i < 3; i++) 
	    *d++ =
	      s[(t     +  t3     ) * 3 + i] * (1.0 - dx) +
	      s[(t     + (t3 + 1)) * 3 + i] *        dx;
	}
      }
    }
#else
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
	    *d++ = (
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
	    *d++ = (
		     s[(t     +  t3     ) * 3 + i] * (MIN_INT - dx) +
		     s[(t     + (t3 + 1)) * 3 + i] *            dx ) >> PRECISION;
	}
      }
    }
#endif
  } else {
    /* dw < w && dh < h, bilinear */
#if !defined(INTEGER_ARITHMETIC)
    double sx, sy, sx2, sy2;
    double dxl, dyu, dxr, dyd;
    double num[3], den;
    int xl, xr, yu, yd, j, k;

    sy = 0;
    for (y = 0; y < dh; y++) {
      sy2 = (double)(y + 1) * (double)h / (double)dh;
      t = (int)sy;
      dyu = (sy > t) ? t + 1 - sy : 0;
      yu = (dyu > 0) ? sy + 1 : sy;
      yd = (int)sy2;
      dyd = sy2 - yd;

      sx = 0;
      for (x = 0; x < dw; x++) {
	sx2 = (double)(x + 1) * (double)w / (double)dw;
	t3 = (int)sx;
	dxl = (sx > t3) ? t3 + 1 - sx : 0;
	xl = (dxl > 0) ? sx + 1 : sx;
	xr = (int)sx2;
	dxr = sx2 - xr;

	num[0] = num[1] = num[2] = den = 0;
	/* upper margin */
	if (dyu > 0) {
	  if (dxl > 0) {
	    for (i = 0; i < 3; i++)
	      num[i] += s[(((yu - 1) * w + xl - 1) * 3) + i] * dxl * dyu;
	    den += dxl * dyu;
	  }
	  for (j = xl; j < xr; j++) {
	    for (i = 0; i < 3; i++)
	      num[i] += s[(((yu - 1) * w + j) * 3) + i] * dyu;
	    den += dyu;
	  }
	  if (dxr > 0) {
	    for (i = 0; i < 3; i++)
	      num[i] += s[(((yu - 1) * w + xr) * 3) + i] * dxr * dyu;
	    den += dxr * dyu;
	  }
	}

	for (k = yu; k < yd; k++) {
	  if (dxl > 0) {
	    for (i = 0; i < 3; i++)
	      num[i] += s[((k * w + xl - 1) * 3) + i] * dxl;
	    den += dxl;
	  }
	  for (j = xl; j < xr; j++) {
	    for (i = 0; i < 3; i++)
	      num[i] += s[((k * w + j) * 3) + i];
	    den++;
	  }
	  if (dxr > 0) {
	    for (i = 0; i < 3; i++)
	      num[i] += s[((k * w + xr) * 3) + i] * dxr;
	    den += dxr;
	  }
	}

	/* lower margin */
	if (dyd > 0) {
	  if (dxl > 0) {
	    for (i = 0; i < 3; i++)
	      num[i] += s[((yd * w + xl - 1) * 3) + i] * dxl * dyd;
	    den += dxl * dyd;
	  }
	  for (j = xl; j < xr; j++) {
	    for (i = 0; i < 3; i++)
	      num[i] += s[((yd * w + j) * 3) + i] * dyd;
	    den += dyd;
	  }
	  if (dxr > 0) {
	    for (i = 0; i < 3; i++)
	      num[i] += s[((yd * w + xr) * 3) + i] * dxr * dyd;
	    den += dxr * dyd;
	  }
	}
	for (i = 0; i < 3; i++) 
	  *d++ = num[i] / den;
	sx = sx2;
      }
      sy = sy2;
    }
#else
    unsigned int sx, sy, sx2, sy2;
    unsigned int dxl, dyu, dxr, dyd;
    unsigned int num[3], den;
    int xl, xr, yu, yd, j, k;

    sy = 0;
    for (y = 0; y < dh; y++) {
      sy2 = ((y + 1) << PRECISION) * h / dh;
      t = sy & DECIMAL_MASK;
      dyu = (t > 0) ? MIN_INT - t : 0;
      yu = ((dyu > 0) ? sy + MIN_INT : sy) >> PRECISION;
      yd = sy2 >> PRECISION;
      dyd = sy2 & DECIMAL_MASK;

      sx = 0;
      for (x = 0; x < dw; x++) {
	sx2 = ((x + 1) << PRECISION) * w / dw;
	t3 = sx & DECIMAL_MASK;
	dxl = (t3 > 0) ? MIN_INT - t3 : 0;
	xl = ((dxl > 0) ? sx + MIN_INT : sx) >> PRECISION;
	xr = sx2 >> PRECISION;
	dxr = sx2 & DECIMAL_MASK;

	num[0] = num[1] = num[2] = den = 0;
	/* upper margin */
	if (dyu > 0) {
	  if (dxl > 0) {
	    for (i = 0; i < 3; i++)
	      num[i] += s[(((yu - 1) * w + xl - 1) * 3) + i] * dxl * dyu;
	    den += dxl * dyu;
	  }
	  for (j = xl; j < xr; j++) {
	    for (i = 0; i < 3; i++)
	      num[i] += s[(((yu - 1) * w + j) * 3) + i] * (dyu << 8);
	    den += (dyu << 8);
	  }
	  if (dxr > 0) {
	    for (i = 0; i < 3; i++)
	      num[i] += s[(((yu - 1) * w + xr) * 3) + i] * dxr * dyu;
	    den += dxr * dyu;
	  }
	}

	for (k = yu; k < yd; k++) {
	  if (dxl > 0) {
	    for (i = 0; i < 3; i++)
	      num[i] += s[((k * w + xl - 1) * 3) + i] * (dxl << 8);
	    den += (dxl << 8);
	  }
	  for (j = xl; j < xr; j++) {
	    for (i = 0; i < 3; i++)
	      num[i] += s[((k * w + j) * 3) + i] << 16;
	    den += 256 * 256;
	  }
	  if (dxr > 0) {
	    for (i = 0; i < 3; i++)
	      num[i] += s[((k * w + xr) * 3) + i] * (dxr << 8);
	    den += (dxr << 8);
	  }
	}

	/* lower margin */
	if (dyd > 0) {
	  if (dxl > 0) {
	    for (i = 0; i < 3; i++)
	      num[i] += s[((yd * w + xl - 1) * 3) + i] * dxl * dyd;
	    den += dxl * dyd;
	  }
	  for (j = xl; j < xr; j++) {
	    for (i = 0; i < 3; i++)
	      num[i] += s[((yd * w + j) * 3) + i] * (dyd << 8);
	    den += (dyd << 8);
	  }
	  if (dxr > 0) {
	    for (i = 0; i < 3; i++)
	      num[i] += s[((yd * w + xr) * 3) + i] * dxr * dyd;
	    den += dxr * dyd;
	  }
	}
	for (i = 0; i < 3; i++) 
	  *d++ = num[i] / den;
	sx = sx2;
      }
      sy = sy2;
    }
#endif
  }
}

static void
magnify_generic32(unsigned char *d, unsigned char *s, int w, int h,
		  int dw, int dh, ImageInterpolateMethod method)
{
  int i, x, y, t, t3;

  if (method == _NOINTERPOLATE ||
      (method == _BILINEAR && ((dw >= w && dh < h) || (dw < w && dh >= h)))) {
    for (y = 0; y < dh; y++) {
      t = y * h / dh * w;
      for (x = 0; x < dw; x++) {
	t3 = x * w / dw + t;
	for (i = 0; i < 3; i++)
	  *d++ = s[(t3 << 2) + i];
	d++;
      }
    }
    return;
  }

  /* method == _BILINEAR */
  if (dw >= w /* && dh >= h */) {
    /* dw >= w && dh >= h, bilinear */
#if !defined(INTEGER_ARITHMETIC)
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
	    *d++ =
	      s[((t +      t3     ) << 2) + i] * (1.0 - dx) * (1.0 - dy) +
	      s[((t +     (t3 + 1)) << 2) + i] *        dx  * (1.0 - dy) +
	      s[((t + w +  t3     ) << 2) + i] * (1.0 - dx) *        dy  +
	      s[((t + w + (t3 + 1)) << 2) + i] *        dx  *        dy;
	  d++;
	} else {
	  for (i = 0; i < 3; i++) 
	    *d++ =
	      s[((t     +  t3     ) << 2) + i] * (1.0 - dx) +
	      s[((t     + (t3 + 1)) << 2) + i] *        dx;
	  d++;
	}
      }
    }
#else
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
	    *d++ = (
		     s[((t +      t3     ) << 2) + i] * (MIN_INT - dx) * (MIN_INT - dy) +
		     s[((t +     (t3 + 1)) << 2) + i] *            dx  * (MIN_INT - dy) +
		     s[((t + w +  t3     ) << 2) + i] * (MIN_INT - dx) *            dy  +
		     s[((t + w + (t3 + 1)) << 2) + i] *            dx  *            dy ) >> PRECISION2;
	  d++;
	}
      } else {
	for (x = 0; x < dw; x++) {
	  dx = ((x << PRECISION) * w / dw) & DECIMAL_MASK;
	  t3 = x * w / dw;

	  for (i = 0; i < 3; i++) 
	    *d++ = (
		     s[((t     +  t3     ) << 2) + i] * (MIN_INT - dx) +
		     s[((t     + (t3 + 1)) << 2) + i] *            dx ) >> PRECISION;
	  d++;
	}
      }
    }
#endif
  } else {
    /* dw < w && dh < h, bilinear */
#if !defined(INTEGER_ARITHMETIC)
    double sx, sy, sx2, sy2;
    double dxl, dyu, dxr, dyd;
    double num[3], den;
    int xl, xr, yu, yd, j, k;

    sy = 0;
    for (y = 0; y < dh; y++) {
      sy2 = (double)(y + 1) * (double)h / (double)dh;
      t = (int)sy;
      dyu = (sy > t) ? t + 1 - sy : 0;
      yu = (dyu > 0) ? sy + 1 : sy;
      yd = (int)sy2;
      dyd = sy2 - yd;

      sx = 0;
      for (x = 0; x < dw; x++) {
	sx2 = (double)(x + 1) * (double)w / (double)dw;
	t3 = (int)sx;
	dxl = (sx > t3) ? t3 + 1 - sx : 0;
	xl = (dxl > 0) ? sx + 1 : sx;
	xr = (int)sx2;
	dxr = sx2 - xr;

	num[0] = num[1] = num[2] = den = 0;
	/* upper margin */
	if (dyu > 0) {
	  if (dxl > 0) {
	    for (i = 0; i < 3; i++)
	      num[i] += s[(((yu - 1) * w + xl - 1) << 2) + i] * dxl * dyu;
	    den += dxl * dyu;
	  }
	  for (j = xl; j < xr; j++) {
	    for (i = 0; i < 3; i++)
	      num[i] += s[(((yu - 1) * w + j) << 2) + i] * dyu;
	    den += dyu;
	  }
	  if (dxr > 0) {
	    for (i = 0; i < 3; i++)
	      num[i] += s[(((yu - 1) * w + xr) << 2) + i] * dxr * dyu;
	    den += dxr * dyu;
	  }
	}

	for (k = yu; k < yd; k++) {
	  if (dxl > 0) {
	    for (i = 0; i < 3; i++)
	      num[i] += s[((k * w + xl - 1) << 2) + i] * dxl;
	    den += dxl;
	  }
	  for (j = xl; j < xr; j++) {
	    for (i = 0; i < 3; i++)
	      num[i] += s[((k * w + j) << 2) + i];
	    den++;
	  }
	  if (dxr > 0) {
	    for (i = 0; i < 3; i++)
	      num[i] += s[((k * w + xr) << 2) + i] * dxr;
	    den += dxr;
	  }
	}

	/* lower margin */
	if (dyd > 0) {
	  if (dxl > 0) {
	    for (i = 0; i < 3; i++)
	      num[i] += s[((yd * w + xl - 1) << 2) + i] * dxl * dyd;
	    den += dxl * dyd;
	  }
	  for (j = xl; j < xr; j++) {
	    for (i = 0; i < 3; i++)
	      num[i] += s[((yd * w + j) << 2) + i] * dyd;
	    den += dyd;
	  }
	  if (dxr > 0) {
	    for (i = 0; i < 3; i++)
	      num[i] += s[((yd * w + xr) << 2) + i] * dxr * dyd;
	    den += dxr * dyd;
	  }
	}
	for (i = 0; i < 3; i++) 
	  *d++ = num[i] / den;
	d++;
	sx = sx2;
      }
      sy = sy2;
    }
#else
    unsigned int sx, sy, sx2, sy2;
    unsigned int dxl, dyu, dxr, dyd;
    unsigned int num[3], den;
    int xl, xr, yu, yd, j, k;

    sy = 0;
    for (y = 0; y < dh; y++) {
      sy2 = ((y + 1) << PRECISION) * h / dh;
      t = sy & DECIMAL_MASK;
      dyu = (t > 0) ? MIN_INT - t : 0;
      yu = ((dyu > 0) ? sy + MIN_INT : sy) >> PRECISION;
      yd = sy2 >> PRECISION;
      dyd = sy2 & DECIMAL_MASK;

      sx = 0;
      for (x = 0; x < dw; x++) {
	sx2 = ((x + 1) << PRECISION) * w / dw;
	t3 = sx & DECIMAL_MASK;
	dxl = (t3 > 0) ? MIN_INT - t3 : 0;
	xl = ((dxl > 0) ? sx + MIN_INT : sx) >> PRECISION;
	xr = sx2 >> PRECISION;
	dxr = sx2 & DECIMAL_MASK;

	num[0] = num[1] = num[2] = den = 0;
	/* upper margin */
	if (dyu > 0) {
	  if (dxl > 0) {
	    for (i = 0; i < 3; i++)
	      num[i] += s[(((yu - 1) * w + xl - 1) << 2) + i] * dxl * dyu;
	    den += dxl * dyu;
	  }
	  for (j = xl; j < xr; j++) {
	    for (i = 0; i < 3; i++)
	      num[i] += s[(((yu - 1) * w + j) << 2) + i] * (dyu << 8);
	    den += (dyu << 8);
	  }
	  if (dxr > 0) {
	    for (i = 0; i < 3; i++)
	      num[i] += s[(((yu - 1) * w + xr) << 2) + i] * dxr * dyu;
	    den += dxr * dyu;
	  }
	}

	for (k = yu; k < yd; k++) {
	  if (dxl > 0) {
	    for (i = 0; i < 3; i++)
	      num[i] += s[((k * w + xl - 1) << 2) + i] * (dxl << 8);
	    den += (dxl << 8);
	  }
	  for (j = xl; j < xr; j++) {
	    for (i = 0; i < 3; i++)
	      num[i] += s[((k * w + j) << 2) + i] << 16;
	    den += 256 * 256;
	  }
	  if (dxr > 0) {
	    for (i = 0; i < 3; i++)
	      num[i] += s[((k * w + xr) << 2) + i] * (dxr << 8);
	    den += (dxr << 8);
	  }
	}

	/* lower margin */
	if (dyd > 0) {
	  if (dxl > 0) {
	    for (i = 0; i < 3; i++)
	      num[i] += s[((yd * w + xl - 1) << 2) + i] * dxl * dyd;
	    den += dxl * dyd;
	  }
	  for (j = xl; j < xr; j++) {
	    for (i = 0; i < 3; i++)
	      num[i] += s[((yd * w + j) << 2) + i] * (dyd << 8);
	    den += (dyd << 8);
	  }
	  if (dxr > 0) {
	    for (i = 0; i < 3; i++)
	      num[i] += s[((yd * w + xr) << 2) + i] * dxr * dyd;
	    den += dxr * dyd;
	  }
	}
	for (i = 0; i < 3; i++) 
	  *d++ = num[i] / den;
	d++;
	sx = sx2;
      }
      sy = sy2;
    }
#endif
  }
}

static void
canonicalize(Image *p, int src, unsigned int shrinked_bpl)
{
  unsigned char *s = memory_ptr(image_image_by_index(p, src));
  unsigned int i;

  if (image_bpl_by_index(p, src) <= shrinked_bpl)
    return;

  for (i = 1; i < image_height_by_index(p, src); i++)
    memmove(s + i * shrinked_bpl, s + i * image_bpl_by_index(p, src), shrinked_bpl);
  image_bpl_by_index(p, src) = shrinked_bpl;
  memory_alloc(image_image_by_index(p, src), shrinked_bpl * image_height_by_index(p, src));
}

/* public interface */

int
image_magnify(Image *p, int src, int dst, int dw, int dh, ImageInterpolateMethod method)
{
  switch (method) {
  case _NOINTERPOLATE:
  case _BILINEAR:
    break;
  default:
    warning("Unsupported magnify method\n");
    return 0;
  }

  switch (p->type) {
  case _GRAY:
  case _INDEX:
    image_bpl_by_index(p, dst) = dw;
    canonicalize(p, src, image_width_by_index(p, src));
    break;
  case _RGB555:
  case _BGR555:
  case _RGB565:
  case _BGR565:
    image_bpl_by_index(p, dst) = dw << 1;
    canonicalize(p, src, image_width_by_index(p, src) << 1);
    break;
  case _RGB24:
  case _BGR24:
    image_bpl_by_index(p, dst) = dw * 3;
    canonicalize(p, src, image_width_by_index(p, src) * 3);
    break;
  case _RGBA32:
  case _ABGR32:
  case _ARGB32:
  case _BGRA32:
    image_bpl_by_index(p, dst) = dw << 2;
    canonicalize(p, src, image_width_by_index(p, src) << 2);
    break;
  default:
    show_message_fnc("unsupported image type %s\n", image_type_to_string(p->type));
    return 0;
  }

  if (!image_image_by_index(p, dst) && ((image_image_by_index(p, dst) = memory_create()) == NULL)) {
    show_message_fnc("No enough memory\n");
    return 0;
  }

  if ((memory_alloc(image_image_by_index(p, dst), image_bpl_by_index(p, dst) * dh)) == NULL) {
    show_message_fnc("No enough memory (%d bytes requested)\n", image_bpl_by_index(p, dst) * dh);
    return 0;
  }

  switch (p->type) {
  case _GRAY:
  case _INDEX:
    magnify_generic8(memory_ptr(image_image_by_index(p, dst)), memory_ptr(image_image_by_index(p, src)), image_width_by_index(p, src), image_height_by_index(p, src), dw, dh);
    break;
  case _RGB555:
  case _BGR555:
  case _RGB565:
  case _BGR565:
    magnify_generic16((unsigned short *)memory_ptr(image_image_by_index(p, dst)), (unsigned short *)memory_ptr(image_image_by_index(p, src)), image_width_by_index(p, src), image_height_by_index(p, src), dw, dh);
    break;
  case _RGB24:
  case _BGR24:
    magnify_generic24(memory_ptr(image_image_by_index(p, dst)), memory_ptr(image_image_by_index(p, src)), image_width_by_index(p, src), image_height_by_index(p, src), dw, dh, method);
    break;
  case _RGBA32:
  case _ABGR32:
  case _ARGB32:
  case _BGRA32:
    magnify_generic32(memory_ptr(image_image_by_index(p, dst)), memory_ptr(image_image_by_index(p, src)), image_width_by_index(p, src), image_height_by_index(p, src), dw, dh, method);
    break;
  default:
    show_message_fnc("unsupported image type %s\n", image_type_to_string(p->type));
    return 0;
  }

  image_width_by_index(p, dst)  = dw;
  image_height_by_index(p, dst) = dh;

  return 1;
}
