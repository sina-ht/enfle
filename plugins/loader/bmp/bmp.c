/*
 * bmp.c -- bmp loader plugin
 * (C)Copyright 2000, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Sep  6 18:12:26 2008.
 * $Id: bmp.c,v 1.18 2009/02/23 14:28:03 sian Exp $
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

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#define UTILS_NEED_GET_LITTLE_UINT32
#define UTILS_NEED_GET_LITTLE_UINT16
#include "enfle/utils.h"
#include "enfle/stream-utils.h"
#include "enfle/loader-plugin.h"

#define BI_RGB  0
#define BI_RLE8 1
#define BI_RLE4 2

//static const unsigned int types = (IMAGE_BGRA32 | IMAGE_BGR24 | IMAGE_INDEX | IMAGE_BITMAP_MSBFirst);

DECLARE_LOADER_PLUGIN_METHODS;

static LoaderPlugin plugin = {
  .type = ENFLE_PLUGIN_LOADER,
  .name = "BMP",
  .description = "BMP Loader plugin version 0.3",
  .author = "Hiroshi Takekawa",
  .image_private = NULL,

  .identify = identify,
  .load = load
};

ENFLE_PLUGIN_ENTRY(loader_bmp)
{
  LoaderPlugin *lp;

  if ((lp = (LoaderPlugin *)calloc(1, sizeof(LoaderPlugin))) == NULL)
    return NULL;
  memcpy(lp, &plugin, sizeof(LoaderPlugin));

  return (void *)lp;
}

ENFLE_PLUGIN_EXIT(loader_bmp, p)
{
  free(p);
}

/* for internal use */

#define WIN_BMP_HEADER_SIZE 40
#define BUFSIZE 1024
static int
load_image(Image *p, Stream *st)
{
  unsigned char buf[BUFSIZE], *pp, *d;
  unsigned int file_size, header_size, image_size, offset_to_image;
  unsigned short int biPlanes;
  unsigned int bytes_per_pal;
  int i, c, c2, y, compress_method;

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
  if (stream_read(st, buf, header_size - 4) != (int)(header_size - 4))
    return 0;

  if (header_size >= WIN_BMP_HEADER_SIZE) {
    image_width(p) = utils_get_little_uint32(&buf[0]);
    image_height(p) = utils_get_little_uint32(&buf[4]);
    biPlanes = utils_get_little_uint16(&buf[8]);
    p->bits_per_pixel = utils_get_little_uint16(&buf[10]);
  } else {
    image_width(p) = utils_get_little_uint16(&buf[0]);
    image_height(p) = utils_get_little_uint16(&buf[2]);
    biPlanes = utils_get_little_uint16(&buf[4]);
    p->bits_per_pixel = utils_get_little_uint16(&buf[6]);
  }

  if (biPlanes != 1)
    return 0;
  if (image_width(p) > 10000 || image_height(p) > 10000)
    return 0;

  switch (p->bits_per_pixel) {
  case 1:
    p->type = _BITMAP_MSBFirst; /* XXX: check order */
    p->depth = p->bits_per_pixel;
    break;
  case 4:
    p->type = _INDEX44;
    p->depth = 4;
    break;
  case 8:
    p->type = _INDEX;
    p->depth = 8;
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
    show_message("bmp: read_header: unknown bpp %d detected.\n", p->bits_per_pixel);
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
    if (p->ncolors * bytes_per_pal > BUFSIZE) {
      err_message_fnc("BUFSIZE is too small.  It must be greater than %d.\n", p->ncolors * bytes_per_pal);
      return 0;
    }
    if (stream_read(st, buf, p->ncolors * bytes_per_pal) != (int)(p->ncolors * bytes_per_pal))
      return 0;
    for (i = 0; i < (int)p->ncolors; i++) {
      p->colormap[i][0] = buf[bytes_per_pal * i + 2];
      p->colormap[i][1] = buf[bytes_per_pal * i + 1];
      p->colormap[i][2] = buf[bytes_per_pal * i + 0];
    }
  } else if (p->depth == 16 && compress_method == 3) {
    /* Read bitmasks */
    if (stream_read(st, buf, 3 * 4) != 3 * 4)
      return 0;
    unsigned int rm = utils_get_little_uint32(buf);
    unsigned int gm = utils_get_little_uint32(buf + 4);
    unsigned int bm = utils_get_little_uint32(buf + 8);
    if (rm == 0xf800)
      p->type = _RGB565;
    else if (rm == 0x7c00)
      p->type = _RGB555;
    else
      warning_fnc("Mask: R %X G %X B %X is not supported\n", rm, gm, bm);
    compress_method = 0;
  }

  image_bpl(p) = (image_width(p) * p->bits_per_pixel) >> 3;
  image_bpl(p) += (4 - (image_bpl(p) % 4)) % 4;

  debug_message_fnc("(%d, %d): bpp %d depth %d compress %d\n", image_width(p), image_height(p), p->bits_per_pixel, p->depth, compress_method);

  if ((d = memory_alloc(image_image(p), image_bpl(p) * image_height(p))) == NULL)
    return 0;

  stream_seek(st, offset_to_image, _SET);
  pp = d + image_bpl(p) * (image_height(p) - 1);

  switch (compress_method) {
  case BI_RGB:
    for (i = (int)(image_height(p) - 1); i >= 0; i--) {
      stream_read(st, pp, image_bpl(p));
      pp -= image_bpl(p);
    }
    break;
  case BI_RLE4:
    if (p->depth != 4)
      show_message("Compressed RI_BLE4 bitmap with depth %d != 4.\n", p->depth);
    /* RLE compressed data */
    /* Not complete. */
    y = image_height(p);
    while ((c = stream_getc(st)) != -1 && y >= 0) {
      if (c != 0) {
        /* code mode */
        c2 = stream_getc(st);
	show_message_fnc("len %d data %d\n", c, c2);
        for (i = 0; i < c; i += 2) {
	  *pp++ = (unsigned char)c2;
        }
      } else {
	if ((c = stream_getc(st)) == 0) {
	  /* line end */
	  show_message_fnc("line end %d\n", y);
	  pp = d + image_bpl(p) * (y - 1);
	  y--;
	} else if (c == 1) {
	  /* image end */
	  break;
	} else if (c == 2) {
	  /* offset */
	  c = stream_getc(st);
	  c2 = stream_getc(st);
	  show_message_fnc("offset %d, %d\n", c, c2);
	  pp += (c - c2 * image_width(p)) / 2;
	} else {
	  /* absolute mode */
	  show_message_fnc("abs len %d\n", c);
	  for (i = 0; i < c; i += 2)
	    *pp++ = (unsigned char)stream_getc(st);
	  if (c % 2 != 0)
	    /* read dummy */
	    c = stream_getc(st);
	}
      }
    }
    break;
  case BI_RLE8:
    if (p->depth != 8)
      show_message("Compressed RI_BLE8 bitmap with depth %d != 8.\n", p->depth);
    /* RLE compressed data */
    y = image_height(p);
    while ((c = stream_getc(st)) != -1 && y >= 0) {
      if (c != 0) {
	/* code mode */
	c2 = stream_getc(st);
	for (i = 0; i < c; i++) {
	  *pp++ = (unsigned char)c2;
	}
      } else {
	if ((c = stream_getc(st)) == 0) {
	  /* line end */
	  pp = d + image_bpl(p) * (y - 1);
	  y--;
	} else if (c == 1) {
	  /* image end */
	  break;
	} else if (c == 2) {
	  /* offset */
	  c = stream_getc(st);
	  c2 = stream_getc(st);
	  pp += (c - c2 * image_width(p));
	} else {
	  /* absolute mode */
	  for (i = 0; i < c; i++)
	    *pp++ = (unsigned char)stream_getc(st);
	  if (c % 2 != 0)
	    /* read dummy */
	    c = stream_getc(st);
	}
      }
    }
    break;
  default:
    show_message("Compressed bitmap (method = %d) not supported.\n", compress_method);
    return 0;
  }

  return 1;
}

/* methods */

DEFINE_LOADER_PLUGIN_IDENTIFY(p, st, vw, c, priv)
{
  unsigned char buf[2];

  if (stream_read(st, buf, 2) != 2)
    return LOAD_NOT;

  if (memcmp(buf, "BM", 2) != 0)
    return LOAD_NOT;

  return LOAD_OK;
}

DEFINE_LOADER_PLUGIN_LOAD(p, st, vw, c, priv)
{
  LoaderStatus status;

  debug_message("bmp loader: load() called\n");

  /* identify() must be called() */
  if ((status = identify(p, st, vw, c, priv)) != LOAD_OK)
    return status;

  if (!load_image(p, st))
    return LOAD_ERROR;

  return LOAD_OK;
}
