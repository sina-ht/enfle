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
 * R = 1.164*(Y - 16) + 1.596*(V-128)
 * G = 1.164*(Y - 16) - 0.813*(U-128) - 0.391*(V-128)
 * B = 1.164*(Y - 16) + 2.018*(U-128)
 *
 * R = (74 * (Y - 16) + 102 * (V - 128)) >> 6
 * G = (74 * (Y - 16) -  52 * (U - 128) - 25 * (V - 128)) >> 6
 * B = (74 * (Y - 16) + 129 * (U - 128)) >> 6
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

#define clip(v) if (v < 0) v = 0; else if (v > 255) v = 255;

/* YUV12 -> ARGB32 */
void
yuv2rgb_32(uint8_t *y, int stride_y,
	   uint8_t *u, uint8_t *v, int stride_uv,
	   uint8_t *out, int width, int height)
{
  int i, j;
  int r, g, b;
  uint8_t *yl, *ul, *vl;
  uint32_t *outp = (uint32_t *)out;

  yl = y;
  ul = u;
  vl = v;
  for (i = 0; i < height; i++) {
    for (j = 0; j < width; j++) {
      r = ((yl[j] << 14) + 22970 * (vl[j >> 1] - 128)) >> 14;
      g = ((yl[j] << 14) +  5638 * (ul[j >> 1] - 128) - 11697 * (vl[j >> 1] - 128)) >> 14;
      b = ((yl[j] << 14) + 29029 * (ul[j >> 1] - 128)) >> 14;
      clip(r); clip(g); clip(b);
      *outp++ = (r << 16) | (g << 8) | b;
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
  int r, g, b;

  yl = y;
  ul = u;
  vl = v;
  for (i = 0; i < height; i++) {
    for (j = 0; j < width; j++) {
      r = ((yl[j] << 14) + 22970 * (vl[j >> 1] - 128)) >> 14;
      g = ((yl[j] << 14) +  5638 * (ul[j >> 1] - 128) - 11697 * (vl[j >> 1] - 128)) >> 14;
      b = ((yl[j] << 14) + 29029 * (ul[j >> 1] - 128)) >> 14;
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
      r = ((yl[j] << 14) + 22970 * (vl[j >> 1] - 128)) >> 14;
      g = ((yl[j] << 14) +  5638 * (ul[j >> 1] - 128) - 11697 * (vl[j >> 1] - 128)) >> 14;
      b = ((yl[j] << 14) + 29029 * (ul[j >> 1] - 128)) >> 14;
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
