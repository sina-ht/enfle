/*
 * image.h -- image interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Oct  9 01:35:54 2000.
 * $Id: image.h,v 1.2 2000/10/08 17:22:16 sian Exp $
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

typedef enum {
  _NORMAL,
  _BILINEAR
} ImageInterpolateMethod;

typedef struct _image_color {
  unsigned char red;
  unsigned char green;
  unsigned char blue;
  unsigned char gray;
  unsigned char index;
} ImageColor;

typedef struct _image Image;
struct _image {
  int width, height;
  int left, top;
  unsigned char *image;
  unsigned int image_size;
  unsigned char *mask;
  unsigned int mask_size;
  unsigned char *comment;
  char *format;
  ImageType type;
  ImageColor background_color;
  ImageColor transparent_color;
  int alpha_enabled;
  int depth;
  int bits_per_pixel;
  int bytes_per_line;
  int ncolors;
  unsigned char colormap[256][3];
  unsigned long red_mask, green_mask, blue_mask;
  Image *next;

  Image *(*dup)(Image *);
  Image *(*magnify)(Image *, int, int, ImageInterpolateMethod);
  void (*destroy)(Image *);
};

#define image_dup(p) (p)->dup((p))
#define image_magnify(p, dw, dh, m) (p)->magnify((p), (dw), (dh), (m))
#define image_destroy(p) (p)->destroy((p))

Image *image_create(void);
char *image_type_to_string(ImageType);

#endif
