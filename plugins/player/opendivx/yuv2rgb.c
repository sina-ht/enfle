/*
 * yuv2rgb.c -- YUV12 to RGB colorspace conversion
 * (C)Copyright 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Wed Jan 24 08:36:18 2001.
 * $Id: yuv2rgb.c,v 1.4 2001/01/23 23:50:02 sian Exp $
 *
 * Not optimized for speed
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

/*
 * YUV to RGB Conversion
 *
 * R = Y                       + 1.402   * (V - 128)
 * G = Y - 0.34414 * (U - 128) - 0.71414 * (V - 128)
 * B = Y + 1.772   * (U - 128)
 *
 * fixed point conversion
 * R = ((Y << 16) +  91881 * (V - 128)) >> 16
 * G = ((Y << 16) -  22553 * (U - 128) - 46801 * (V - 128)) >> 16
 * B = ((Y << 16) + 116129 * (U - 128)) >> 16
 *
 * various coeffecients (for reference)
 * static const int coeff_table [8][4] = {
 *    { 103219, -12257, -30659, 121621 },
 *    { 103219, -12257, -30659, 121621 }, // ITU-R Rec. 709
 *    {  91881, -22553, -46801, 116129 },
 *    {  91881, -22553, -46801, 116129 }, // reserved
 *    {  91750, -21749, -46652, 116654 }, // FCC
 *    {  91881, -22553, -46801, 116129 }, // ITU-R Rec. 624-4 System B, G
 *    {  91881, -22553, -46801, 116129 }, // SMPTE 170M
 *    { 103284, -14851, -31235, 119668 }  // SMPTE 240
 * };
 *
 * YUV12(YV12)
 *
 * This is the format of choice for many software MPEG codecs. It comprises an NxN Y
 * plane followed by (N/2)x(N/2) V and U planes.
 *
 *                   Horizontal  Vertical
 *  Y Sample Period      1          1
 *  U Sample Period      2          2
 *  V Sample Period      2          2
 *
 * RGB to YUV Conversion (for reference)
 *
 * Y =   0.299  * R + 0.587  * G + 0.114  * B
 * U = - 0.1687 * R - 0.3313 * G + 0.5    * B + 128
 * V =   0.5    * R - 0.4187 * G - 0.0813 * B + 128
 */

#include <stdio.h>
#include "divxdecore/yuv2rgb.h"

/* XXX: should use array for faster clipping */
#define clip(v) if (v < 0) v = 0; else if (v > 255) v = 255;

/* old param */
//#define RV 22970
//#define GU 5638
//#define GV 11697
//#define BU 29029
//#define SHIFT 14

#define RV 91881
#define GU 22553
#define GV 46801
#define BU 116129
#define SHIFT 16

/* XXX: could use array for faster conversion */
#define YUV2R(Y, U, V) (((Y << SHIFT) + RV * (V - 128)) >> SHIFT)
#define YUV2G(Y, U, V) (((Y << SHIFT) - GU * (U - 128) - GV * (V - 128)) >> SHIFT)
#define YUV2B(Y, U, V) (((Y << SHIFT) + BU * (U - 128)) >> SHIFT)

/* YUV12 -> ARGB32 */
void
yuv2rgb_32(uint8_t *yo, int stride_y,
	   uint8_t *uo, uint8_t *vo, int stride_uv,
	   uint8_t *out, int width, int height)
{
  int i, j;
  int r, g, b;
  uint8_t *yl, *ul, *vl;
  uint32_t *outp = (uint32_t *)out;

  yl = yo;
  ul = uo;
  vl = vo;
  for (i = 0; i < height; i++) {
#if 1
    for (j = 0; j < width; j++) {
      r = YUV2R(yl[j], ul[j >> 1], vl[j >> 1]);
      g = YUV2G(yl[j], ul[j >> 1], vl[j >> 1]);
      b = YUV2B(yl[j], ul[j >> 1], vl[j >> 1]);
      clip(r); clip(g); clip(b);
      *outp++ = (r << 16) | (g << 8) | b;
    }
#else
    for (j = 0;;) {
      int y, u, v;

      y = yl[j];
      u = ul[j >> 1]; v = vl[j >> 1];
      if (j == width)
	break;
      r = YUV2R(y, u, v);
      g = YUV2G(y, u, v);
      b = YUV2B(y, u, v);
      clip(r); clip(g); clip(b);
      *outp++ = (r << 16) | (g << 8) | b;
      j++;
      if (j == width)
	break;
      y = yl[j];
      r = YUV2R(y, u, v);
      g = YUV2G(y, u, v);
      b = YUV2B(y, u, v);
      clip(r); clip(g); clip(b);
      *outp++ = (r << 16) | (g << 8) | b;
      j++;
    }
#endif
    yl += stride_y;
    if (i % 2) {
      ul += stride_uv;
      vl += stride_uv;
    }
  }
}

void
yuv2rgb_24(uint8_t *y, int stride_y,
	   uint8_t *u, uint8_t *v, int stride_uv,
	   uint8_t *out, int width, int height)
{
  int i, j;
  uint8_t *yl, *ul, *vl;
  int r, g, b;

  yl = y;
  ul = u;
  vl = v;
  for (i = 0; i < height; i++) {
    for (j = 0; j < width; j++) {
      r = YUV2R(yl[j], ul[j >> 1], vl[j >> 1]);
      g = YUV2G(yl[j], ul[j >> 1], vl[j >> 1]);
      b = YUV2B(yl[j], ul[j >> 1], vl[j >> 1]);
      clip(r); clip(g); clip(b);
      out[0] = r; out[1] = g; out[2] = b;
      out += 3;
    }
    yl += stride_y;
    if (i % 2) {
      ul += stride_uv;
      vl += stride_uv;
    }
  }
}

void
yuv2rgb_16(uint8_t *y, int stride_y,
	   uint8_t *u, uint8_t *v, int stride_uv,
	   uint8_t *out, int width, int height)
{
  int i, j;
  int r, g, b;
  uint8_t *yl, *ul, *vl;
  uint16_t *outp = (uint16_t *)out;

  yl = y;
  ul = u;
  vl = v;
  for (i = 0; i < height; i++) {
    for (j = 0; j < width; j++) {
      r = YUV2R(yl[j], ul[j >> 1], vl[j >> 1]);
      g = YUV2G(yl[j], ul[j >> 1], vl[j >> 1]);
      b = YUV2B(yl[j], ul[j >> 1], vl[j >> 1]);
      clip(r); clip(g); clip(b);
      *outp++ = ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | (b >> 3);
    }
    yl += stride_y;
    if (i % 2) {
      ul += stride_uv;
      vl += stride_uv;
    }
  }
}
