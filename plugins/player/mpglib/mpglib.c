/*
 * mpglib.c -- mpglib player plugin, which exploits mpglib.
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Jan 29 23:58:07 2001.
 * $Id: mpglib.c,v 1.1 2001/01/29 15:10:31 sian Exp $
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
#include "mpglib/mpg123.h"
#include "mpglib/mpglib.h"

#include "common.h"

#ifndef USE_PTHREAD
#  error pthread is mandatory for mpglib plugin
#endif

#include <pthread.h>

#include "utils/timer.h"
#include "enfle/player-plugin.h"

typedef struct _mpglib_info {
  struct mpstr mp;
  Image *p;
  unsigned char *input_buffer, *output_buffer;
  unsigned int pos;
  int eof;
  pthread_t audio_thread;
} Mpglib_info;

static const unsigned int types =
  (IMAGE_RGBA32 | IMAGE_BGRA32 | IMAGE_RGB24 | IMAGE_BGR24 | IMAGE_BGR_WITH_BITMASK);

static PlayerStatus identify(Movie *, Stream *);
static PlayerStatus load(VideoWindow *, Movie *, Stream *);

static PlayerStatus play(Movie *);
static PlayerStatus pause_movie(Movie *);
static PlayerStatus stop_movie(Movie *);

static PlayerPlugin plugin = {
  type: ENFLE_PLUGIN_PLAYER,
  name: "Mpglib",
  description: "Mpglib Player plugin version 0.1",
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

#define MP3_READ_BUFFER_SIZE 4096
#define MP3_DECODE_BUFFER_SIZE 8192

static PlayerStatus
load_movie(VideoWindow *vw, Movie *m, Stream *st)
{
  Mpglib_info *info;
  Image *p;
  int read_size, write_size;

  if ((info = calloc(1, sizeof(Mpglib_info))) == NULL) {
    show_message("Mpglib: play_movie: No enough memory.\n");
    return PLAY_ERROR;
  }

  if ((info->input_buffer = malloc(MP3_READ_BUFFER_SIZE)) == NULL)
    goto error;
  if ((info->output_buffer = malloc(MP3_DECODE_BUFFER_SIZE)) == NULL)
    goto error;

  InitMP3(&info->mp);

  read_size = stream_read(st, info->input_buffer, MP3_READ_BUFFER_SIZE);
  stream_rewind(st);
  decodeMP3(&info->mp,
	    info->input_buffer, read_size,
	    info->output_buffer, MP3_DECODE_BUFFER_SIZE, &write_size);
  m->has_audio = 1;
  m->sampleformat = _AUDIO_FORMAT_S16_LE;
  m->channels = info->mp.fr.stereo;
  m->samplerate = freqs[info->mp.fr.sampling_frequency];

  ExitMP3(&info->mp);

  m->requested_type = video_window_request_type(vw, types, &m->direct_decode);
  if (!m->direct_decode) {
    show_message(__FUNCTION__ ": Cannot direct decoding...\n");
    return PLAY_ERROR;
  }
  debug_message("Mpglib: requested type: %s direct\n", image_type_to_string(m->requested_type));

  m->has_video = 0;
  m->width = 120;
  m->height = 80;
  m->framerate = 0;
  m->num_of_frames = 1;
  m->rendering_width = m->width;
  m->rendering_height = m->height;

  p = info->p = image_create();
  p->width = m->rendering_width;
  p->height = m->rendering_height;
  p->type = m->requested_type;
  if ((p->rendered.image = memory_create()) == NULL)
    goto error;
  memory_request_type(p->rendered.image, video_window_preferred_memory_type(vw));

  switch (vw->bits_per_pixel) {
  case 32:
    p->depth = 24;
    p->bits_per_pixel = 32;
    p->bytes_per_line = p->width * 4;
    break;
  case 24:
    p->depth = 24;
    p->bits_per_pixel = 24;
    p->bytes_per_line = p->width * 3;
    break;
  case 16:
    p->depth = 16;
    p->bits_per_pixel = 16;
    p->bytes_per_line = p->width * 2;
    break;
  default:
    show_message("Cannot render bpp %d\n", vw->bits_per_pixel);
    return PLAY_ERROR;
  }

  if (memory_alloc(p->rendered.image, p->bytes_per_line * p->height) == NULL)
    goto error;

  m->movie_private = (void *)info;
  m->st = st;
  m->status = _STOP;

  m->initialize_screen(vw, m, m->rendering_width, m->rendering_height);

  play(m);

  return PLAY_OK;

 error:
  if (info->output_buffer)
    free(info->output_buffer);
  if (info->input_buffer)
    free(info->input_buffer);
  free(info);
  return PLAY_ERROR;
}

static void *play_audio(void *);

static PlayerStatus
play(Movie *m)
{
  Mpglib_info *info = (Mpglib_info *)m->movie_private;

  debug_message(__FUNCTION__ "()\n");

  switch (m->status) {
  case _PLAY:
    return PLAY_OK;
  case _PAUSE:
    return pause_movie(m);
  case _STOP:
    m->status = _PLAY;
    break;
  case _UNLOADED:
    return PLAY_ERROR;
  default:
    return PLAY_ERROR;
  }

  InitMP3(&info->mp);

  m->current_frame = 0;
  m->current_sample = 0;
  info->pos = 0;
  info->eof = 0;

  if (m->has_audio)
    pthread_create(&info->audio_thread, NULL, play_audio, m);

  return PLAY_OK;
}

static void *
play_audio(void *arg)
{
  Movie *m = arg;
  Mpglib_info *info = (Mpglib_info *)m->movie_private;
  AudioDevice *ad;
  int read_size, write_size, ret;

  debug_message(__FUNCTION__ "()\n");

  if ((ad = m->ap->open_device(NULL, m->c)) == NULL) {
    show_message("Cannot open device.\n");
    pthread_exit((void *)PLAY_ERROR);
  }

  if (!m->ap->set_params(ad, &m->sampleformat, &m->channels, &m->samplerate))
    show_message("Some params are set wrong.\n");

  while (m->status == _PLAY) {
    read_size = stream_read(m->st, info->input_buffer, MP3_READ_BUFFER_SIZE);
    info->pos += read_size;
    if (!read_size) {
      info->eof = 1;
      break;
    }
    ret = decodeMP3(&info->mp,
		    info->input_buffer, read_size,
		    info->output_buffer, MP3_DECODE_BUFFER_SIZE, &write_size);
    while (ret == MP3_OK) {
      m->ap->write_device(ad, info->output_buffer, write_size);
      ret = decodeMP3(&info->mp,
		      NULL, 0,
		      info->output_buffer, MP3_DECODE_BUFFER_SIZE, &write_size);
    }
  }

  m->ap->sync_device(ad);
  m->ap->close_device(ad);

  debug_message(__FUNCTION__ " exiting.\n");

  pthread_exit((void *)PLAY_OK);
}

static PlayerStatus
play_main(Movie *m, VideoWindow *vw)
{
  Mpglib_info *info = (Mpglib_info *)m->movie_private;
  //Image *p = info->p;

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

  if (info->eof) {
    stop_movie(m);
    return PLAY_OK;
  }

  //m->render_frame(vw, m, p);

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
  Mpglib_info *info = (Mpglib_info *)m->movie_private;
  void *a;

  debug_message(__FUNCTION__ "()\n");

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

  if (info->audio_thread) {
    pthread_join(info->audio_thread, &a);
    info->audio_thread = 0;
  }

  ExitMP3(&info->mp);

  return PLAY_OK;
}

static void
unload_movie(Movie *m)
{
  Mpglib_info *info = (Mpglib_info *)m->movie_private;

  debug_message(__FUNCTION__ "()\n");

  stop_movie(m);

  if (info) {
    if (info->p)
      image_destroy(info->p);
    if (info->input_buffer)
      free(info->input_buffer);
    if (info->output_buffer)
      free(info->output_buffer);

    debug_message(__FUNCTION__ ": freeing info\n");
    free(info);
    debug_message(__FUNCTION__ ": all Ok\n");
  }
}

/* methods */

static PlayerStatus
identify(Movie *m, Stream *st)
{
  if (strlen(st->path) >= 4 && !strcasecmp(st->path + strlen(st->path) - 4, ".mp3"))
    return PLAY_OK;
  return PLAY_NOT;
}

static PlayerStatus
load(VideoWindow *vw, Movie *m, Stream *st)
{
  debug_message("mpglib player: load() called\n");

#ifdef IDENTIFY_BEFORE_PLAY
  {
    PlayerStatus status;

    if ((status = identify(m, st)) != PLAY_OK)
      return status;
  }
#endif

  m->play = play;
  m->play_main = play_main;
  m->pause_movie = pause_movie;
  m->stop = stop_movie;
  m->unload_movie = unload_movie;

  return load_movie(vw, m, st);
}