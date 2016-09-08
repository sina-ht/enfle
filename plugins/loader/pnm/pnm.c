/*
 * pnm.c -- pnm loader plugin
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Mar  6 12:18:15 2004.
 * $Id: pnm.c,v 1.8 2004/03/06 03:43:36 sian Exp $
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
#include <ctype.h>

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "enfle/stream-utils.h"
#include "enfle/loader-plugin.h"

typedef enum _pnmencode {
  _ASCII,
  _RAW
} PNMencode;

typedef struct _pnmdata {
  int maxval;
  PNMencode encode;
} PNMdata;

//static const unsigned int types = (IMAGE_BITMAP_MSBFirst | IMAGE_GRAY | IMAGE_RGB24 | IMAGE_INDEX);

DECLARE_LOADER_PLUGIN_METHODS;

static LoaderPlugin plugin = {
  .type = ENFLE_PLUGIN_LOADER,
  .name = "PNM",
  .description = "PNM Loader plugin version 0.1",
  .author = "Hiroshi Takekawa",
  .image_private = NULL,

  .identify = identify,
  .load = load
};

ENFLE_PLUGIN_ENTRY(loader_pnm)
{
  LoaderPlugin *lp;

  if ((lp = (LoaderPlugin *)calloc(1, sizeof(LoaderPlugin))) == NULL)
    return NULL;
  memcpy(lp, &plugin, sizeof(LoaderPlugin));

  return (void *)lp;
}

ENFLE_PLUGIN_EXIT(loader_pnm, p)
{
  free(p);
}

/* for internal use */

static char *
get_token(Stream *st)
{
  static char *line = NULL;
  static int last = 0;
  static int i = 0;
  int start, len;
  char *ret;

  if (st == NULL) {
    if (line)
      free(line);
    line = NULL;
    last = 0;
    i = 0;
    return NULL;
  }

  if (i >= last) {
    for (;;) {
      if (line)
	free(line);

      if ((line = stream_gets(st)) == NULL) {
	show_message("stream_gets() returns NULL\n");
	return NULL;
      }

      if (line[0] != '#')
	break;
    }
    /* No line should be longer than 70 characters (ppm(5)), check with some room. */
    if ((last = strlen(line)) > 80)
      return NULL;
    i = 0;
  }

  while (i < last && isspace(line[i]))
    i++;

  start = i;
  while (i < last && !isspace(line[i]))
    i++;
  len = i - start;

  if ((ret = malloc(len + 1)) == NULL)
    return NULL;
  strncpy(ret, line + start, len);
  ret[len] = '\0';

  while (i < last && isspace(line[i]))
    i++;

  return ret;
}

static int
pnm_read_header(Stream *st, Image *p, PNMdata *data)
{
  char *token;
  unsigned int i;

  if ((token = get_token(st)) == NULL)
    return 0;

  if (token[0] != 'P') {
    free(token);
    return 0;
  }

  switch ((int)token[1]) {
  case '1':
    /* ascii bitmap */
    p->type = _BITMAP_MSBFirst;
    p->depth = p->bits_per_pixel = 1;
    p->ncolors = 2;
    p->format_detail = (char *)"PBM(ASCII)";
    data->encode = _ASCII;
    break;
  case '2':
    /* ascii graymap */
    p->type = _GRAY;
    p->depth = p->bits_per_pixel = 8;
    p->format_detail = (char *)"PGM(ASCII)";
    data->encode = _ASCII;
    break;
  case '3':
    /* ascii pixmap */
    p->type = _RGB24;
    p->depth = p->bits_per_pixel = 24;
    p->format_detail = (char *)"PPM(ASCII)";
    data->encode = _ASCII;
    break;
  case '4':
    /* raw bitmap */
    p->type = _BITMAP_MSBFirst;
    p->depth = p->bits_per_pixel = 1;
    p->ncolors = 2;
    p->format_detail = (char *)"PBM(RAW)";
    data->encode = _RAW;
    break;
  case '5':
    /* raw graymap */
    p->type = _GRAY;
    p->depth = p->bits_per_pixel = 8;
    p->format_detail = (char *)"PGM(RAW)";
    data->encode = _RAW;
    break;
  case '6':
    /* raw pixmap */
    p->type = _RGB24;
    p->depth = p->bits_per_pixel = 24;
    p->format_detail = (char *)"PPM(RAW)";
    data->encode = _RAW;
    break;
  case '7':
    /* xvpic */
    p->type = _INDEX;
    p->depth = p->bits_per_pixel = 8;
    p->format_detail = (char *)"xvpic";
    data->encode = _RAW;
    free(token);
    if ((token = get_token(st)) == NULL)
      return 0;
    {
      int id = atoi(token);
      if (id != 332) {
	show_message("Seems to be xvpic. But id is not 332 but %d.\n", id);
	return 0;
      }
    }
    break;
  default:
    free(token);
    return 0;
  }
  free(token);

  if ((token = get_token(st)) == NULL)
    return 0;
  image_width(p) = atoi(token);
  free(token);
  if (!image_width(p))
    return 0;

  if ((token = get_token(st)) == NULL)
    return 0;
  image_height(p) = atoi(token);
  free(token);
  if (!image_height(p))
    return 0;

  if (p->type != _BITMAP_MSBFirst) {
    if ((token = get_token(st)) == NULL)
      return 0;
    data->maxval = atoi(token);
    free(token);
  } else
    data->maxval = 1;

  switch (p->type) {
  case _BITMAP_MSBFirst:
    image_bpl(p) = (image_width(p) + 7) >> 3;
    break;
  case _GRAY:
    p->ncolors = data->maxval + 1;
    image_bpl(p) = image_width(p);
    break;
  case _INDEX:
    p->ncolors = data->maxval + 1;
    for (i = 0; i < p->ncolors; i++) {
      p->colormap[i][0] = i & 0xe0;
      p->colormap[i][1] = (i & 0x1c) << 3;
      p->colormap[i][2] = (i & 3) << 6;
    }
    image_bpl(p) = image_width(p);
    break;
  case _RGB24:
    p->ncolors = 1 << 24;
    image_bpl(p) = image_width(p) * 3;
    break;
  default:
    show_message_fnc("unexpected image type %s\n", image_type_to_string(p->type));
    return 0;
  }

  if (image_width(p) <= 0 || image_height(p) <= 0 || p->ncolors <= 0) {
    debug_message("This file seems to be PNM. But invalid parameters %d %d %d.\n", image_width(p), image_height(p), p->ncolors);
    return 0;
  }

  debug_message_fnc("max %d ncolors %d (%d, %d) %s\n", data->maxval, p->ncolors, image_width(p), image_height(p), image_type_to_string(p->type));

  return 1;
}

/* methods */

DEFINE_LOADER_PLUGIN_IDENTIFY(p, st, vw, c, priv)
{
  PNMdata pnmdata;

  if (!pnm_read_header(st, p, &pnmdata)) {
    get_token(NULL);
    return LOAD_NOT;
  }

  get_token(NULL);
  return LOAD_OK;
}

DEFINE_LOADER_PLUGIN_LOAD(p, st, vw, c, priv)
{
  PNMdata pnmdata;
  unsigned int i, j, x, y, image_size;
  unsigned char b, *d;
  char *token;

  debug_message("pnm: %s() called\n", __FUNCTION__);

  if (!pnm_read_header(st, p, &pnmdata)) {
    debug_message("Weird. Identified but pnm_read_header() failed.\n");
    goto error;
  }

  image_size = image_bpl(p) * image_height(p);
  debug_message("pnm: %s: allocate memory %d bytes for image\n", __FUNCTION__, image_size);
  if ((d = memory_alloc(image_image(p), image_size)) == NULL)
    goto error;

  switch (pnmdata.encode) {
  case _RAW:
    stream_read(st, d, image_size);
    break;
  case _ASCII:
    if (p->type == _BITMAP_MSBFirst) {
      for (y = 0; y < image_height(p); y++) {
	b = 0;
	for (x = 0; x < image_width(p); x++) {
	  if ((token = get_token(st)) == NULL)
	    goto error;
	  j = atoi(token);
	  free(token);
	  if (j == 1)
	    b |= 1;
	  else if (j != 0) {
	    show_message("pnm: %s: Invalid PBM file.\n", __FUNCTION__);
	    goto error;
	  }
	  if (x % 8 == 7) {
	    *d++ = b;
	    b = 0;
	  } else
	    b <<= 1;
	}
	if (x % 8 != 0) {
	  for (; x % 8 != 7; x++)
	    b <<= 1;
	  *d++ = b;
	}
      }
    } else {
      for (i = 0; i < image_size; i++) {
	if ((token = get_token(st)) == NULL)
	  goto error;
	j = atoi(token);
	free(token);
	if (j > p->ncolors) {
	  show_message("pnm: %s: Invalid PNM file.\n", __FUNCTION__);
	  goto error;
	}
	*d++ = j * 255 / pnmdata.maxval;
      }
    }
  }

  get_token(NULL);
  return LOAD_OK;

 error:
  get_token(NULL);
  return LOAD_ERROR;
}
