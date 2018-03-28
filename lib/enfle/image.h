/*
 * image.h -- image interface header
 * (C)Copyright 2000-2006 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Wed Apr 19 00:32:37 2006.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _IMAGE_H
#define _IMAGE_H

#include "memory.h"

typedef enum _image_type {
  _BITMAP_LSBFirst = 0,
  _BITMAP_MSBFirst,
  _GRAY,
  _GRAY_ALPHA,
  _INDEX44,
  _INDEX,
  _RGB555,
  _BGR555,
  _RGB565,
  _BGR565,
  _RGB24,
  _BGR24,
  _RGBA32,
  _ABGR32,
  _ARGB32,
  _BGRA32,
  _YUY2,
  _YV12,
  _I420,
  _UYVY,
  _IMAGETYPE_TERMINATOR
} ImageType;

#define IMAGE_BITMAP_LSBFirst  (1 <<  0)
#define IMAGE_BITMAP_MSBFirst  (1 <<  1)
#define IMAGE_GRAY             (1 <<  2)
#define IMAGE_GRAY_ALPHA       (1 <<  3)
#define IMAGE_INDEX44          (1 <<  4)
#define IMAGE_INDEX            (1 <<  5)
#define IMAGE_RGB555           (1 <<  6)
#define IMAGE_BGR555           (1 <<  7)
#define IMAGE_RGB565           (1 <<  8)
#define IMAGE_BGR565           (1 <<  9)
#define IMAGE_RGB24            (1 << 10)
#define IMAGE_BGR24            (1 << 11)
#define IMAGE_RGBA32           (1 << 12)
#define IMAGE_ABGR32           (1 << 13)
#define IMAGE_ARGB32           (1 << 14)
#define IMAGE_BGRA32           (1 << 15)
#define IMAGE_YUY2             (1 << 16)
#define IMAGE_YV12             (1 << 17)
#define IMAGE_I420             (1 << 18)
#define IMAGE_UYVY             (1 << 19)

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
  unsigned int swidth, sheight;
  unsigned int left, top;
  unsigned int bytes_per_line;
  Memory *image;
} ImageData;

typedef enum {
  _NOTHING_DISPOSAL,
  _LEFTIMAGE,
  _RESTOREBACKGROUND,
  _RESTOREPREVIOUS
} ImageTransparentDisposal;

#define IMAGE_INDEX_ORIGINAL 0
#define IMAGE_INDEX_RENDERED 1
#define IMAGE_INDEX_WORK 2
#define IMAGE_INDEX_MASK 3
#define IMAGE_INDEX_MAX 4
typedef struct _image Image;
struct _image {
  ImageType type;
  ImageData data[IMAGE_INDEX_MAX];
  ImageColor background_color;
  ImageColor transparent_color;
  ImageTransparentDisposal transparent_disposal;
  char *comment;
  char *format, *format_detail;
  int alpha_enabled;
  int direct_renderable;
  int depth;
  int bits_per_pixel;
  unsigned int ncolors;
  unsigned char colormap[256][3];
};

Image *image_create(void);
const char *image_type_to_string(ImageType);

Image *image_dup(Image *);
int image_compare(Image *, Image *);
int image_magnify(Image *, int, int, int, int, ImageInterpolateMethod);
int image_data_alloc_from_other(Image *, int, int);
int image_data_copy(Image *, int, int);
int image_data_swap(Image *, int, int);
void image_clean(Image *);
void image_destroy(Image *);

#define image_by_index(p, idx) ((p)->data[idx])
#define image_width_by_index(p, idx) (image_by_index(p,idx).width)
#define image_height_by_index(p, idx) (image_by_index(p,idx).height)
#define image_left_by_index(p, idx) (image_by_index(p,idx).left)
#define image_top_by_index(p, idx) (image_by_index(p,idx).top)
#define image_bpl_by_index(p, idx) (image_by_index(p,idx).bytes_per_line)
#define image_image_by_index(p, idx) (image_by_index(p,idx).image)
#define image_original(p) (image_by_index(p,IMAGE_INDEX_ORIGINAL))
#define image_original_width(p) (image_width_by_index(p,IMAGE_INDEX_ORIGINAL))
#define image_original_height(p) (image_height_by_index(p,IMAGE_INDEX_ORIGINAL))
#define image_original_left(p) (image_left_by_index(p,IMAGE_INDEX_ORIGINAL))
#define image_original_top(p) (image_top_by_index(p,IMAGE_INDEX_ORIGINAL))
#define image_original_bpl(p) (image_bpl_by_index(p,IMAGE_INDEX_ORIGINAL))
#define image_original_image(p) (image_image_by_index(p,IMAGE_INDEX_ORIGINAL))
#define image_work(p) (image_by_index(p,IMAGE_INDEX_WORK))
#define image_work_width(p) (image_width_by_index(p,IMAGE_INDEX_WORK))
#define image_work_height(p) (image_height_by_index(p,IMAGE_INDEX_WORK))
#define image_work_left(p) (image_left_by_index(p,IMAGE_INDEX_WORK))
#define image_work_top(p) (image_top_by_index(p,IMAGE_INDEX_WORK))
#define image_work_bpl(p) (image_bpl_by_index(p,IMAGE_INDEX_WORK))
#define image_work_image(p) (image_image_by_index(p,IMAGE_INDEX_WORK))
#define image_rendered(p) (image_by_index(p,IMAGE_INDEX_RENDERED))
#define image_rendered_width(p) (image_width_by_index(p,IMAGE_INDEX_RENDERED))
#define image_rendered_height(p) (image_height_by_index(p,IMAGE_INDEX_RENDERED))
#define image_rendered_left(p) (image_left_by_index(p,IMAGE_INDEX_RENDERED))
#define image_rendered_top(p) (image_top_by_index(p,IMAGE_INDEX_RENDERED))
#define image_rendered_bpl(p) (image_bpl_by_index(p,IMAGE_INDEX_RENDERED))
#define image_rendered_image(p) (image_image_by_index(p,IMAGE_INDEX_RENDERED))
#define image_rendered_set_image(p, m) image_image_by_index(p,IMAGE_INDEX_RENDERED) = (m)
#define image_mask_width(p) (image_width_by_index(p,IMAGE_INDEX_MASK))
#define image_mask_height(p) (image_height_by_index(p,IMAGE_INDEX_MASK))
#define image_mask_left(p) (image_left_by_index(p,IMAGE_INDEX_MASK))
#define image_mask_top(p) (image_top_by_index(p,IMAGE_INDEX_MASK))
#define image_mask_bpl(p) (image_bpl_by_index(p,IMAGE_INDEX_MASK))
#define image_mask_image(p) (image_image_by_index(p,IMAGE_INDEX_MASK))
#define image_mask_set_image(p, m) image_image_by_index(p,IMAGE_INDEX_MASK) = (m)
#define image_renderable_image(p) (p)->direct_renderable ? image_rendered_image(p) : image_image(p)

#define image_width(p) image_original_width(p)
#define image_height(p) image_original_height(p)
#define image_left(p) image_original_left(p)
#define image_top(p) image_original_top(p)
#define image_bpl(p) image_original_bpl(p)
#define image_image(p) image_original_image(p)

#endif
