/*
 * bmp.c -- bmp loader plugin
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Dec  2 23:05:40 2000.
 * $Id: bmp.c,v 1.4 2000/12/03 08:40:04 sian Exp $
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

#include "common.h"

#include "utils.h"
#include "stream-utils.h"
#include "loader-plugin.h"

DECLARE_LOADER_PLUGIN_METHODS;

static LoaderPlugin plugin = {
  type: ENFLE_PLUGIN_LOADER,
  name: "BMP",
  description: "BMP Loader plugin version 0.2",
  author: "Hiroshi Takekawa",

  identify: identify,
  load: load
};

void *
plugin_entry(void)
{
  LoaderPlugin *lp;

  if ((lp = (LoaderPlugin *)calloc(1, sizeof(LoaderPlugin))) == NULL)
    return NULL;
  memcpy(lp, &plugin, sizeof(LoaderPlugin));

  return (void *)lp;
}

void
plugin_exit(void *p)
{
  free(p);
}

/* for internal use */

#define WIN_BMP_HEADER_SIZE 40
static int
load_image(Image *p, Stream *st)
{
  unsigned char buf[1024], *pp, *d;
  unsigned int file_size, header_size, image_size, offset_to_image;
  unsigned short int biPlanes;
  int i, bytes_per_pal;
  int compress_method;

  if (stream_read(st, buf, 12) != 12)
    return 0;

  file_size = utils_get_little_uint32(&buf[0]);
  offset_to_image = utils_get_little_uint32(&buf[8]);
  if (file_size < offset_to_image)
    return 0;

  if (!stream_read_little_uint32(st, &header_size))
    return 0;
  if (header_size > 64)
    return 0;
  if (stream_read(st, buf, header_size - 4) != header_size - 4)
    return 0;

  if (header_size >= WIN_BMP_HEADER_SIZE) {
    p->width = utils_get_little_uint32(&buf[0]);
    p->height = utils_get_little_uint32(&buf[4]);
    biPlanes = utils_get_little_uint16(&buf[8]);
    p->bits_per_pixel = utils_get_little_uint16(&buf[10]);
  } else {
    p->width = utils_get_little_uint16(&buf[0]);
    p->height = utils_get_little_uint16(&buf[2]);
    biPlanes = utils_get_little_uint16(&buf[4]);
    p->bits_per_pixel = utils_get_little_uint16(&buf[6]);
  }

  if (biPlanes != 1)
    return 0;

  switch (p->bits_per_pixel) {
  case 1:
    p->type = _BITMAP_MSBFirst; /* XXX: check order */
    p->depth = p->bits_per_pixel;
  case 4:
  case 8:
    p->type = _INDEX;
    p->depth = p->bits_per_pixel;
    break;
  case 16:
    fprintf(stderr, "bmp: read_header: bpp 16 detected.\n");
    return 0;
  case 24:
    p->type = _BGR24;
    p->depth = 24;
    break;
  case 32:
    p->type = _BGRA32;
    p->depth = 24;
    break;
  default:
    fprintf(stderr, "bmp: read_header: unknown bpp %d detected.\n", p->bits_per_pixel);
    return 0;
  }

  compress_method = 0;
  if (header_size >= WIN_BMP_HEADER_SIZE) {
    compress_method = utils_get_little_uint16(&buf[12]);
    image_size = utils_get_little_uint32(&buf[16]);
  }
  /* other header informations are intentionally ignored */

  if (p->depth <= 8) {
    p->ncolors = 1 << p->depth;
    bytes_per_pal = (header_size >= WIN_BMP_HEADER_SIZE) ? 4 : 3;
    if (stream_read(st, buf, p->ncolors * bytes_per_pal) != p->ncolors * bytes_per_pal)
      return 0;
    for (i = 0; i < p->ncolors; i++) {
      p->colormap[i][0] = buf[bytes_per_pal * i + 2];
      p->colormap[i][1] = buf[bytes_per_pal * i + 1];
      p->colormap[i][2] = buf[bytes_per_pal * i + 0];
    }
  }

  p->bytes_per_line = (p->width * p->bits_per_pixel) >> 3;
  p->bytes_per_line += (4 - (p->bytes_per_line % 4)) % 4;

  if ((d = memory_alloc(p->image, p->bytes_per_line * p->height)) == NULL)
    return 0;

  stream_seek(st, offset_to_image, _SET);
  pp = d + p->bytes_per_line * (p->height - 1);
  if (!compress_method) {
    for (i = p->height - 1; i >= 0; i--) {
      stream_read(st, pp, p->bytes_per_line);
      pp -= p->bytes_per_line;
    }
  } else {
    fprintf(stderr, "Compressed bitmap not yet supported.\n");
    return 0;
  }

  p->next = NULL;

  return 1;
}

/* methods */

DEFINE_LOADER_PLUGIN_IDENTIFY(p, st, priv)
{
  char buf[2];

  if (stream_read(st, buf, 2) != 2)
    return LOAD_NOT;

  if (memcmp(buf, "BM", 2) != 0)
    return LOAD_NOT;

  return LOAD_OK;
}

DEFINE_LOADER_PLUGIN_LOAD(p, st, priv)
{
  LoaderStatus status;

  debug_message("bmp loader: load() called\n");

  /* identify() must be called() */
  if ((status = identify(p, st, priv)) != LOAD_OK)
    return status;

  if (!load_image(p, st))
    return LOAD_ERROR;

  return LOAD_OK;
}
