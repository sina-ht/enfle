/*
 * image.c -- image interface
 * (C)Copyright 2000-2006 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Wed Mar  1 00:40:21 2006.
 * $Id: image.c,v 1.19 2006/03/12 08:24:16 sian Exp $
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

#include <stdlib.h>

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "image.h"

static const char *image_type_to_string_array[] = {
  "_BITMAP_LSBFirst",
  "_BITMAP_MSBFirst",
  "_GRAY",
  "_GRAY_ALPHA",
  "_INDEX",
  "_RGB555",
  "_BGR555",
  "_RGB565",
  "_BGR565",
  "_RGB24",
  "_BGR24",
  "_RGBA32",
  "_ABGR32",
  "_ARGB32",
  "_BGRA32",
  "_YUY2",
  "_YV12",
  "_I420",
  "_UYVY"
};

Image *
image_create(void)
{
  return calloc(1, sizeof(Image));
}

const char *
image_type_to_string(ImageType type)
{
  if (type >= _IMAGETYPE_TERMINATOR)
    return "Invalid ImageType";
  return image_type_to_string_array[(int)type];
}

/* methods */

Image *
image_dup(Image *p)
{
  Image *new;

  if ((new = image_create()) == NULL)
    return NULL;
  memcpy(new, p, sizeof(Image));

  if (image_rendered_image(p) &&
      (image_rendered_image(new) = memory_dup_as_shm(image_rendered_image(p))) == NULL)
    goto error;

  if (image_work_image(p) &&
      (image_work_image(new) = memory_dup(image_work_image(p))) == NULL)
    goto error;

  if (image_image(p) && (image_image(new) = memory_dup(image_image(p))) == NULL)
    goto error;

  if (image_mask_image(p) && (image_mask_image(new) = memory_dup(image_mask_image(p))) == NULL)
    goto error;

  if (p->comment && (new->comment = strdup((const char *)p->comment)) == NULL)
    goto error;

  return new;

 error:
  image_destroy(new);
  return NULL;
}

int
image_compare(Image *p, Image *p2)
{
  if (image_width(p) != image_width(p2) || image_height(p) != image_height(p2) || p->type != p2->type)
    return 0;
  return 1;
}

int
image_data_alloc_from_other(Image *p, int src, int dst)
{
  Memory *s, *d;

  s = image_image_by_index(p, src);
  if (!(d = image_image_by_index(p, dst))) {
    d = image_image_by_index(p, dst) = memory_create();
    if (!d)
      return 0;
  }
  if (!memory_alloc(d, memory_used(s)))
    return 0;
  
  image_width_by_index(p, dst) = image_width_by_index(p, src);
  image_height_by_index(p, dst) = image_height_by_index(p, src);
  image_left_by_index(p, dst) = image_left_by_index(p, src);
  image_top_by_index(p, dst) = image_top_by_index(p, src);
  image_bpl_by_index(p, dst) = image_bpl_by_index(p, src);

  return 1;
}

int
image_data_copy(Image *p, int src, int dst)
{
  Memory *s, *d;

  s = image_image_by_index(p, src);
  if (image_image_by_index(p, dst))
    memory_destroy(image_image_by_index(p, dst));
  d = image_image_by_index(p, dst) = memory_dup(s);
  if (!d)
    return 0;

  image_width_by_index(p, dst) = image_width_by_index(p, src);
  image_height_by_index(p, dst) = image_height_by_index(p, src);
  image_left_by_index(p, dst) = image_left_by_index(p, src);
  image_top_by_index(p, dst) = image_top_by_index(p, src);
  image_bpl_by_index(p, dst) = image_bpl_by_index(p, src);

  return 1;
}

int
image_data_swap(Image *p, int idx1, int idx2)
{
  ImageData tmp;

  memcpy(&tmp, &image_by_index(p, idx1), sizeof(ImageData));
  memcpy(&image_by_index(p, idx1), &image_by_index(p, idx2), sizeof(ImageData));
  memcpy(&image_by_index(p, idx2), &tmp, sizeof(ImageData));

  return 1;
}

void
image_clean(Image *p)
{
  memory_free(image_work_image(p));
}

void
image_destroy(Image *p)
{
  if (image_rendered_image(p))
    memory_destroy(image_rendered_image(p));
  if (image_work_image(p))
    memory_destroy(image_work_image(p));
  if (image_image(p))
    memory_destroy(image_image(p));
  if (image_mask_image(p))
    memory_destroy(image_mask_image(p));
  if (p->comment)
    free(p->comment);
  free(p);
}
