/*
 * tga.c -- tga loader plugin
 * (C)Copyright 2000-2006 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Nov 18 23:05:34 2011.
 * $Id: tga.c,v 1.1 2006/03/03 16:54:03 sian Exp $
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#define UTILS_NEED_GET_LITTLE_UINT32
#define UTILS_NEED_GET_LITTLE_UINT16
#include "enfle/utils.h"
#include "enfle/stream-utils.h"
#include "enfle/loader-plugin.h"

typedef enum {
  _NO_IMAGE = 0,
  _UNCOMPRESSED_INDEX = 1,
  _UNCOMPRESSED_RGB = 2,
  _UNCOMPRESSED_BITMAP = 3,
  _RLE_INDEX = 9,
  _RLE_RGB = 10,
  _RLE_BITMAP = 11
} TGAImageType;

//static const unsigned int types = (IMAGE_BGRA32 | IMAGE_BGR24 | IMAGE_INDEX | IMAGE_BITMAP_MSBFirst);

DECLARE_LOADER_PLUGIN_METHODS;

static LoaderPlugin plugin = {
  .type = ENFLE_PLUGIN_LOADER,
  .name = "TGA",
  .description = "TGA Loader plugin version 0.1",
  .author = "Hiroshi Takekawa",
  .image_private = NULL,

  .identify = identify,
  .load = load
};

ENFLE_PLUGIN_ENTRY(loader_tga)
{
  LoaderPlugin *lp;

  if ((lp = (LoaderPlugin *)calloc(1, sizeof(LoaderPlugin))) == NULL)
    return NULL;
  memcpy(lp, &plugin, sizeof(LoaderPlugin));

  return (void *)lp;
}

ENFLE_PLUGIN_EXIT(loader_tga, p)
{
  free(p);
}

/* for internal use */

static int
load_image(Image *p, Stream *st)
{
  unsigned char buf[18];
  unsigned int id_len, color_map_type;
  TGAImageType image_type;
  unsigned int cm_first, cm_nentries, cm_size;
  unsigned int image_desc;
  unsigned char *pp, *d;
  int i;
#if 0
  unsigned int bytes_per_pal;
#endif

  if (stream_read(st, buf, 18) != 18)
    return 0;
  id_len = buf[0];
  color_map_type = buf[1];
  image_type = buf[2];
  cm_first = utils_get_little_uint16(&buf[3]);
  cm_first = cm_first; // dummy
  cm_nentries = utils_get_little_uint16(&buf[5]);
  cm_size = buf[7];
  image_left(p) = utils_get_little_uint16(&buf[8]);
  image_top(p) = utils_get_little_uint16(&buf[10]);
  image_width(p) = utils_get_little_uint16(&buf[12]);
  image_height(p) = utils_get_little_uint16(&buf[14]);
  p->bits_per_pixel = buf[16];
  image_desc = buf[17];

  debug_message_fnc("TGA (%d,%d) type %d bpp %d desc %02X detected\n", image_width(p), image_height(p), image_type, p->bits_per_pixel, image_desc);

  if (image_type != _UNCOMPRESSED_RGB) {
    err_message_fnc("Unsupported TGA image type %d\n", image_type);
    return 0;
  }

  if ((image_desc & 0xc0) != 0)
    warning_fnc("Interleaved image.  Probably displayed incorrectly.\n");

  /* Skip Image ID */
  if (!stream_seek(st, id_len, _CUR))
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
    p->type = _RGB555;
    p->depth = 16;
    break;
  case 24:
    p->type = _BGR24;
    p->depth = 24;
    break;
  case 32:
    p->type = _BGRA32;
    p->depth = 24;
    break;
  default:
    show_message("tga: unknown bpp %d detected.\n", p->bits_per_pixel);
    return 0;
  }

  /* Read color map here */
  if (color_map_type != 0) {
#if 0
    if (image_type == _UNCOMPRESSED_INDEX) {
      p->ncolors = 1 << p->depth;
      bytes_per_pal = 4; // 3
      if (stream_read(st, buf, p->ncolors * bytes_per_pal) != (int)(p->ncolors * bytes_per_pal))
	return 0;
      for (i = 0; i < (int)p->ncolors; i++) {
	p->colormap[i][0] = buf[bytes_per_pal * i + 2];
	p->colormap[i][1] = buf[bytes_per_pal * i + 1];
	p->colormap[i][2] = buf[bytes_per_pal * i + 0];
      }
    }
#endif
    /* Just skip color map for RGB */
    if (!stream_seek(st, cm_size * cm_nentries, _CUR))
      return 0;
  }

  image_bpl(p) = (image_width(p) * p->bits_per_pixel) >> 3;
  if ((d = memory_alloc(image_image(p), image_bpl(p) * image_height(p))) == NULL)
    return 0;

  switch (image_desc & 0x30) {
  case 0x00: /* bottom left */
  case 0x10: /* bottom right, should be flipped */
    pp = d + image_bpl(p) * (image_height(p) - 1);
    for (i = (int)(image_height(p) - 1); i >= 0; i--) {
      stream_read(st, pp, image_bpl(p));
      pp -= image_bpl(p);
    }
    break;
  case 0x20: /* top left */
  case 0x30: /* top right, should be flipped */
    stream_read(st, d, image_bpl(p) * image_height(p));
    break;
  }

  return 1;
}

/* methods */

DEFINE_LOADER_PLUGIN_IDENTIFY(p, st, vw, c, priv)
{
  unsigned char buf[26];

  if (!stream_seek(st, -26, _END))
    return LOAD_ERROR;
  if (stream_read(st, buf, 26) != 26)
    return LOAD_ERROR;
  if (memcmp(buf + 8, "TRUEVISION-XFILE", 16) != 0)
    return LOAD_NOT;

  return LOAD_OK;
}

DEFINE_LOADER_PLUGIN_LOAD(p, st, vw, c, priv)
{
  debug_message("tga loader: load() called\n");

  stream_rewind(st);
  if (!load_image(p, st))
    return LOAD_ERROR;

  return LOAD_OK;
}
