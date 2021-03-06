/*
 * pcx.c -- pcx loader plugin
 * (C)Copyright 2000, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Wed Mar  1 00:23:31 2006.
 *
 * Note: This plugin is not complete.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include <stdio.h>
#include <stdlib.h>

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "enfle/loader-plugin.h"
#define UTILS_NEED_GET_LITTLE_UINT16
#include "enfle/utils.h"

//static const unsigned int types = (IMAGE_RGB24 | IMAGE_RGBA32 | IMAGE_GRAY | IMAGE_INDEX);

DECLARE_LOADER_PLUGIN_METHODS;

static LoaderPlugin plugin = {
  .type = ENFLE_PLUGIN_LOADER,
  .name = "PCX",
  .description = "PCX Loader plugin version 0.1",
  .author = "Hiroshi Takekawa",
  .image_private = NULL,

  .identify = identify,
  .load = load
};

ENFLE_PLUGIN_ENTRY(loader_pcx)
{
  LoaderPlugin *lp;

  if ((lp = (LoaderPlugin *)calloc(1, sizeof(LoaderPlugin))) == NULL)
    return NULL;
  memcpy(lp, &plugin, sizeof(LoaderPlugin));

  return (void *)lp;
}

ENFLE_PLUGIN_EXIT(loader_pcx, p)
{
  free(p);
}

/* methods */

DEFINE_LOADER_PLUGIN_IDENTIFY(p, st, vw __attribute__((unused)), c __attribute__((unused)), priv __attribute__((unused)))
{
  unsigned char buf[128];
  int xmax, ymax, xmin, ymin;

  /* Read in the signature bytes */
  if (stream_read(st, buf, 128) != 128)
    return LOAD_NOT;

  /* Manufacturer: Constant Flag, 10 = ZSoft .pcx */
  if (buf[0] != 10)
    return LOAD_NOT;

  /* version */
  switch (buf[1]) {
  case 0:
    p->format_detail = (char *)"Version 2.5 of PC Paintbrush";
    break;
  case 2:
    p->format_detail = (char *)"Version 2.8 w/palette information";
    break;
  case 3:
    p->format_detail = (char *)"Version 2.8 w/o palette information";
    break;
  case 4:
    p->format_detail = (char *)"PC Paintbrush for Windows(Plus for Windows uses Ver 5)";
    break;
  case 5:
    p->format_detail = (char *)
      "Version 3.0 and > of PC Paintbrush and PC Paintbrush +,"
      "includes Publisher's Paintbrush . Includes 24-bit .PCX files";
    break;
  default:
    return LOAD_NOT;
  }

  /* encoding */
  if (buf[2] != 1)
    return LOAD_NOT;

  /* reserved */
  if (buf[64])
    warning("reserved byte should be 0\n");

  /* bpp */
  switch (buf[3]) {
  case 1:
  case 2:
  case 4:
  case 8:
    break;
  default:
    return LOAD_NOT;
  }

  xmin = utils_get_little_uint16(buf + 4);
  ymin = utils_get_little_uint16(buf + 6);
  xmax = utils_get_little_uint16(buf + 8);
  ymax = utils_get_little_uint16(buf + 10);

  if (xmin > xmax || ymin > ymax)
    return LOAD_NOT;

  /* nPlanes */
  switch (buf[65]) {
  case 1:
  case 3:
  case 4:
    break;
  default:
    return LOAD_NOT;
  }

  return LOAD_OK;
}

DEFINE_LOADER_PLUGIN_LOAD(p, st, vw
#if !defined(IDENTIFY_BEFORE_LOAD)
__attribute__((unused))
#endif
, config
#if !defined(IDENTIFY_BEFORE_LOAD)
__attribute__((unused))
#endif
, priv
#if !defined(IDENTIFY_BEFORE_LOAD)
__attribute__((unused))
#endif
)
{
  int j, l;
  unsigned int i, k, ppl;
  int nplanes;
  int xmax, ymax, xmin, ymin;
  unsigned char buf[769];
  unsigned char c, *d, *dd;

  debug_message("pcx loader: load() called\n");

#ifdef IDENTIFY_BEFORE_LOAD
  {
    LoaderStatus status;

    if ((status = identify(p, st, vw, config, priv)) != LOAD_OK)
      return status;
    stream_rewind(st);
  }
#endif

  if (stream_read(st, buf, 128) != 128)
    return LOAD_ERROR;

  xmin = utils_get_little_uint16(buf + 4);
  ymin = utils_get_little_uint16(buf + 6);
  xmax = utils_get_little_uint16(buf + 8);
  ymax = utils_get_little_uint16(buf + 10);

  if (xmin > xmax || ymin > ymax)
    return LOAD_ERROR;

  image_top(p) = 0;
  image_left(p) = 0;
  image_width(p)  = xmax - xmin + 1;
  image_height(p) = ymax - ymin + 1;

  /* buf[3] is bpp per plane */
  debug_message("bpp per plane: %d\n", buf[3]);
  switch (buf[3]) {
  case 1:
  case 2:
  case 4:
  case 8:
    nplanes = buf[65];
    debug_message("nplanes: %d\n", nplanes);
    switch (nplanes) {
    case 1:
      /* mono */
      switch (buf[68]) {
      case 1:
	p->type = _INDEX;
	break;
      case 2:
	p->type = _GRAY;
	break;
      default:
	warning("Palette info = %d\n", buf[68]);
	break;
      }
      p->depth = 8;
      p->bits_per_pixel = 8;
      image_bpl(p) = image_width(p);
      ppl = utils_get_little_uint16(buf + 66);
      break;
    case 3:
      /* RGB */
      p->type = _RGB24;
      p->depth = 24;
      p->bits_per_pixel = 24;
      image_bpl(p) = image_width(p) * 3;
      ppl = utils_get_little_uint16(buf + 66) / 3;
      break;
    case 4:
      /* RGBI */
      p->type = _RGBA32;
      p->depth = 24;
      p->bits_per_pixel = 32;
      image_bpl(p) = image_width(p) * 4;
      ppl = utils_get_little_uint16(buf + 66) / 4;
      break;
    default:
      return LOAD_ERROR;
    }
    break;
  default:
    return LOAD_ERROR;
  }

  /* buf[68] is palette info. */
  if (buf[68]) {
    debug_message("Palette info: %d\n", buf[68]);
    if (buf[1] == 5) {
      if (!stream_seek(st, -769, _END))
	return LOAD_ERROR;
      if (stream_read(st, buf, 769) != 769)
	return LOAD_ERROR;
      if (!stream_seek(st, 128, _SET))
	return LOAD_ERROR;
      if (buf[0] == 12) {
	debug_message("256 color palette.\n");
	p->ncolors = 256;
	for (i = 0; i < 256; i++) {
	  p->colormap[i][0] = buf[i * 3 + 1];
	  p->colormap[i][1] = buf[i * 3 + 2];
	  p->colormap[i][2] = buf[i * 3 + 3];
	}
      }
    } else {
      p->ncolors = 16;
      debug_message("16 color palette.\n");
      for (i = 0; i < 16; i++) {
	p->colormap[i][0] = buf[i * 3 + 16];
	p->colormap[i][1] = buf[i * 3 + 17];
	p->colormap[i][2] = buf[i * 3 + 18];
      }
    }
  }

  /* allocate memory for returned image */
  if (memory_alloc(image_image(p), image_bpl(p) * image_height(p)) == NULL)
    return LOAD_ERROR;

  debug_message("ppl: %d\n", ppl);
  /* read image */
  d = dd = memory_ptr(image_image(p));
  for (i = 0; i < image_height(p); i++) {
    for (j = 0; j < nplanes; j++) {
      d = dd + j;
      for (k = 0; k < ppl;) {
	if (stream_read(st, &c, 1) != 1)
	  return LOAD_ERROR;
	if ((c & 0xc0) == 0xc0) {
	  l = c & 0x3f;
	  if (stream_read(st, &c, 1) != 1)
	    return LOAD_ERROR;
	} else {
	  l = 1;
	}
	do {
	  if (k < image_width(p)) {
	    *d = c;
	    d += nplanes;
	  }
	  k++;
	} while (--l > 0);
      }
    }
    dd += image_bpl(p);
  }

  return LOAD_OK;
}
