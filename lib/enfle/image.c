/*
 * image.c -- image interface
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Jun 16 02:34:59 2001.
 * $Id: image.c,v 1.9 2001/06/15 18:46:18 sian Exp $
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

int image_magnify_main(Image *, int, int, ImageInterpolateMethod);

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
  "_BGRA32",
  "_YUV420",
  "_YUV420P",
  "_YVU420",
  "_YVU420P"
};

static Image *duplicate(Image *);
static int compare(Image *, Image *);
static int magnify(Image *, int, int, ImageInterpolateMethod);
static void destroy(Image *);

static Image template = {
  duplicate: duplicate,
  compare: compare,
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

  if (p->rendered.image && (new->rendered.image = memory_dup_as_shm(p->rendered.image)) == NULL)
    goto error;

  if (p->magnified.image && (new->magnified.image = memory_dup(p->magnified.image)) == NULL)
    goto error;

  if (p->image && (new->image = memory_dup(p->image)) == NULL)
    goto error;

  if (p->mask && (new->mask = memory_dup(p->mask)) == NULL)
    goto error;

  if (p->comment && (new->comment = strdup(p->comment)) == NULL)
    goto error;

  if (p->next && (new->next = duplicate(p->next)) == NULL)
    goto error;

  return new;

 error:
  destroy(new);
  return NULL;
}

static int
compare(Image *p, Image *p2)
{
  if (p->width != p2->width || p->height != p2->height || p->type != p2->type)
    return 0;
  return 1;
}

static int
magnify(Image *p, int dw, int dh, ImageInterpolateMethod method)
{
  return image_magnify_main(p, dw, dh, method);
}

static void
destroy(Image *p)
{
  if (p->rendered.image)
    memory_destroy(p->rendered.image);
  if (p->magnified.image)
    memory_destroy(p->magnified.image);
  if (p->image)
    memory_destroy(p->image);
  if (p->mask)
    memory_destroy(p->mask);
  if (p->comment)
    free(p->comment);
  free(p);
}
