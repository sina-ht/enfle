/*
 * ungif.c -- gif loader plugin, which exploits libungif.
 * (C)Copyright 1998, 99, 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Jan  6 01:28:06 2001.
 * $Id: ungif.c,v 1.10 2001/04/28 12:56:10 sian Exp $
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
#include <gif_lib.h>

#include "common.h"

#include "enfle/loader-plugin.h"

DECLARE_LOADER_PLUGIN_METHODS;

static LoaderPlugin plugin = {
  type: ENFLE_PLUGIN_LOADER,
  name: "UNGIF",
  description: "UNGIF Loader plugin version 0.3",
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

static int
ungif_input_func(GifFileType *GifFile, GifByteType *p, int s)
{
  Stream *st = (Stream *)GifFile->UserData;

  return stream_read(st, p, s);
}

static LoaderStatus
load_image(Image *p, Stream *st)
{
  int i, j, size, sheight, row, col, extcode;
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

	/* This GIF should be treated by player plugin. */
	return LOAD_NOT;
      }
      image_loaded++;

      if (DGifGetImageDesc(GifFile) == GIF_ERROR)
	goto error;
      p->top = row = GifFile->Image.Top;
      p->left = col = GifFile->Image.Left;
      p->width = GifFile->Image.Width;
      p->height = GifFile->Image.Height;

      //debug_message("IMAGE_DESC_RECORD_TYPE: (%d, %d) (%d, %d)\n", p->left, p->top, p->width, p->height);

      if (GifFile->Image.Interlace) {
	for (i = 0; i < 4; i++)
	  for (j = row + ioffset[i]; j < row + p->height; j += ijumps[i])
	    if (DGifGetLine(GifFile, &ScreenBuffer[j][col], p->width) == GIF_ERROR)
	      goto error;
      } else {
	for (i = 0; i < p->height; i++)
	  if (DGifGetLine(GifFile, &ScreenBuffer[row++][col], p->width) == GIF_ERROR)
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
	  show_message("No enough memory for comment. Try to continue.\n");
	} else {
	  memcpy(p->comment, &Extension[1], Extension[0]);
	  p->comment[Extension[0]] = '\0';
	  debug_message("%s\n", p->comment);
	}
	break;
      case GRAPHICS_EXT_FUNC_CODE:
	if (Extension[1] & 1) {
	  p->alpha_enabled = 1;
	  p->mask = NULL; /* to be auto-generated */
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
	    unsigned char *tmp;

	    if ((tmp = realloc(p->comment, strlen(p->comment) + Extension[0] + 1)) == NULL) {
	      show_message("No enough memory for comment(append). Truncated.\n");
	    } else {
	      memcpy(tmp + strlen(tmp), &Extension[1], Extension[0]);
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
  p->bytes_per_line = p->width;
  p->next = NULL;

  if ((d = memory_alloc(p->image, p->bytes_per_line * p->height)) == NULL)
    goto error_after_closed;

  for (i = 0; i < p->height; i++)
    memcpy(&d[i * p->width], &ScreenBuffer[p->top + i][p->left], p->width * sizeof(GifPixelType));

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

DEFINE_LOADER_PLUGIN_IDENTIFY(p, st, priv)
{
  char buf[3];

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

DEFINE_LOADER_PLUGIN_LOAD(p, st, priv)
{
  debug_message("ungif loader: load() called\n");

#ifdef IDENTIFY_BEFORE_LOAD
  {
    LoaderStatus status;

    if ((status = identify(p, st, priv)) != LOAD_OK)
      return status;
    stream_rewind(st);
  }
#endif

  return load_image(p, st);
}
