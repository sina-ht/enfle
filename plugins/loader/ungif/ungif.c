/*
 * ungif.c -- gif loader plugin, which exploits libungif.
 * (C)Copyright 1998, 99, 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Sep 30 04:31:36 2000.
 * $Id: ungif.c,v 1.1 2000/09/30 17:36:36 sian Exp $
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

#include "loader-plugin.h"

static LoaderStatus identify(Image *, Stream *);
static LoaderStatus load(Image *, Stream *);

static LoaderPlugin plugin = {
  type: ENFLE_PLUGIN_LOADER,
  name: "UNGIF",
  description: "UNGIF Loader plugin version 0.1",
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

static int
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

  if ((GifFile = DGifOpen(st, ungif_input_func)) == NULL) {
#if 1
    PrintGifError();
#endif
    return 0;
  }

  sheight = GifFile->SHeight;

  if ((ScreenBuffer = (GifRowType *)
       calloc(sheight, sizeof(GifRowType *))) == NULL) {
    if (DGifCloseFile(GifFile) == GIF_ERROR)
      PrintGifError();
    return 0;
  }

  size = GifFile->SWidth * sizeof(GifPixelType);
  if ((ScreenBuffer[0] = (GifRowType)calloc(sheight, size)) == NULL) {
    if (DGifCloseFile(GifFile) == GIF_ERROR)
      PrintGifError();
    return 0;
  }
  for (i = 1; i < sheight; i++)
    ScreenBuffer[i] = ScreenBuffer[0] + i * size;
  memset(ScreenBuffer[0], sheight * size, GifFile->SBackGroundColor);

  do {

    debug_message("GetRecordType ");

    if (DGifGetRecordType(GifFile, &RecordType) == GIF_ERROR) {
      PrintGifError();
#if 0
      DGifCloseFile(GifFile);
      free(ScreenBuffer[0]);
      free(ScreenBuffer);
      return 0;
#endif
    }

    debug_message("done.\n");

    switch (RecordType) {
    case IMAGE_DESC_RECORD_TYPE:
      if (DGifGetImageDesc(GifFile) == GIF_ERROR)
	goto error;
      p->top = row = GifFile->Image.Top;
      p->left = col = GifFile->Image.Left;
      p->width = GifFile->Image.Width;
      p->height = GifFile->Image.Height;

      debug_message("IMAGE_DESC_RECORD_TYPE: (%d, %d) (%d, %d)\n",
		    p->left, p->top, p->width, p->height);

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
      while (Extension != NULL) {
	if (DGifGetExtensionNext(GifFile, &Extension) == GIF_ERROR)
	  goto error;
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
  p->image_size = p->bytes_per_line * p->height;
  if ((p->image = calloc(1, p->image_size)) == NULL)
    goto error_after_closed;

  for (i = 0; i < p->height; i++)
    for (j = 0; j < p->width; j++)
      p->image[i * p->width + j] = ScreenBuffer[p->top + i][p->left + j];

  free(ScreenBuffer[0]);
  free(ScreenBuffer);

  return 1;

 error:
  PrintGifError();
  DGifCloseFile(GifFile);
 error_after_closed:
  free(ScreenBuffer[0]);
  free(ScreenBuffer);

  return 0;
}

/* methods */

static LoaderStatus
identify(Image *p, Stream *st)
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

  return LOAD_NOT;
}

static LoaderStatus
load(Image *p, Stream *st)
{
  debug_message("ungif loader: load() called\n");

#ifdef IDENTIFY_BEFORE_LOAD
  {
    LoaderStatus status;

    if ((status = identify(p, st)) != LOAD_OK)
      return status;
    stream_rewind(st);
  }
#endif

  if (!load_image(p, st))
    return LOAD_ERROR;

  return LOAD_OK;
}
