/*
 * webp.c -- webp loader plugin
 * (C)Copyright 2010-2016 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Wed May  4 19:55:05 2016.
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

#include <stdio.h>
#include <stdlib.h>

#include <webp/decode.h>

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "enfle/stream-utils.h"
#include "enfle/loader-plugin.h"

//static const unsigned int types = (IMAGE_BGRA32 | IMAGE_BGR24 | IMAGE_INDEX | IMAGE_BITMAP_MSBFirst);

DECLARE_LOADER_PLUGIN_METHODS;

static LoaderPlugin plugin = {
  .type = ENFLE_PLUGIN_LOADER,
  .name = "WEBP",
  .description = "WEBP Loader plugin version 0.1.1",
  .author = "Hiroshi Takekawa",
  .image_private = NULL,

  .identify = identify,
  .load = load
};

ENFLE_PLUGIN_ENTRY(loader_webp)
{
  LoaderPlugin *lp;

  if ((lp = (LoaderPlugin *)calloc(1, sizeof(LoaderPlugin))) == NULL)
    return NULL;
  memcpy(lp, &plugin, sizeof(LoaderPlugin));

  return (void *)lp;
}

ENFLE_PLUGIN_EXIT(loader_webp, p)
{
  free(p);
}

/* for internal use */

static int
load_image(Image *p, Stream *st)
{
  unsigned char *buf, *d;
  unsigned int size;
  int w, h;

  /* Read whole stream into buffer... */
  size = 0;
  buf = NULL;
  {
    unsigned char *tmp;
    int len;
    int bufsize = 65536;

    for (;;) {
      if ((tmp = realloc(buf, bufsize)) == NULL) {
        free(buf);
        return LOAD_ERROR;
      }
      buf = tmp;
      len = stream_read(st, buf + size, bufsize - size);
      size += len;
      if (len < bufsize - size)
        break;
      bufsize += 65536;
    }
  }

  d = WebPDecodeRGB(buf, size, &w, &h);
  if (d == NULL)
    return 0;

  image_width(p) = w;
  image_height(p) = h;
  p->type = _RGB24;
  p->bits_per_pixel = 24;
  p->depth = 24;

  debug_message_fnc("WEBP (%d,%d) %d bytes\n", image_width(p), image_height(p), size);

  image_bpl(p) = (image_width(p) * p->bits_per_pixel) >> 3;
  memory_set(image_image(p), d, _NORMAL, image_bpl(p) * image_height(p), image_bpl(p) * image_height(p));

  return 1;
}

/* methods */

DEFINE_LOADER_PLUGIN_IDENTIFY(p, st, vw, c, priv)
{
  unsigned char buf[16];

  if (stream_read(st, buf, 16) != 16)
    return LOAD_ERROR;
  if (memcmp(buf, "RIFF", 4) != 0)
    return LOAD_NOT;
  if (memcmp(buf + 8, "WEBPVP8 ", 8) != 0)
    return LOAD_NOT;

  return LOAD_OK;
}

DEFINE_LOADER_PLUGIN_LOAD(p, st, vw, c, priv)
{
  debug_message("webp loader: load() called\n");

  stream_rewind(st);
  if (!load_image(p, st))
    return LOAD_ERROR;

  return LOAD_OK;
}
