/*
 * ungif.c -- gif player plugin, which exploits libungif.
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Jun 18 21:36:06 2001.
 * $Id: ungif.c,v 1.19 2001/06/18 16:23:47 sian Exp $
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

#include "enfle/player-plugin.h"

typedef struct _ungif_info {
  GifFileType *gf;
  GifRowType *buffer;
  Image *p;
} UNGIF_info;

typedef enum {
  _NOTHING_DISPOSAL,
  _LEFTIMAGE,
  _RESTOREBACKGROUND,
  _RESTOREPREVIOUS
} Disposal;

static const unsigned int types = IMAGE_INDEX;

DECLARE_PLAYER_PLUGIN_METHODS;

static PlayerStatus pause_movie(Movie *);
static PlayerStatus stop_movie(Movie *);

static PlayerPlugin plugin = {
  type: ENFLE_PLUGIN_PLAYER,
  name: "UNGIF",
  description: "UNGIF Player plugin version 0.2.3",
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
load_movie(VideoWindow *vw, Movie *m, Stream *st)
{
  int i, size;
  UNGIF_info *info;
  Image *p;

  if ((info = calloc(1, sizeof(UNGIF_info))) == NULL) {
    show_message("UNGIF: load_movie: No enough memory.\n");
    return PLAY_ERROR;
  }

  m->requested_type = video_window_request_type(vw, types, &m->direct_decode);
  debug_message("UNGIF: requested type: %s %s\n", image_type_to_string(m->requested_type), m->direct_decode ? "direct" : "not direct");

  if ((info->gf = DGifOpen(st, ungif_input_func)) == NULL) {
    PrintGifError();
    free(info);
    return PLAY_ERROR;
  }

  m->width = info->gf->SWidth;
  m->height = info->gf->SHeight;
  m->rendering_width  = m->width;
  m->rendering_height = m->height;

  p = info->p = image_create();
  p->width = m->rendering_width;
  p->height = m->rendering_height;
  p->type = _INDEX;
  p->depth = 8;
  p->bytes_per_line = m->width;
  p->bits_per_pixel = 8;
  p->next = NULL;

  if (m->direct_decode) {
    p->rendered.image = memory_create();
    //memory_request_type(p->rendered.image, video_window_preferred_memory_type(vw));
    if (memory_alloc(p->rendered.image, p->bytes_per_line * p->height) == NULL)
      return PLAY_ERROR;
  } else {
    p->rendered.image = memory_create();
    //memory_request_type(p->rendered.image, video_window_preferred_memory_type(vw));
    p->image = memory_create();
    if (memory_alloc(p->image, p->bytes_per_line * p->height) == NULL)
      return PLAY_ERROR;
  }

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

  m->movie_private = (void *)info;
  m->st = st;
  m->status = _PLAY;
  m->current_frame = 0;

  m->initialize_screen(vw, m, m->width, m->height);

  return PLAY_OK;
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
play_main(Movie *m, VideoWindow *vw)
{
  int i, j, extcode, image_loaded = 0;
  int if_transparent = 0, transparent_index = 0, delay = 0, image_disposal = 0;
  int l, t, w, h;
  static int ioffset[] = { 0, 4, 2, 1 };
  static int ijumps[] = { 8, 8, 4, 2 };
  UNGIF_info *info = (UNGIF_info *)m->movie_private;
  GifFileType *gf = info->gf;
  GifRowType *rows;
  GifRecordType rectype;
  GifByteType *extension;
  ColorMapObject *ColorMap;
  Image *p = info->p;

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

  do {
    if (DGifGetRecordType(gf, &rectype) == GIF_ERROR) {
      if (image_loaded)
	break;
      PrintGifError();
      return PLAY_OK;
    }

    switch (rectype) {
    case IMAGE_DESC_RECORD_TYPE:
      if (DGifGetImageDesc(gf) == GIF_ERROR)
	return PLAY_ERROR;

      ColorMap = gf->Image.ColorMap ? gf->Image.ColorMap : gf->SColorMap;
      p->ncolors = ColorMap->ColorCount;

      for (i = 0; i < p->ncolors; i++) {
	p->colormap[i][0] = ColorMap->Colors[i].Red;
	p->colormap[i][1] = ColorMap->Colors[i].Green;
	p->colormap[i][2] = ColorMap->Colors[i].Blue;
      }

      p->background_color.index = gf->SBackGroundColor;
      p->transparent_color.index = transparent_index;

      l = gf->Image.Left;
      t = gf->Image.Top;
      w = gf->Image.Width;
      h = gf->Image.Height;

      debug_message("IMAGE_DESC_RECORD_TYPE: (%d, %d) (%d, %d)\n", l, t, w, h);

      if (if_transparent) {
	/* First, allocate memory */
	if ((rows = calloc(h, sizeof(GifRowType *))) == NULL)
	  return PLAY_ERROR;
	if ((rows[0] = calloc(w * h, sizeof(GifPixelType))) == NULL) {
	  free(rows);
	  return PLAY_ERROR;
	}
	for (i = 1; i < h; i++)
	  rows[i] = rows[0] + i * w;
	/* Then, load into it */
	if (gf->Image.Interlace) {
	  for (i = 0; i < 4; i++)
	    for (j = ioffset[i]; j < h; j += ijumps[i])
	      if (DGifGetLine(gf, rows[j], w) == GIF_ERROR) {
		free(rows[0]);
		free(rows);
		return PLAY_ERROR;
	      }
	} else {
	  for (i = 0; i < h; i++)
	    if (DGifGetLine(gf, rows[i], w) == GIF_ERROR) {
	      free(rows[0]);
	      free(rows);
	      return PLAY_ERROR;
	    }
	}
	/* Last, draw into screen, processing transparency */
	for (i = 0; i < h; i++)
	  for (j = 0; j < w; j++)
	    if (rows[i][j] != transparent_index)
	      info->buffer[i + t][j + l] = rows[i][j];
	free(rows[0]);
	free(rows);
      } else {
	/* Just load into screen */
	if (gf->Image.Interlace) {
	  for (i = 0; i < 4; i++)
	    for (j = ioffset[i]; j < h; j += ijumps[i])
	      if (DGifGetLine(gf, &info->buffer[j + t][l], w) == GIF_ERROR)
		return PLAY_ERROR;
	} else {
	  for (i = 0; i < h; i++)
	    if (DGifGetLine(gf, &info->buffer[i + t][l], w) == GIF_ERROR)
	      return PLAY_ERROR;
	}
      }

      if (m->direct_decode) {
	if (memory_alloc(p->rendered.image, p->bytes_per_line * p->height) == NULL)
	  return PLAY_ERROR;
	memcpy(memory_ptr(p->rendered.image), info->buffer[0], memory_used(p->rendered.image));
      } else {
	if (memory_alloc(p->image, p->bytes_per_line * p->height) == NULL)
	  return PLAY_ERROR;
	memcpy(memory_ptr(p->image), info->buffer[0], memory_used(p->image));
      }

      m->current_frame++;
      image_loaded = 1;
      break;
    case EXTENSION_RECORD_TYPE:
      if (DGifGetExtension(gf, &extcode, &extension) == GIF_ERROR)
	return PLAY_ERROR;

      switch (extcode) {
      case COMMENT_EXT_FUNC_CODE:
	debug_message("comment: ");
	if ((p->comment = malloc(extension[0] + 1)) == NULL) {
	  debug_message("(abandoned)\n");
	  show_message("No enough memory for comment. Try to continue.\n");
	} else {
	  memcpy(p->comment, &extension[1], extension[0]);
	  p->comment[extension[0]] = '\0';
	  debug_message("%s\n", p->comment);
	}
	break;
      case GRAPHICS_EXT_FUNC_CODE:
	if (extension[1] & 1) {
	  if_transparent = 1;
	  transparent_index = extension[4];
	} else {
	  if_transparent = 0;
	  transparent_index = -1;
	}
	image_disposal = (extension[1] & 0x1c) >> 2;
	delay = extension[2] + (extension[3] << 8);
	break;
      default:
	break;
      }

      for (;;) {
	if (DGifGetExtensionNext(gf, &extension) == GIF_ERROR)
	  return PLAY_ERROR;
	if (extension == NULL)
	  break;

	switch (extcode) {
	case COMMENT_EXT_FUNC_CODE:
	  {
	    unsigned char *tmp;

	    if ((tmp = realloc(p->comment, strlen(p->comment) + extension[0] + 1)) == NULL) {
	      show_message("No enough memory for comment(append). Truncated.\n");
	    } else {
	      memcpy(tmp + strlen(tmp), &extension[1], extension[0]);
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
      stop_movie(m);
      return PLAY_OK;
    default:
      break;
    }
  } while (!image_loaded);

  m->render_frame(vw, m, p);
  if (delay)
    m->pause_usec(delay * 10000);

  if (image_disposal == _RESTOREBACKGROUND)
    memset(info->buffer[0], gf->SBackGroundColor, m->width * m->height);

  return PLAY_OK;
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
  UNGIF_info *info = (UNGIF_info *)m->movie_private;

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
  if (DGifCloseFile(info->gf) == GIF_ERROR) {
    PrintGifError();
    return PLAY_ERROR;
  }

  stream_rewind(m->st);
  if ((info->gf = DGifOpen(m->st, ungif_input_func)) == NULL) {
    PrintGifError();
    return PLAY_ERROR;
  }

  m->current_frame = 0;

  return PLAY_OK;
}

static void
unload_movie(Movie *m)
{
  UNGIF_info *info = (UNGIF_info *)m->movie_private;

  if (info->buffer[0])
    free(info->buffer[0]);
  if (info->buffer)
    free(info->buffer);
  if (info->p)
    image_destroy(info->p);

  if (DGifCloseFile(info->gf) == GIF_ERROR) {
    PrintGifError();
  }

  free(info);
}

/* methods */

DEFINE_PLAYER_PLUGIN_IDENTIFY(m, st, c, priv)
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

DEFINE_PLAYER_PLUGIN_LOAD(vw, m, st, c, priv)
{
  debug_message("ungif player: load() called\n");

#ifdef IDENTIFY_BEFORE_PLAY
  {
    PlayerStatus status;

    if ((status = identify(m, st, c, priv)) != PLAY_OK)
      return status;
    stream_rewind(st);
  }
#endif

  m->play = play;
  m->play_main = play_main;
  m->pause_movie = pause_movie;
  m->stop = stop_movie;
  m->unload_movie = unload_movie;

  return load_movie(vw, m, st);
}
