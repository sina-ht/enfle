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
 *  V Sample Period      2          2
 *  U Sample Period      2          2
 */

#include <stdio.h>
#include "yuv2rgb.h"

#ifndef YUV2RGB_32_MMX
void
yuv2rgb_32(uint8_t *y, int stride_y,
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
      out++;
    }
    yl += stride_y;
    if (i % 2) {
      ul += stride_uv;
      vl += stride_uv;
    }
  }
}
#endif

#ifndef YUV2RGB_24_MMX
void
yuv2rgb_24(uint8_t *y, int stride_y,
	   uint8_t *u, uint8_t *v, int stride_uv,
	   uint8_t *out, int width, int height)
{
  int i, j;
  uint8_t *yl, *ul, *vl;

  //fprintf(stderr, __FUNCTION__ ": %p %d %p %p %d %p (%d x %d)\n", y, stride_y, u, v, stride_uv, out, width, height);

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
#endif

#ifndef YUV2RGB_16_MMX
void
yuv2rgb_16(uint8_t *y, int stride_y,
	   uint8_t *u, uint8_t *v, int stride_uv,
	   uint8_t *outp, int width, int height)
{
  int i, j;
  uint8_t *yl, *ul, *vl, r, g, b;
  uint16_t *out = (uint16_t *)outp;

  yl = y;
  ul = u;
  vl = v;
  for (i = 0; i < height; i++) {
    for (j = 0; j < width; j++) {
      r = ((yl[j] << 14) + 22970 * (vl[j >> 1] - 128)) >> 14;
      g = ((yl[j] << 14) +  5638 * (ul[j >> 1] - 128) - 11697 * (vl[j >> 1] - 128)) >> 14;
      b = ((yl[j] << 14) + 29029 * (ul[j >> 1] - 128)) >> 14;
      *out++ = ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | (b >> 3);
    }
    yl += stride_y;
    if (i % 2) {
      ul += stride_uv;
      vl += stride_uv;
    }
  }
}
#endif
