/*
 * ungif.c -- gif player plugin, which exploits libungif.
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Oct 10 17:52:30 2000.
 * $Id: ungif.c,v 1.3 2000/10/10 11:49:18 sian Exp $
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

#include "player-plugin.h"

typedef struct _ungif_info {
  GifFileType *gf;
  GifRowType *buffer;
} UNGIF_info;

static PlayerStatus identify(Movie *, Stream *);
static PlayerStatus load(Movie *, Stream *);

static PlayerStatus pause_movie(Movie *);

static PlayerPlugin plugin = {
  type: ENFLE_PLUGIN_PLAYER,
  name: "UNGIF",
  description: "UNGIF Player plugin version 0.1",
  author: "Hiroshi Takekawa",

  identify: identify,
  load: load
};

void *
plugin_entry(void)
{
  PlayerPlugin *pp;

  if ((pp = (PlayerPlugin *)calloc(1, sizeof(PlayerPlugin))) == NULL)
    return NULL;
  memcpy(pp, &plugin, sizeof(PlayerPlugin));

  return (void *)pp;
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

static PlayerStatus
load_movie(Movie *m, Stream *st)
{
  int i, size;
  UNGIF_info *info;

  if ((info = calloc(1, sizeof(UNGIF_info))) == NULL) {
    show_message("UNGIF: play_movie: No enough memory.\n");
    return PLAY_ERROR;
  }

  if ((info->gf = DGifOpen(st, ungif_input_func)) == NULL) {
    PrintGifError();
    free(info);
    return PLAY_ERROR;
  }

  m->width = info->gf->SWidth;
  m->height = info->gf->SHeight;

  if ((info->buffer = (GifRowType *)calloc(m->height, sizeof(GifRowType *))) == NULL) {
    return PLAY_ERROR;
  }

  size = m->width * sizeof(GifPixelType);
  if ((info->buffer[0] = (GifRowType)calloc(m->height, size)) == NULL) {
    return PLAY_ERROR;
  }

  for (i = 1; i < m->height; i++)
    info->buffer[i] = info->buffer[0] + i * size;
  memset(info->buffer[0], info->gf->SBackGroundColor, m->height * size);

  m->initialize_screen(m, m->width, m->height);

  m->st = st;
  m->movie_private = (void *)info;
  m->status = _PLAY;
  m->nthframe = 0;

  return PLAY_OK;
}

static void *
get_screen(Movie *m)
{
  UNGIF_info *info;

  if (m->movie_private) {
    info = (UNGIF_info *)m->movie_private;
    return info->buffer[0];
  }

  return NULL;
}

static PlayerStatus
play(Movie *m)
{
  switch (m->status) {
  case _PLAY:
    return PLAY_OK;
  case _PAUSE:
    return pause_movie(m);
  case _STOP:
    m->status = _PLAY;
    return PLAY_OK;
  case _UNLOADED:
    return PLAY_ERROR;
  }

  return PLAY_ERROR;
}

static PlayerStatus
play_main(Movie *m)
{
  int i, j, size, row, col, extcode, image_loaded = 0;
  int transparent_index = 0, delay = 0, image_disposal = 0;
  static int ioffset[] = { 0, 4, 2, 1 };
  static int ijumps[] = { 8, 8, 4, 2 };
  UNGIF_info *info = (UNGIF_info *)m->movie_private;
  GifFileType *gf = info->gf;
  GifRecordType RecordType;
  GifByteType *Extension;
  ColorMapObject *ColorMap;
  Image *p;
  Transparent transparent_disposal = _DONOTHING;

  switch (m->status) {
  case _PLAY:
    break;
  case _PAUSE:
  case _STOP:
    return PLAY_OK;
  case _UNLOADED:
    return PLAY_ERROR;
  default:
    return PLAY_ERROR;
  }

  p = image_create();

  do {
    if (DGifGetRecordType(gf, &RecordType) == GIF_ERROR) {
      PrintGifError();
      break;
    }

    switch (RecordType) {
    case IMAGE_DESC_RECORD_TYPE:
      if (DGifGetImageDesc(gf) == GIF_ERROR)
	goto error;
      p->top = row = gf->Image.Top;
      p->left = col = gf->Image.Left;
      p->width = gf->Image.Width;
      p->height = gf->Image.Height;

      debug_message("IMAGE_DESC_RECORD_TYPE: (%d, %d) (%d, %d)\n", p->left, p->top, p->width, p->height);

      memset(info->buffer[0], gf->SBackGroundColor, m->height * size);
      if (gf->Image.Interlace) {
	for (i = 0; i < 4; i++)
	  for (j = row + ioffset[i]; j < row + p->height; j += ijumps[i])
	    if (DGifGetLine(gf, &info->buffer[j][col], p->width) == GIF_ERROR)
	      goto error;
      } else {
	for (i = 0; i < p->height; i++)
	  if (DGifGetLine(gf, &info->buffer[row++][col], p->width) == GIF_ERROR)
	    goto error;
      }

      p->type = _INDEX;
      p->bytes_per_line = p->width;
      p->image_size = p->bytes_per_line * p->height;
      p->background_color.index = gf->SBackGroundColor;
      if (m->nthframe == 0 && image_disposal == _RESTOREBACKGROUND) {
	/* guess... */
	p->transparent_disposal = _TRANSPARENT;
      } else {
	p->transparent_disposal = transparent_disposal;
      }
      p->transparent_color.index = transparent_index;
      p->image_disposal = image_disposal;

      if ((p->image = malloc(p->image_size)) == NULL)
	goto error;

      for (i = 0; i < p->height; i++)
	memcpy(&p->image[i * p->width], info->buffer[p->top + i], p->width);

      ColorMap = gf->Image.ColorMap ? gf->Image.ColorMap : gf->SColorMap;
      p->ncolors = ColorMap->ColorCount;

      for (i = 0; i < p->ncolors; i++) {
	p->colormap[i][0] = ColorMap->Colors[i].Red;
	p->colormap[i][1] = ColorMap->Colors[i].Green;
	p->colormap[i][2] = ColorMap->Colors[i].Blue;
      }

      m->nthframe++;
      image_loaded = 1;
      break;
    case EXTENSION_RECORD_TYPE:
      if (DGifGetExtension(gf, &extcode, &Extension) == GIF_ERROR)
	goto error;

      switch (extcode) {
      case COMMENT_EXT_FUNC_CODE:
	debug_message("comment: ");
	if ((p->comment = malloc(Extension[0] + 1)) == NULL) {
	  show_message("No enough memory for comment. Try to continue.\n");
	  debug_message("(abandoned)\n");
	} else {
	  memcpy(p->comment, &Extension[1], Extension[0]);
	  p->comment[Extension[0]] = '\0';
	  debug_message("%s\n", p->comment);
	}
	break;
      case GRAPHICS_EXT_FUNC_CODE:
	if (Extension[1] & 1) {
	  transparent_disposal = _TRANSPARENT;
	  transparent_index = Extension[4];
	} else {
	  transparent_disposal = _DONOTHING;
	  transparent_index = -1;
	}
	image_disposal = (Extension[1] & 0x1c) >> 2;
	delay = Extension[2] + (Extension[3] << 8);
	break;
      default:
	break;
      }

      for (;;) {
	if (DGifGetExtensionNext(gf, &Extension) == GIF_ERROR)
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
    case TERMINATE_RECORD_TYPE:
      image_destroy(p);
      m->status = _STOP;
      return PLAY_OK;
    default:
      break;
    }
  } while (!image_loaded);

  m->render_frame(m, p);
  if (delay)
    m->pause_usec(delay * 10000);

  image_destroy(p);

  return PLAY_OK;

 error:
  image_destroy(p);

  return PLAY_ERROR;
}

static PlayerStatus
pause_movie(Movie *m)
{
  switch (m->status) {
  case _PLAY:
    m->status = _PAUSE;
    return PLAY_OK;
  case _PAUSE:
    m->status = _PLAY;
    return PLAY_OK;
  case _STOP:
    return PLAY_OK;
  case _UNLOADED:
    return PLAY_ERROR;
  }

  return PLAY_ERROR;
}

static PlayerStatus
stop_movie(Movie *m)
{
  switch (m->status) {
  case _PLAY:
    m->status = _STOP;
    break;
  case _PAUSE:
  case _STOP:
    return PLAY_OK;
  case _UNLOADED:
    return PLAY_ERROR;
  default:
    return PLAY_ERROR;
  }

  /* stop */
  return PLAY_ERROR;
}

static void
unload_movie(Movie *m)
{
  UNGIF_info *info = (UNGIF_info *)m->movie_private;

  if (info->buffer[0])
    free(info->buffer[0]);
  if (info->buffer)
    free(info->buffer);

  if (DGifCloseFile(info->gf) == GIF_ERROR) {
    PrintGifError();
  }

  free(info);
}

/* methods */

static PlayerStatus
identify(Movie *m, Stream *st)
{
  char buf[3];

  if (stream_read(st, buf, 3) != 3)
    return PLAY_NOT;

  if (memcmp(buf, "GIF", 3) != 0)
    return PLAY_NOT;

  if (stream_read(st, buf, 3) != 3)
    return PLAY_NOT;

  if (memcmp(buf, "87a", 3) == 0)
    return PLAY_OK;
  if (memcmp(buf, "89a", 3) == 0)
    return PLAY_OK;

  show_message("GIF detected, but version is neither 87a nor 89a.\n");

  return PLAY_OK;
}

static PlayerStatus
load(Movie *m, Stream *st)
{
  debug_message("ungif player: load() called\n");

#ifdef IDENTIFY_BEFORE_PLAY
  {
    PlayerStatus status;

    if ((status = identify(m, st)) != PLAY_OK)
      return status;
    stream_rewind(st);
  }
#endif

  m->get_screen = get_screen;
  m->play = play;
  m->play_main = play_main;
  m->pause_movie = pause_movie;
  m->stop = stop_movie;
  m->unload_movie = unload_movie;

  return load_movie(m, st);
}
