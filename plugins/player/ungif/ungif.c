/*
 * ungif.c -- gif player plugin, which exploits libungif.
 * (C)Copyright 2000, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Wed Mar  1 00:32:34 2006.
 * $Id: ungif.c,v 1.34 2006/03/12 08:24:16 sian Exp $
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

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "enfle/player-plugin.h"

typedef struct _ungif_info {
  GifFileType *gf;
  GifRowType *buffer;
  unsigned char *prev_buffer;
  unsigned int buffer_size;
  Image *p;
  unsigned int delay_time;
  unsigned int max_loop_count;
  unsigned int loop_count;
} UNGIF_info;

static const unsigned int types = IMAGE_INDEX;

DECLARE_PLAYER_PLUGIN_METHODS;

static PlayerStatus pause_movie(Movie *);
static PlayerStatus stop_movie(Movie *);

static PlayerPlugin plugin = {
  .type = ENFLE_PLUGIN_PLAYER,
  .name = "UNGIF",
  .description = "UNGIF Player plugin version 0.4.1 with libungif",
  .author = "Hiroshi Takekawa",

  .identify = identify,
  .load = load
};

ENFLE_PLUGIN_ENTRY(player_ungif)
{
  PlayerPlugin *pp;

  if ((pp = (PlayerPlugin *)calloc(1, sizeof(PlayerPlugin))) == NULL)
    return NULL;
  memcpy(pp, &plugin, sizeof(PlayerPlugin));

  return (void *)pp;
}

ENFLE_PLUGIN_EXIT(player_ungif, p)
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
  int i, size, direct_renderable;
  UNGIF_info *info;
  Image *p;

  if ((info = calloc(1, sizeof(UNGIF_info))) == NULL) {
    show_message("UNGIF: load_movie: No enough memory.\n");
    return PLAY_ERROR;
  }

  m->requested_type = video_window_request_type(vw, types, &direct_renderable);
  debug_message("UNGIF: requested type: %s %s\n", image_type_to_string(m->requested_type), direct_renderable ? "direct" : "not direct");

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
  p->direct_renderable = direct_renderable;
  image_width(p) = m->rendering_width;
  image_height(p) = m->rendering_height;
  p->type = _INDEX;
  p->depth = 8;
  image_bpl(p) = m->width;
  p->bits_per_pixel = 8;

  if (p->direct_renderable) {
    image_rendered_image(p) = memory_create();
    //memory_request_type(image_rendered_image(p), video_window_preferred_memory_type(vw));
    if (memory_alloc(image_rendered_image(p), image_bpl(p) * image_height(p)) == NULL)
      return PLAY_ERROR;
  } else {
    image_rendered_image(p) = memory_create();
    //memory_request_type(image_rendered_image(p), video_window_preferred_memory_type(vw));
    image_image(p) = memory_create();
    if (memory_alloc(image_image(p), image_bpl(p) * image_height(p)) == NULL)
      return PLAY_ERROR;
  }

  if ((info->buffer = (GifRowType *)calloc(m->height, sizeof(GifRowType *))) == NULL) {
    return PLAY_ERROR;
  }

  size = m->width * sizeof(GifPixelType);
  if ((info->buffer[0] = (GifRowType)calloc(m->height, size)) == NULL) {
    return PLAY_ERROR;
  }
  if ((info->prev_buffer = calloc(m->height, size)) == NULL) {
    return PLAY_ERROR;
  }
  info->buffer_size = m->height * size;

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
  UNGIF_info *info = (UNGIF_info *)m->movie_private;

  switch (m->status) {
  case _PLAY:
  case _RESIZING:
    return PLAY_OK;
  case _PAUSE:
    return pause_movie(m);
  case _STOP:
  case _REWINDING:
    m->status = _PLAY;
    break;
  case _UNLOADED:
    return PLAY_ERROR;
  default:
    warning("Unknown status %d\n", m->status);
    return PLAY_ERROR;
  }

  m->current_frame = 0;

  if (info->max_loop_count && info->loop_count >= info->max_loop_count)
    return PLAY_OK;

  stream_rewind(m->st);
  if ((info->gf = DGifOpen(m->st, ungif_input_func)) == NULL) {
    PrintGifError();
    return PLAY_ERROR;
  }

  return PLAY_OK;
}

static PlayerStatus
play_main(Movie *m, VideoWindow *vw)
{
  int extcode, image_loaded = 0;
  unsigned int i, j, w, h;
  int if_transparent = 0, transparent_index = 0, delay = 0, image_disposal = 0, user_input = 0;
  int reading_netscape_ext;
  int l, t;
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
  case _RESIZING:
    break;
  case _PAUSE:
  case _STOP:
  case _REWINDING:
    return PLAY_OK;
  case _UNLOADED:
    return PLAY_ERROR;
  default:
    warning("Unknown status %d\n", m->status);
    return PLAY_ERROR;
  }

  if (!info->gf)
    return PLAY_OK;

  if (timer_is_running(m->timer) &&
      timer_get_micro(m->timer) < info->delay_time)
    return PLAY_OK;
  timer_stop(m->timer);

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

      //debug_message("IMAGE_DESC_RECORD_TYPE: (%d, %d) (%d, %d)\n", l, t, w, h);

      /* save previous image */
      memcpy(info->prev_buffer, info->buffer[0], info->buffer_size);

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

      if (p->direct_renderable) {
	if (memory_alloc(image_rendered_image(p), image_bpl(p) * image_height(p)) == NULL)
	  return PLAY_ERROR;
	memcpy(memory_ptr(image_rendered_image(p)), info->buffer[0], memory_used(image_rendered_image(p)));
      } else {
	if (memory_alloc(image_image(p), image_bpl(p) * image_height(p)) == NULL)
	  return PLAY_ERROR;
	memcpy(memory_ptr(image_image(p)), info->buffer[0], memory_used(image_image(p)));
      }

      m->current_frame++;
      image_loaded = 1;
      break;
    case EXTENSION_RECORD_TYPE:
      if (DGifGetExtension(gf, &extcode, &extension) == GIF_ERROR)
	return PLAY_ERROR;

      reading_netscape_ext = 0;
      switch (extcode) {
      case COMMENT_EXT_FUNC_CODE:
	//debug_message("comment: ");
	if ((p->comment = malloc(extension[0] + 1)) == NULL) {
	  //debug_message("(abandoned)\n");
	  show_message("No enough memory for comment. Try to continue.\n");
	} else {
	  memcpy(p->comment, &extension[1], extension[0]);
	  p->comment[extension[0]] = '\0';
	  //debug_message("%s\n", p->comment);
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
	user_input = extension[1] & 2;
	image_disposal = (extension[1] & 0x1c) >> 2;
	delay = extension[2] + (extension[3] << 8);
	//debug_message("UNGIF: Graphics Extention: trans %d(index %d) user_input %d disposal %d delay %d\n", if_transparent, transparent_index, user_input, image_disposal, delay);
	break;
      case APPLICATION_EXT_FUNC_CODE:
	debug_message("UNGIF: Got application extension: %c%c%c%c%c%c%c%c: %d bytes.\n",
		      extension[1], extension[2], extension[3], extension[4],
		      extension[5], extension[6], extension[7], extension[8],
		      extension[0]);
	if (memcmp(extension + 1, "NETSCAPE", 8) == 0) {
	  if (extension[0] != 11) {
	    debug_message("UNGIF: Netscape extension found, but the size of the block is %d bytes(should be 11 bytes), ignored.\n", extension[0]);
	    break;
	  }
	  debug_message("UNGIF: Authentication code %c%c%c.\n", extension[9], extension[10], extension[11]);
	  if (memcmp(extension + 9, "2.0", 3) != 0) {
	    debug_message("UNGIF: Netscape extension found, but the authenication code is not known, ignored.\n");
	    break;
	  }
	  reading_netscape_ext = 1;
	} else {
	  debug_message("UNGIF: Unknown extension, ignored.\n");
	}
	break;
      default:
	debug_message("UNGIF: extension %X ignored.\n", extcode);
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
	    char *tmp;

	    if ((tmp = realloc(p->comment, strlen((const char *)p->comment) + extension[0] + 1)) == NULL) {
	      show_message("No enough memory for comment(append). Truncated.\n");
	    } else {
	      memcpy(tmp + strlen((const char *)tmp), &extension[1], extension[0]);
	      p->comment = tmp;
	    }
	  }
	  break;
	case APPLICATION_EXT_FUNC_CODE:
	  if (reading_netscape_ext) {
	    //debug_message("UNGIF: The size of data sub-block is %d bytes.\n", extension[0]);
	    //debug_message("UNGIF: The first byte of the data sub-block is %X.\n", extension[1]);
	    if ((extension[1] & 7) == 1) {
	      info->max_loop_count = extension[2] | (extension[3] << 8);
	      debug_message("UNGIF: Loop times specified: %d\n", info->max_loop_count);
	    } else {
	      debug_message("UNGIF: Unknown, ignored.\n");
	    }
	  }
	  break;
	default:
	  break;
	}
      }
      break;
    case TERMINATE_RECORD_TYPE:
      info->loop_count++;
      if (info->max_loop_count)
	debug_message("UNGIF: loop %d/%d\n", info->loop_count, info->max_loop_count);
      stop_movie(m);
      if (info->max_loop_count == 0 || info->loop_count < info->max_loop_count)
	m->status = _REWINDING;
      return PLAY_OK;
    default:
      break;
    }
  } while (!image_loaded);

  m->render_frame(vw, m, p);
  info->delay_time = delay * 10000;
  timer_start(m->timer);

  if (image_disposal == _RESTOREBACKGROUND)
    memset(info->buffer[0], gf->SBackGroundColor, m->width * m->height);
  if (image_disposal == _RESTOREPREVIOUS) {
    /* Restore previous image */
    memcpy(info->buffer[0], info->prev_buffer, info->buffer_size);
  }

  return PLAY_OK;
}

static PlayerStatus
pause_movie(Movie *m)
{
  switch (m->status) {
  case _PLAY:
  case _RESIZING:
    m->status = _PAUSE;
    return PLAY_OK;
  case _PAUSE:
    m->status = _PLAY;
    return PLAY_OK;
  case _STOP:
  case _REWINDING:
    return PLAY_OK;
  case _UNLOADED:
    return PLAY_ERROR;
  default:
    warning("Unknown status %d\n", m->status);
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
  case _RESIZING:
    m->status = _STOP;
    break;
  case _PAUSE:
  case _STOP:
  case _REWINDING:
    return PLAY_OK;
  case _UNLOADED:
    return PLAY_ERROR;
  default:
    warning("Unknown status %d\n", m->status);
    return PLAY_ERROR;
  }

  if (info->gf) {
    /* stop */
    if (DGifCloseFile(info->gf) == GIF_ERROR) {
      PrintGifError();
      return PLAY_ERROR;
    }
    info->gf = NULL;
  }

  return PLAY_OK;
}

static void
unload_movie(Movie *m)
{
  UNGIF_info *info = (UNGIF_info *)m->movie_private;

  timer_stop(m->timer);
  if (info->prev_buffer)
    free(info->prev_buffer);
  if (info->buffer[0])
    free(info->buffer[0]);
  if (info->buffer)
    free(info->buffer);
  if (info->p)
    image_destroy(info->p);

  if (info->gf && DGifCloseFile(info->gf) == GIF_ERROR) {
    PrintGifError();
  }

  free(info);
}

/* methods */

DEFINE_PLAYER_PLUGIN_IDENTIFY(m, st, c, priv)
{
  unsigned char buf[3];

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
