/*
 * rotate.c -- Rotate Effect plugin
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Apr 16 21:12:07 2001.
 * $Id: rotate.c,v 1.1 2001/04/18 05:23:57 sian Exp $
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

#include <sys/stat.h>
#include <errno.h>

#define REQUIRE_STRING_H
#define REQUIRE_UNISTD_H
#include "compat.h"

#include "common.h"

#include "utils/libstring.h"
#include "enfle/effect-plugin.h"

static Image *effect(Image *, void *);

static EffectPlugin plugin = {
  type: ENFLE_PLUGIN_EFFECT,
  name: "Rotate",
  description: "Rotate Effect plugin version 0.1",
  author: "Hiroshi Takekawa",

  effect: effect
};

void *
plugin_entry(void)
{
  EffectPlugin *ep;

  if ((ep = (EffectPlugin *)calloc(1, sizeof(EffectPlugin))) == NULL)
    return NULL;
  memcpy(ep, &plugin, sizeof(EffectPlugin));

  return (void *)ep;
}

void
plugin_exit(void *p)
{
  free(p);
}

/* for internal use */

static int
horizontal_flip(Image *dest, Image *src)
{
  unsigned char *d = memory_ptr(dest->image);
  unsigned char *s = memory_ptr(src->image);
  int bytes_per_pixel = src->bits_per_pixel >> 3;
  unsigned int y;

  if ((bytes_per_pixel << 3) != src->bits_per_pixel)
    return 0;
  image_compare(src, dest);

  s += src->bytes_per_line * (src->height - 1);
  for (y = 0; y < src->height; y++) {
    memcpy(d, s, src->bytes_per_line);
    d += src->bytes_per_line;
    s -= src->bytes_per_line;
  }

  return 1;
}

static int
left_rotate(Image *dest, Image *src)
{
  unsigned char *d = memory_ptr(dest->image);
  unsigned char *s = memory_ptr(src->image);
  int bytes_per_pixel = src->bits_per_pixel >> 3;
  unsigned int x, y;
  int i;

  if ((bytes_per_pixel << 3) != src->bits_per_pixel)
    return 0;
  image_compare(src, dest);

  dest->width = src->height;
  dest->height = src->width;
  dest->bytes_per_line = bytes_per_pixel * dest->width;

  s += (src->width - 1) * bytes_per_pixel;
  for (x = 0; x < src->width; x++) {
    for (y = 0; y < src->height; y++) {
      for (i = 0; i < bytes_per_pixel; i++)
	d[i] = s[i];
      s += src->bytes_per_line;
      d += bytes_per_pixel;
    }
    s -= src->height * src->bytes_per_line + bytes_per_pixel;
  }

  return 1;
}

static int
vertical_flip(Image *dest, Image *src)
{
  unsigned char *d = memory_ptr(dest->image);
  unsigned char *s = memory_ptr(src->image);
  int bytes_per_pixel = src->bits_per_pixel >> 3;
  unsigned int x, y;
  int i;

  if ((bytes_per_pixel << 3) != src->bits_per_pixel)
    return 0;
  image_compare(src, dest);

  s += (src->width - 1) * bytes_per_pixel;
  for (y = 0; y < src->height; y++) {
    for (x = 0; x < src->width; x++) {
      for (i = 0; i < bytes_per_pixel; i++)
	d[i] = s[i];
      s -= bytes_per_pixel;
      d += bytes_per_pixel;
    }
    s += src->bytes_per_line + src->width * bytes_per_pixel;
    d += src->bytes_per_line - src->width * bytes_per_pixel;
  }

  return 1;
}

static int
right_rotate(Image *dest, Image *src)
{
  unsigned char *d = memory_ptr(dest->image);
  unsigned char *s = memory_ptr(src->image);

  int bytes_per_pixel = src->bits_per_pixel >> 3;
  unsigned int x, y;
  int i;

  if ((bytes_per_pixel << 3) != src->bits_per_pixel)
    return 0;
  image_compare(src, dest);

  dest->width = src->height;
  dest->height = src->width;
  dest->bytes_per_line = bytes_per_pixel * dest->width;

  s += (src->height - 1) * src->bytes_per_line;
  for (x = 0; x < src->width; x++) {
    for (y = 0; y < src->height; y++) {
      for (i = 0; i < bytes_per_pixel; i++)
	d[i] = s[i];
      s -= src->bytes_per_line;
      d += bytes_per_pixel;
    }
    s += src->height * src->bytes_per_line + bytes_per_pixel;
  }

  return 1;
}

/* methods */

static Image *
effect(Image *p, void *params)
{
  Config *c = (Config *)params;
  Image *save;
  int function, result;

  function = config_get_int(c, "/enfle/plugins/effect/rotate/function", &result);
  if (!result)
    return NULL;

  if ((save = image_dup(p)) == NULL)
    return NULL;

  switch (function) {
  case -2:
    result = horizontal_flip(p, save);
    break;
  case 2:
    result = vertical_flip(p, save);
    break;
  case -1:
    result = left_rotate(p, save);
    break;
  case 1:
    result = right_rotate(p, save);
    break;
  default:
    show_message("Rotate: " __FUNCTION__ ": Invalid function = %d\n", function);
    result = 0;
    break;
  }

  if (!result) {
    image_destroy(save);
    return NULL;
  }

  return save;
}
