/*
 * image.h -- image interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Apr 16 10:36:10 2001.
 * $Id: image.h,v 1.10 2001/04/18 05:40:04 sian Exp $
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

#ifndef _IMAGE_H
#define _IMAGE_H

#include "memory.h"

typedef enum _image_type {
  _BITMAP_LSBFirst = 0,
  _BITMAP_MSBFirst,
  _GRAY,
  _GRAY_ALPHA,
  _INDEX,
  _RGB_WITH_BITMASK,
  _BGR_WITH_BITMASK,
  _RGB24,
  _BGR24,
  _RGBA32,
  _ABGR32,
  _ARGB32,
  _BGRA32,
  _IMAGETYPE_TERMINATOR
} ImageType;

#define IMAGE_BITMAP_LSBFirst  (1 <<  0)
#define IMAGE_BITMAP_MSBFirst  (1 <<  1)
#define IMAGE_GRAY             (1 <<  2)
#define IMAGE_GRAY_ALPHA       (1 <<  3)
#define IMAGE_INDEX            (1 <<  4)
#define IMAGE_RGB_WITH_BITMASK (1 <<  5)
#define IMAGE_BGR_WITH_BITMASK (1 <<  6)
#define IMAGE_RGB24            (1 <<  7)
#define IMAGE_BGR24            (1 <<  8)
#define IMAGE_RGBA32           (1 <<  9)
#define IMAGE_ABGR32           (1 << 10)
#define IMAGE_ARGB32           (1 << 11)
#define IMAGE_BGRA32           (1 << 12)

typedef enum {
  _NOINTERPOLATE,
  _BILINEAR
} ImageInterpolateMethod;

typedef struct _image_color {
  unsigned char red;
  unsigned char green;
  unsigned char blue;
  unsigned char gray;
  unsigned char index;
} ImageColor;

typedef struct _image_data {
  unsigned int width, height;
  unsigned int bytes_per_line;
  int left, top;
  Memory *image;
} ImageData;

typedef struct _image Image;
struct _image {
  ImageType type;
  unsigned int width, height;
  unsigned int bytes_per_line;
  int left, top;
  Memory *image;
  ImageData magnified;
  ImageData rendered;
  ImageColor background_color;
  ImageColor transparent_color;
  Memory *mask;
  unsigned char *comment;
  char *format;
  const unsigned char *format_detail;
  int alpha_enabled;
  int if_magnified;
  int depth;
  int bits_per_pixel;
  int ncolors;
  unsigned char colormap[256][3];
  unsigned long red_mask, green_mask, blue_mask;
  Image *next;

  Image *(*duplicate)(Image *);
  int (*compare)(Image *, Image *);
  int (*magnify)(Image *, int, int, ImageInterpolateMethod);
  void (*destroy)(Image *);
};

#define image_dup(p) (p)->duplicate((p))
#define image_compare(p, p2) (p)->compare((p), (p2))
#define image_magnify(p, dw, dh, m) (p)->magnify((p), (dw), (dh), (m))
#define image_destroy(p) (p)->destroy((p))

Image *image_create(void);
const char *image_type_to_string(ImageType);

#endif
