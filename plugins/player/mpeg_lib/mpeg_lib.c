/*
 * mpeg_lib.c -- mpeg_lib player plugin, which exploits mpeg_lib.
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Thu Oct 12 21:09:03 2000.
 * $Id: mpeg_lib.c,v 1.2 2000/10/12 15:43:47 sian Exp $
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpeg.h>

#include "common.h"

#include "player-plugin.h"

typedef struct _mpeg_lib_info {
  unsigned char *buffer;
  ImageDesc img;
} MPEG_lib_info;

static PlayerStatus identify(Movie *, Stream *);
static PlayerStatus load(UIData *, Movie *, Stream *);

static PlayerStatus pause_movie(Movie *);
static PlayerStatus stop_movie(Movie *);

static PlayerPlugin plugin = {
  type: ENFLE_PLUGIN_PLAYER,
  name: "MPEG_lib",
  description: "MPEG_lib Player plugin version 0.2",
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
load_movie(UIData *uidata, Movie *m, Stream *st)
{
  MPEG_lib_info *info;

  if ((info = calloc(1, sizeof(MPEG_lib_info))) == NULL) {
    show_message("MPEG_lib: play_movie: No enough memory.\n");
    return PLAY_ERROR;
  }

  SetMPEGOption(MPEG_DITHER, FULL_COLOR_DITHER);
  if (!OpenMPEGStream((void *)st, &info->img, mpeg_lib_input_func)) {
    free(info);
    return PLAY_ERROR;
  }

  m->width = info->img.Width;
  m->height = info->img.Height;

  if ((info->buffer = calloc(m->width * 4, m->height)) == NULL) {
    return PLAY_ERROR;
  }

  m->movie_private = (void *)info;
  m->st = st;
  m->status = _PLAY;
  m->nthframe = 0;

  m->initialize_screen(uidata, m, m->width, m->height);

  return PLAY_OK;
}

static void *
get_screen(Movie *m)
{
  MPEG_lib_info *info;

  if (m->movie_private) {
    info = (MPEG_lib_info *)m->movie_private;
    return info->buffer;
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
play_main(Movie *m, UIData *uidata)
{
  int more_frame;
  MPEG_lib_info *info = (MPEG_lib_info *)m->movie_private;
  Image *p;

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

  more_frame = GetMPEGFrame(info->buffer);

  p->width  = m->width;
  p->height = m->height;
  p->type = _RGBA32;
  p->depth = 24;
  p->bytes_per_line = m->width * 4;
  p->bits_per_pixel = 32;
  p->next = NULL;
  p->image_size = p->bytes_per_line * p->height;
  if ((p->image = malloc(p->image_size)) == NULL)
    goto error;
  memcpy(p->image, info->buffer, p->image_size);

  m->nthframe++;

  m->render_frame(uidata, m, p);

  image_destroy(p);

  if (!more_frame)
    stop_movie(m);

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
  MPEG_lib_info *info = (MPEG_lib_info *)m->movie_private;

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
  RewindMPEGStream(m->st, &info->img, mpeg_lib_rewind_func);
  m->nthframe = 0;

  return PLAY_ERROR;
}

static void
unload_movie(Movie *m)
{
  MPEG_lib_info *info = (MPEG_lib_info *)m->movie_private;

  if (info->buffer)
    free(info->buffer);

  CloseMPEG();

  free(info);
}

/* methods */

static PlayerStatus
identify(Movie *m, Stream *st)
{
  char buf[4];
  static unsigned char video_stream_header[]  = { 0x00, 0x00, 0x01, 0xb3 };
  static unsigned char system_stream_header[] = { 0x00, 0x00, 0x01, 0xba };

  if (stream_read(st, buf, 4) != 4)
    return PLAY_NOT;

  if (memcmp(buf, video_stream_header, 4) == 0)
    return PLAY_OK;
  if (memcmp(buf, system_stream_header, 4) == 0) {
    show_message("This is a system MPEG stream. Needs demultiplexing.\n");
    show_message("mpeg_lib only supports video MPEG stream. Sorry.\n");
    return PLAY_NOT;
  }

  return PLAY_NOT;
}

static PlayerStatus
load(UIData *uidata, Movie *m, Stream *st)
{
  debug_message("mpeg_lib player: load() called\n");

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

  return load_movie(uidata, m, st);
}
