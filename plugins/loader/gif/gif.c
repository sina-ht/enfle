/*
 * gif.c -- gif loader plugin
 * (C)Copyright 1998, 99, 2000, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Mar  6 12:19:45 2004.
 * $Id: gif.c,v 1.3 2004/03/06 03:43:36 sian Exp $
 *
 * NOTE: This file does not include LZW code
 *
 *             The Graphics Interchange Format(c) is
 *       the Copyright property of CompuServe Incorporated.
 * GIF(sm) is a Service Mark property of CompuServe Incorporated.
 *
 */

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "enfle/loader-plugin.h"

#include "giflib.h"

DECLARE_LOADER_PLUGIN_METHODS;

static LoaderPlugin plugin = {
  .type = ENFLE_PLUGIN_LOADER,
  .name = "GIF",
  .description = "GIF Loader plugin version 0.1",
  .author = "Hiroshi Takekawa",
  .image_private = NULL,

  .identify = identify,
  .load = load
};

ENFLE_PLUGIN_ENTRY(loader_gif)
{
  LoaderPlugin *lp;

  if ((lp = (LoaderPlugin *)calloc(1, sizeof(LoaderPlugin))) == NULL)
    return NULL;
  memcpy(lp, &plugin, sizeof(LoaderPlugin));

  return (void *)lp;
}

ENFLE_PLUGIN_EXIT(loader_gif, p)
{
  free(p);
}

/* converts giflib format image into enfle format image */
static int
gif_convert(Image *p, GIF_info *g_info, GIF_image *image)
{
  GIF_ct *ct;
  int i, if_animated;
  //int transparent_disposal;

#if 0
  if (image->next != NULL) {
    if ((p->next = image_create()) == NULL) {
      image_destroy(p);
      return 0;
    }
    if (!gif_convert(p->next, g_info, image->next)) {
      image_destroy(p);
      return 0;
    }
  } else
    p->next = NULL;
#endif

  //swidth = g_info->sd->width;
  //sheight = g_info->sd->height;

  image_left(p) = image->id->left;
  image_top(p) = image->id->top;
  image_width(p) = image->id->width;
  image_height(p) = image->id->height;

#if 0
  if (image_width(p) > swidth || image_height(p) > sheight) {
    show_message("screen (%dx%d) but image (%dx%d)\n",
		 swidth, sheight, p->width, p->height);
    swidth = image_width(p);
    sheight = image_height(p);
  }
#endif

  p->ncolors = image->id->lct_follows ? 1 << image->id->depth : 1 << g_info->sd->depth;
  p->type = _INDEX;
  //p->delay = image->gc->delay ? image->gc->delay : 1;

  if_animated = g_info->npics > 1 ? 1 : 0;

  debug_message("GIF: %d pics animation %d\n", g_info->npics, if_animated);

#if 0
  if (image->gc->transparent) {
    p->transparent_disposal = if_animated ? _TRANSPARENT : transparent_disposal;
    p->transparent.index = image->gc->transparent_index;
  } else
    p->transparent_disposal = _DONOTHING;
  p->image_disposal = image->gc->disposal;
  p->background.index = g_info->sd->back;
#endif

  if (image->id->lct_follows)
    ct = image->id->lct;
  else if (g_info->sd->gct_follows)
    ct = g_info->sd->gct;
  else {
    fprintf(stderr, "Null color table..\n");
    ct = NULL;
  }

  for (i = 0; i < (int)p->ncolors; i++) {
    p->colormap[i][0] = ct->cell[i]->value[0];
    p->colormap[i][1] = ct->cell[i]->value[1];
    p->colormap[i][2] = ct->cell[i]->value[2];
  }

  image_bpl(p) = image_width(p);
  if (!image_image(p))
    image_image(p) = memory_create();
  if (memory_alloc(image_image(p), image_bpl(p) * image_height(p)) == NULL)
    return 0;
  memcpy(memory_ptr(image_image(p)), image->data, image_bpl(p) * image_height(p));

  return 1;
}

static int
load_image(Image *p, Stream *st)
{
  GIF_info *g_info;
  int c;

  g_info = GIFReadSignature(st, &c);
  if (c != 0) {
    if (c == NOENOUGHMEM)
      err_message("gif loader: No enough memory for g_info.\n");
    return (c == NOTGIFFILE) ? LOAD_NOT : LOAD_ERROR;
  }

  if (g_info->revision != 87 && g_info->revision != 89) {
    err_message("gif loader: GIF87a or GIF89a only...sorry\n");
    return LOAD_ERROR;
  }

  if (GIFReadScreenDescriptor(st, g_info) != SUCCESS) {
    err_message("No enough memory for sd.\n");
    return LOAD_ERROR;
  }

  if (g_info->sd->aspect_ratio) {
    double ratio = (double)((g_info->sd->aspect_ratio + 15) / 64);

    if ((int)ratio != 1)
      warning("Aspect ratio = %f ... ignored\n", ratio);
  }

  do {
    c = GIFParseNextBlock(st, g_info);
    if (g_info->npics > 1) {
      GIFDestroyData(g_info);
      return LOAD_NOT;
    }
  } while (c == PARSE_OK);

  if (g_info->comment != NULL)
    p->comment = (unsigned char *)strdup(g_info->comment);

  if (c == PARSE_ERROR)
    err_message("gif loader: Parse error: %s at 0x%lX.\n", g_info->err, stream_tell(st));

  gif_convert(p, g_info, g_info->top);

  /* TODO: Use Screen g_info->sd->width, g_info->sd->height */

  GIFDestroyData(g_info);

  return LOAD_OK;
}

DEFINE_LOADER_PLUGIN_IDENTIFY(p, st, vw, c, priv)
{
  unsigned char buf[3];

  if (stream_read(st, buf, 3) != 3)
    return LOAD_NOT;

  if (memcmp(buf, "GIF", 3) != 0)
    return LOAD_NOT;

  if (stream_read(st, buf, 3) != 3)
    return LOAD_NOT;

  if (memcmp(buf, "87a", 3) == 0)
    return LOAD_OK;
  if (memcmp(buf, "89a", 3) == 0)
    return LOAD_OK;

  show_message("GIF detected, but version is neither 87a nor 89a.\n");

  return LOAD_OK;
}

DEFINE_LOADER_PLUGIN_LOAD(p, st, vw, c, priv)
{
  debug_message("gif loader: load() called\n");

#ifdef IDENTIFY_BEFORE_LOAD
  {
    LoaderStatus status;

    if ((status = identify(p, st, vw, c, priv)) != LOAD_OK)
      return status;
    stream_rewind(st);
  }
#endif

  return load_image(p, st);
}
