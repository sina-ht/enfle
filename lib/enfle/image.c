/*
 * image.c -- image interface
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Dec  3 16:33:26 2000.
 * $Id: image.c,v 1.4 2000/12/03 08:40:04 sian Exp $
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

#include "common.h"

#include "image.h"

void *image_magnify_main(Image *, int, int, ImageInterpolateMethod);

static const char *image_type_to_string_array[] = {
  "_BITMAP_LSBFirst",
  "_BITMAP_MSBFirst",
  "_GRAY",
  "_GRAY_ALPHA",
  "_INDEX",
  "_RGB_WITH_BITMASK",
  "_BGR_WITH_BITMASK",
  "_RGB24",
  "_BGR24",
  "_RGBA32",
  "_ABGR32",
  "_ARGB32",
  "_BGRA32"
};

static Image *duplicate(Image *);
static Image *magnify(Image *, int, int, ImageInterpolateMethod);
static void destroy(Image *);

static Image template = {
  duplicate: duplicate,
  magnify: magnify,
  destroy: destroy
};

Image *
image_create(void)
{
  Image *p;

  if ((p = calloc(1, sizeof(Image))) == NULL)
    return NULL;
  memcpy(p, &template, sizeof(Image));

  if ((p->image = memory_create()) == NULL) {
    free(p);
    return NULL;
  }

  if ((p->mask = memory_create()) == NULL) {
    free(p);
    memory_destroy(p->mask);
    return NULL;
  }

  return p;
}

const char *
image_type_to_string(ImageType type)
{
  if (type < 0)
    return "Negative ImageType";
  if (type >= _IMAGETYPE_TERMINATOR)
    return "Invalid ImageType";
  return image_type_to_string_array[(int)type];
}

/* methods */

static Image *
duplicate(Image *p)
{
  Image *new;

  if ((new = image_create()) == NULL)
    return NULL;
  memcpy(new, p, sizeof(Image));

  if ((new->image = memory_dup_as_shm(p->image)) == NULL) {
    destroy(new);
    return NULL;
  }

  if (p->next) {
    if ((new->next = duplicate(p->next)) == NULL) {
      destroy(new);
      return NULL;
    }
  }

  return new;
}

static Image *
magnify(Image *p, int dw, int dh, ImageInterpolateMethod method)
{
  Image *new = image_dup(p);

  if (new == NULL)
    return NULL;
  image_magnify_main(new, dw, dh, method);

  return new;
}

static void
destroy(Image *p)
{
  if (p->image)
    memory_destroy(p->image);
  if (p->mask)
    memory_destroy(p->mask);
  if (p->comment)
    free(p->comment);
  free(p);
}
