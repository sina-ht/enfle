/*
 * mpeg_lib.c -- mpeg_lib player plugin, which exploits mpeg_lib.
 * (C)Copyright 2000, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Mar  6 12:14:55 2004.
 * $Id: mpeg_lib.c,v 1.21 2004/03/06 03:43:36 sian Exp $
 *
 * NOTES:
 *  Requires mpeg_lib version 1.3.1 (or later).
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
#include <mpeg.h>

#include "common.h"

#include "enfle/player-plugin.h"

typedef struct _mpeg_lib_info {
  Image *p;
  ImageDesc img;
} MPEG_lib_info;

static const unsigned int types = IMAGE_RGBA32;

DECLARE_PLAYER_PLUGIN_METHODS;

static PlayerStatus play(Movie *);
static PlayerStatus pause_movie(Movie *);
static PlayerStatus stop_movie(Movie *);

static PlayerPlugin plugin = {
  .type = ENFLE_PLUGIN_PLAYER,
  .name = "MPEG_lib",
  .description = "MPEG_lib Player plugin version 0.2.3",
  .author = "Hiroshi Takekawa",

  .identify = identify,
  .load = load
};

ENFLE_PLUGIN_ENTRY(player_mpeg_lib)
{
  PlayerPlugin *pp;

  if ((pp = (PlayerPlugin *)calloc(1, sizeof(PlayerPlugin))) == NULL)
    return NULL;
  memcpy(pp, &plugin, sizeof(PlayerPlugin));

  return (void *)pp;
}

ENFLE_PLUGIN_EXIT(player_mpeg_lib, p)
{
  free(p);
}

/* for internal use */

static int
mpeg_lib_input_func(void *d, unsigned char *p, int s)
{
  Stream *st = (Stream *)d;

  return stream_read(st, p, s);
}

static void
mpeg_lib_rewind_func(void *d)
{
  Stream *st = (Stream *)d;

  stream_rewind(st);
}

static PlayerStatus
load_movie(VideoWindow *vw, Movie *m, Stream *st)
{
  MPEG_lib_info *info;
  Image *p;

  if ((info = calloc(1, sizeof(MPEG_lib_info))) == NULL) {
    show_message("MPEG_lib: play_movie: No enough memory.\n");
    return PLAY_ERROR;
  }

  m->requested_type = video_window_request_type(vw, types, &m->direct_decode);
  debug_message("MPEG_lib: requested type: %s %s\n", image_type_to_string(m->requested_type), m->direct_decode ? "direct" : "not direct");

  SetMPEGOption(MPEG_DITHER, FULL_COLOR_DITHER);
  if (!OpenMPEGStream((void *)st, &info->img, mpeg_lib_input_func)) {
    free(info);
    return PLAY_ERROR;
  }

  m->width = info->img.Width;
  m->height = info->img.Height;
  m->rendering_width  = m->width;
  m->rendering_height = m->height;

  p = info->p = image_create();
  p->width = m->rendering_width;
  p->height = m->rendering_height;
  p->type = _RGBA32;
  p->depth = 24;
  p->bytes_per_line = p->width * 4;
  p->bits_per_pixel = 32;
  p->next = NULL;

  if (m->direct_decode) {
    p->rendered.image = memory_create();
    //memory_request_type(p->rendered.image, video_window_preferred_memory_type(vw));
    if (memory_alloc(p->rendered.image, p->width * p->height * 4) == NULL)
      return PLAY_ERROR;
  } else {
    p->rendered.image = memory_create();
    //memory_request_type(p->rendered.image, video_window_preferred_memory_type(vw));
    p->image = memory_create();
    if (memory_alloc(p->image, p->width * p->height * 4) == NULL)
      return PLAY_ERROR;
  }

  m->movie_private = (void *)info;
  m->st = st;
  m->status = _STOP;

  m->initialize_screen(vw, m, p->width, p->height);

  return play(m);
}

static PlayerStatus
play(Movie *m)
{
  MPEG_lib_info *info = (MPEG_lib_info *)m->movie_private;

  switch (m->status) {
  case _PLAY:
  case _RESIZING:
    return PLAY_OK;
  case _PAUSE:
    return pause_movie(m);
  case _STOP:
    m->status = _PLAY;
    break;
  case _UNLOADED:
    return PLAY_ERROR;
  default:
    warning("Unknown status %d\n", m->status);
    return PLAY_ERROR;
  }

  RewindMPEGStream(m->st, &info->img, mpeg_lib_rewind_func);
  m->current_frame = 0;

  return PLAY_OK;
}

static PlayerStatus
play_main(Movie *m, VideoWindow *vw)
{
  int more_frame;
  MPEG_lib_info *info = (MPEG_lib_info *)m->movie_private;
  Image *p = info->p;

  switch (m->status) {
  case _PLAY:
  case _RESIZING:
    break;
  case _PAUSE:
  case _STOP:
    return PLAY_OK;
  case _UNLOADED:
    return PLAY_ERROR;
  default:
    warning("Unknown status %d\n", m->status);
    return PLAY_ERROR;
  }

  more_frame = GetMPEGFrame(memory_ptr(m->direct_decode ? p->rendered.image : p->image));

  m->current_frame++;
  m->render_frame(vw, m, p);

  if (!more_frame)
    stop_movie(m);

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
    return PLAY_OK;
  case _UNLOADED:
    warning("Unknown status %d\n", m->status);
    return PLAY_ERROR;
  }

  return PLAY_ERROR;
}

static PlayerStatus
stop_movie(Movie *m)
{
  switch (m->status) {
  case _PLAY:
  case _RESIZING:
    m->status = _STOP;
    return PLAY_OK;
  case _PAUSE:
  case _STOP:
    return PLAY_OK;
  case _UNLOADED:
    return PLAY_ERROR;
  default:
    return PLAY_ERROR;
    warning("Unknown status %d\n", m->status);
  }

  return PLAY_ERROR;
}

static void
unload_movie(Movie *m)
{
  MPEG_lib_info *info = (MPEG_lib_info *)m->movie_private;

  stop_movie(m);

  if (info->p)
    image_destroy(info->p);

  CloseMPEG();

  free(info);
}

/* methods */

DEFINE_PLAYER_PLUGIN_IDENTIFY(m, st, c, priv)
{
  char buf[4];
  static unsigned char video_stream_header[]  = { 0x00, 0x00, 0x01, 0xb3 };
  static unsigned char system_stream_header[] = { 0x00, 0x00, 0x01, 0xba };

  if (stream_read(st, buf, 4) != 4)
    return PLAY_NOT;

  if (memcmp(buf, video_stream_header, 4) == 0)
    return PLAY_OK;
  if (memcmp(buf, system_stream_header, 4) == 0) {
    show_message("This seems to be a system MPEG stream. mpeg_lib can play video MPEG streams only.\n");
    return PLAY_NOT;
  }

  return PLAY_NOT;
}

DEFINE_PLAYER_PLUGIN_LOAD(vw, m, st, c, priv)
{
  debug_message("mpeg_lib player: load() called\n");

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
