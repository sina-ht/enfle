/*
 * yuv2rgb.c
 *
 * RGB to YUV Conversion
 *
 * Y =   0.2990R + 0.5870G + 0.1140B
 * U = - 0.1684R - 0.3316G + 0.5000B + 128
 * V =   0.5000R - 0.4187G - 0.0813B + 128
 *
 * Y = (  9798 * R + 19235 * G +  3736 * B) >> 15
 * U = (- 5518 * R - 10866 * G + (B << 15)) >> 15 + 128
 * V = ( (R << 15) - 13720 * G -  2664 * B) >> 15 + 128
 *
 * YUV to RGB Conversion
 *
 * R = ((Y << 14) + 22970 * (V - 128)) >> 14
 * G = ((Y << 14) -  5638 * (U - 128) - 11697 * (V - 128)) >> 14
 * B = ((Y << 14) + 29029 * (U - 128)) >> 14
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
 */

#include <stdio.h>
#include "divxdecore/yuv2rgb.h"

/* YUV12 -> ARGB32 */
void
yuv2rgb_32(uint8_t *y, int stride_y,
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
#if 0
      out++;
      *out++ = ((yl[j] << 14) + 22970 * (vl[j >> 1] - 128)) >> 14;
      *out++ = ((yl[j] << 14) +  5638 * (ul[j >> 1] - 128) - 11697 * (vl[j >> 1] - 128)) >> 14;
      *out++ = ((yl[j] << 14) + 29029 * (ul[j >> 1] - 128)) >> 14;
#else
      r = ((yl[j] << 14) + 22970 * (vl[j >> 1] - 128)) >> 14;
      g = ((yl[j] << 14) +  5638 * (ul[j >> 1] - 128) - 11697 * (vl[j >> 1] - 128)) >> 14;
      b = ((yl[j] << 14) + 29029 * (ul[j >> 1] - 128)) >> 14;
      if (r < 0)
	r = 0;
      else if (r > 255)
	r = 255;
      if (g < 0)
	g = 0;
      else if (g > 255)
	g = 255;
      if (b < 0)
	b = 0;
      else if (b > 255)
	b = 255;
      out[1] = r;
      out[2] = g;
      out[3] = b;
      out += 4;
#endif
    }
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

  yl = y;
  ul = u;
  vl = v;
  for (i = 0; i < height; i++) {
    for (j = 0; j < width; j++) {
      *out++ = ((yl[j] << 14) + 22970 * (vl[j >> 1] - 128)) >> 14;
      *out++ = ((yl[j] << 14) +  5638 * (ul[j >> 1] - 128) - 11697 * (vl[j >> 1] - 128)) >> 14;
      *out++ = ((yl[j] << 14) + 29029 * (ul[j >> 1] - 128)) >> 14;
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
  uint8_t *yl, *ul, *vl, r, g, b;
  uint16_t *outp = (uint16_t *)out;

  yl = y;
  ul = u;
  vl = v;
  for (i = 0; i < height; i++) {
    for (j = 0; j < width; j++) {
      r = ((yl[j] << 14) + 22970 * (vl[j >> 1] - 128)) >> 14;
      g = ((yl[j] << 14) +  5638 * (ul[j >> 1] - 128) - 11697 * (vl[j >> 1] - 128)) >> 14;
      b = ((yl[j] << 14) + 29029 * (ul[j >> 1] - 128)) >> 14;
      *outp++ = ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | (b >> 3);
    }
    yl += stride_y;
    if (i % 2) {
      ul += stride_uv;
      vl += stride_uv;
    }
  }
}
