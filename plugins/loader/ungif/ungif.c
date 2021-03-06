/*
 * ungif.c -- gif loader plugin, which exploits libungif.
 * (C)Copyright 1998, 99, 2000, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Wed Mar  1 00:23:16 2006.
 *
 * NOTES:
 *  This file does NOT include LZW code.
 *  This plugin is incomplete, but works.
 *  Requires libungif version 3.1.0 or later(4.1.0 recommended).
 *
 *             The Graphics Interchange Format(c) is
 *       the Copyright property of CompuServe Incorporated.
 * GIF(sm) is a Service Mark property of CompuServe Incorporated.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include <stdio.h>
#include <stdlib.h>
#include <gif_lib.h>

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "enfle/loader-plugin.h"

DECLARE_LOADER_PLUGIN_METHODS;

//static const unsigned int types = IMAGE_INDEX;

static LoaderPlugin plugin = {
  .type = ENFLE_PLUGIN_LOADER,
  .name = "UNGIF",
  .description = "UNGIF Loader plugin version 0.3 with libungif",
  .author = "Hiroshi Takekawa",
  .image_private = NULL,

  .identify = identify,
  .load = load
};

ENFLE_PLUGIN_ENTRY(loader_ungif)
{
  LoaderPlugin *lp;

  if ((lp = (LoaderPlugin *)calloc(1, sizeof(LoaderPlugin))) == NULL)
    return NULL;
  memcpy(lp, &plugin, sizeof(LoaderPlugin));

  return (void *)lp;
}

ENFLE_PLUGIN_EXIT(loader_ungif, p)
{
  free(p);
}

/* for internal use */

static int
ungif_input_func(GifFileType *GifFile, GifByteType *p, int s)
{
  Stream *st = (Stream *)GifFile->UserData;

  return stream_read(st, p, s);
}

static LoaderStatus
load_image(Image *p, Stream *st)
{
  int size, col, extcode;
  unsigned int i, j, row, sheight;
  static int ioffset[] = { 0, 4, 2, 1 };
  static int ijumps[] = { 8, 8, 4, 2 };
  GifRecordType RecordType;
  GifByteType *Extension;
  GifRowType *ScreenBuffer;
  GifFileType *GifFile;
  ColorMapObject *ColorMap;
  int image_loaded = 0;
  unsigned char *d;

  if ((GifFile = DGifOpen(st, ungif_input_func)) == NULL) {
#ifdef DEBUG
    PrintGifError();
#endif
    return LOAD_ERROR;
  }

  sheight = GifFile->SHeight;

  if ((ScreenBuffer = (GifRowType *)
       calloc(sheight, sizeof(GifRowType *))) == NULL) {
    if (DGifCloseFile(GifFile) == GIF_ERROR)
      PrintGifError();
    return LOAD_ERROR;
  }

  size = GifFile->SWidth * sizeof(GifPixelType);
  if ((ScreenBuffer[0] = (GifRowType)calloc(sheight, size)) == NULL) {
    free(ScreenBuffer);
    if (DGifCloseFile(GifFile) == GIF_ERROR)
      PrintGifError();
    return LOAD_ERROR;
  }
  for (i = 1; i < sheight; i++)
    ScreenBuffer[i] = ScreenBuffer[0] + i * size;
  memset(ScreenBuffer[0], GifFile->SBackGroundColor, sheight * size);
  p->background_color.index = GifFile->SBackGroundColor;

  do {
    if (DGifGetRecordType(GifFile, &RecordType) == GIF_ERROR) {
      if (!image_loaded)
	goto error;
      /* Show error message, but try to display. */
      PrintGifError();
      break;
    }

    switch (RecordType) {
    case IMAGE_DESC_RECORD_TYPE:
      if (image_loaded) {
	debug_message("GIF contains Multiple images. (maybe animated?)\n");
	DGifCloseFile(GifFile);
	free(ScreenBuffer[0]);
	free(ScreenBuffer);

	if (p->comment) {
	  free(p->comment);
	  p->comment = NULL;
	}

	/* This GIF should be treated by the UNGIF player plugin. */
	return LOAD_NOT;
      }
      image_loaded++;

      if (DGifGetImageDesc(GifFile) == GIF_ERROR)
	goto error;
      image_top(p) = row = GifFile->Image.Top;
      image_left(p) = col = GifFile->Image.Left;
      image_width(p) = GifFile->Image.Width;
      image_height(p) = GifFile->Image.Height;

      //debug_message("IMAGE_DESC_RECORD_TYPE: (%d, %d) (%d, %d)\n", p->left, p->top, image_width(p), image_height(p));

      if (GifFile->Image.Interlace) {
	for (i = 0; i < 4; i++)
	  for (j = row + ioffset[i]; j < row + image_height(p); j += ijumps[i])
	    if (DGifGetLine(GifFile, &ScreenBuffer[j][col], image_width(p)) == GIF_ERROR)
	      goto error;
      } else {
	for (i = 0; i < image_height(p); i++)
	  if (DGifGetLine(GifFile, &ScreenBuffer[row++][col], image_width(p)) == GIF_ERROR)
	    goto error;
      }
      break;
    case EXTENSION_RECORD_TYPE:
      if (DGifGetExtension(GifFile, &extcode, &Extension) == GIF_ERROR)
	goto error;

      switch (extcode) {
      case COMMENT_EXT_FUNC_CODE:
	debug_message("comment: ");
	if ((p->comment = malloc(Extension[0] + 1)) == NULL) {
	  debug_message("(abandoned)\n");
	  warning("No enough memory for comment. Try to continue.\n");
	} else {
	  memcpy(p->comment, &Extension[1], Extension[0]);
	  p->comment[Extension[0]] = '\0';
	  debug_message("%s\n", p->comment);
	}
	break;
      case GRAPHICS_EXT_FUNC_CODE:
	if (Extension[1] & 1) {
	  p->alpha_enabled = 1;
	  p->transparent_color.index = Extension[4];
	}
	/* ignore image disposal and delay in this loader plugin */
	break;
      default:
	break;
      }

      for (;;) {
	if (DGifGetExtensionNext(GifFile, &Extension) == GIF_ERROR)
	  goto error;

	if (Extension == NULL)
	  break;

	switch (extcode) {
	case COMMENT_EXT_FUNC_CODE:
	  {
	    char *tmp;

	    if ((tmp = realloc(p->comment, strlen((const char *)p->comment) + Extension[0] + 1)) == NULL) {
	      warning("No enough memory for comment(append). Truncated.\n");
	    } else {
	      memcpy(tmp + strlen((const char *)tmp), &Extension[1], Extension[0]);
	      p->comment = tmp;
	    }
	  }
	  break;
	default:
	  break;
	}
      }
      break;
    default:
      break;
    }
  } while (RecordType != TERMINATE_RECORD_TYPE);

  ColorMap = GifFile->Image.ColorMap ? GifFile->Image.ColorMap : GifFile->SColorMap;
  p->ncolors = ColorMap->ColorCount;

  for (i = 0; i < p->ncolors; i++) {
    p->colormap[i][0] = ColorMap->Colors[i].Red;
    p->colormap[i][1] = ColorMap->Colors[i].Green;
    p->colormap[i][2] = ColorMap->Colors[i].Blue;
  }

  if (DGifCloseFile(GifFile) == GIF_ERROR)
    goto error;

  p->type = _INDEX;
  p->depth = 8;
  p->bits_per_pixel = 8;
  image_bpl(p) = image_width(p);

  if ((d = memory_alloc(image_image(p), image_bpl(p) * image_height(p))) == NULL)
    goto error_after_closed;

  for (i = 0; i < image_height(p); i++)
    memcpy(&d[i * image_width(p)], &ScreenBuffer[image_top(p) + i][image_left(p)], image_width(p) * sizeof(GifPixelType));

  free(ScreenBuffer[0]);
  free(ScreenBuffer);

  return LOAD_OK;

 error:
  PrintGifError();
  DGifCloseFile(GifFile);
 error_after_closed:
  free(ScreenBuffer[0]);
  free(ScreenBuffer);

  return LOAD_ERROR;
}

/* methods */

DEFINE_LOADER_PLUGIN_IDENTIFY(p __attribute__((unused)), st, vw __attribute__((unused)), c __attribute__((unused)), priv __attribute__((unused)))
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

  warning("GIF detected, but version is neither 87a nor 89a.\n");

  return LOAD_OK;
}

DEFINE_LOADER_PLUGIN_LOAD(p, st, vw
#if !defined(IDENTIFY_BEFORE_LOAD)
__attribute__((unused))
#endif
, c
#if !defined(IDENTIFY_BEFORE_LOAD)
__attribute__((unused))
#endif
, priv
#if !defined(IDENTIFY_BEFORE_LOAD)
__attribute__((unused))
#endif
)
{
  debug_message("ungif loader: load() called\n");

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
