/*
 * libmpeg2.c -- libmpeg2 player plugin, which exploits libmpeg2.
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Thu Feb 22 03:15:30 2001.
 * $Id: libmpeg2.c,v 1.8 2001/02/21 18:25:41 sian Exp $
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
#include <inttypes.h>

#define REQUIRE_STRING_H
#define REQUIRE_UNISTD_H
#include "compat.h"
#include "common.h"

#ifndef USE_PTHREAD
#  error pthread is mandatory for libmpeg2 plugin
#endif

#include "utils/timer.h"
#include "enfle/player-plugin.h"
#include "libmpeg2_vo.h"
#include "demultiplexer/demultiplexer_mpeg.h"

typedef struct _libmpeg2_info {
  Image *p;
  AudioDevice *ad;
  Demultiplexer *demux;
  vo_instance_t *vo;
  int rendering_type;
  pthread_mutex_t update_mutex;
  pthread_cond_t update_cond;
  int nvstreams;
  int nvstream;
  int v_fd_in;
  int v_fd_out;
  pthread_t video_thread;
  int nastreams;
  int nastream;
  int a_fd_in;
  int a_fd_out;
  pthread_t audio_thread;
} Libmpeg2_info;

static const unsigned int types =
  (IMAGE_RGBA32 | IMAGE_BGRA32 | IMAGE_RGB24 | IMAGE_BGR24 | IMAGE_BGR_WITH_BITMASK);

static PlayerStatus identify(Movie *, Stream *);
static PlayerStatus load(VideoWindow *, Movie *, Stream *);

static PlayerStatus play(Movie *);
static PlayerStatus pause_movie(Movie *);
static PlayerStatus stop_movie(Movie *);

static PlayerPlugin plugin = {
  type: ENFLE_PLUGIN_PLAYER,
  name: "Libmpeg2",
  description: "Libmpeg2 Player plugin version 0.1",
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

static PlayerStatus
load_movie(VideoWindow *vw, Movie *m, Stream *st)
{
  Libmpeg2_info *info;
  Image *p;

  if ((info = calloc(1, sizeof(Libmpeg2_info))) == NULL) {
    show_message("Libmpeg2: play_movie: No enough memory.\n");
    return PLAY_ERROR;
  }

  pthread_mutex_init(&info->update_mutex, NULL);
  pthread_cond_init(&info->update_cond, NULL);

  m->requested_type = video_window_request_type(vw, types, &m->direct_decode);
  if (!m->direct_decode) {
    show_message(__FUNCTION__ ": Cannot direct decoding...\n");
    return PLAY_ERROR;
  }
  debug_message("Libmpeg2: requested type: %s direct\n", image_type_to_string(m->requested_type));

  info->demux = demultiplexer_mpeg_create();
  demultiplexer_mpeg_set_input(info->demux, st);
  if (!demultiplexer_examine(info->demux))
    return PLAY_NOT;

  m->has_audio = 0;
  if (demultiplexer_mpeg_naudios(info->demux)) {
    if (m->ap == NULL)
      show_message("Audio not played.\n");
    else {
      info->nastreams = demultiplexer_mpeg_naudios(info->demux);
      /* XXX: stream should be selectable */
      info->nastream = 0;
      demultiplexer_mpeg_set_audio(info->demux, info->nastream);

      show_message("audio(%d streams)\n", info->nastreams);
      if (m->ap->bytes_written == NULL)
	show_message("audio sync may be incorrect.\n");
      m->has_audio = 1;
    }
  } else {
    debug_message("No audio streams.\n");
  }

  m->has_video = 1;
  if (!demultiplexer_mpeg_nvideos(info->demux)) {
    m->has_video = 0;
    m->width = 120;
    m->height = 80;
    m->framerate = 0;
    m->num_of_frames = 1;
    m->rendering_width = m->width;
    m->rendering_height = m->height;
    show_message("warning: This stream has no video stream.\n");
  } else {
    if ((info->nvstreams = demultiplexer_mpeg_nvideos(info->demux)) > 1) {
      show_message("There are %d video streams in this whole stream.\n", info->nvstreams);
      show_message("Only the first video stream will be played(so far). Sorry.\n");
    }

    /* XXX: stream should be selectable */
    info->nvstream = 0;
    demultiplexer_mpeg_set_audio(info->demux, info->nvstream);
  }

  debug_message("video(%d streams)\n", info->nvstreams);

  p = info->p = image_create();
  p->type = m->requested_type;
  if ((p->rendered.image = memory_create()) == NULL)
    goto error;
  memory_request_type(p->rendered.image, video_window_preferred_memory_type(vw));

  if ((info->vo = vo_enfle_rgb_open(vw, m, p)) == NULL)
    goto error;

  switch (vw->bits_per_pixel) {
  case 32:
    p->depth = 24;
    p->bits_per_pixel = 32;
    break;
  case 24:
    p->depth = 24;
    p->bits_per_pixel = 24;
    break;
  case 16:
    p->depth = 16;
    p->bits_per_pixel = 16;
    break;
  default:
    show_message("Cannot render bpp %d\n", vw->bits_per_pixel);
    return PLAY_ERROR;
  }

  m->movie_private = (void *)info;
  m->st = st;
  m->status = _STOP;

  play(m);

  return PLAY_OK;

 error:
  if (info->demux)
    demultiplexer_destroy(info->demux);
  free(info);
  return PLAY_ERROR;
}

static void *play_video(void *);
static void *play_audio(void *);

static PlayerStatus
play(Movie *m)
{
  Libmpeg2_info *info = (Libmpeg2_info *)m->movie_private;
  int fds[2];

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

  m->current_frame = 0;
  m->current_sample = 0;
  timer_start(m->timer);
  demultiplexer_set_eof(info->demux, 0);

  stream_rewind(m->st);
  if (info->a_fd_in) {
    close(info->a_fd_in);
    info->a_fd_in = 0;
  }
  if (info->a_fd_out) {
    close(info->a_fd_out);
    info->a_fd_out = 0;
  }
  if (info->v_fd_in) {
    close(info->v_fd_in);
    info->v_fd_in = 0;
  }
  if (info->v_fd_out) {
    close(info->v_fd_out);
    info->v_fd_out = 0;
  }

    
  if (m->has_video) {
    if (pipe(fds) != 0)
      return PLAY_ERROR;
    info->v_fd_in = fds[0];
    info->v_fd_out = fds[1];
    pthread_create(&info->video_thread, NULL, play_video, m);
  }
  if (m->has_audio) {
    if (pipe(fds) != 0)
      return PLAY_ERROR;
    info->a_fd_in = fds[0];
    info->a_fd_out = fds[1];
    pthread_create(&info->audio_thread, NULL, play_audio, m);
  }

  demultiplexer_mpeg_set_vfd(info->demux, info->v_fd_out);
  demultiplexer_mpeg_set_afd(info->demux, info->a_fd_out);
  demultiplexer_start(info->demux);

  return PLAY_OK;
}

static void *
play_video(void *arg)
{
  Movie *m = arg;
  Libmpeg2_info *info = (Libmpeg2_info *)m->movie_private;

  debug_message(__FUNCTION__ "()\n");

  while (m->status == _PLAY) {
    if (m->current_frame >= m->num_of_frames) {
      demultiplexer_set_eof(info->demux, 1);
      break;
    }

    pthread_mutex_lock(&info->update_mutex);

    /* decoding */

    pthread_cond_wait(&info->update_cond, &info->update_mutex);
    pthread_mutex_unlock(&info->update_mutex);
  }

  debug_message(__FUNCTION__ " exiting.\n");
  pthread_exit((void *)PLAY_OK);
}

#define AUDIO_READ_SIZE 2048
#define AUDIO_WRITE_SIZE 4096

static void *
play_audio(void *arg)
{
  Movie *m = arg;
  Libmpeg2_info *info = (Libmpeg2_info *)m->movie_private;
  AudioDevice *ad;
  short input_buffer[AUDIO_WRITE_SIZE];
  short output_buffer[AUDIO_WRITE_SIZE];
  int samples_to_read;

  debug_message(__FUNCTION__ "()\n");

  if ((ad = m->ap->open_device(NULL, m->c)) == NULL) {
    show_message("Cannot open device.\n");
    pthread_exit((void *)PLAY_ERROR);
  }
  info->ad = ad;

  if (!m->ap->set_params(ad, &m->sampleformat, &m->channels, &m->samplerate))
    show_message("Some params are set wrong.\n");

  samples_to_read = AUDIO_READ_SIZE;

  while (m->status == _PLAY) {
    if (m->current_sample >= m->num_of_samples)
      break;
    if (m->num_of_samples < m->current_sample + samples_to_read)
      samples_to_read = m->num_of_samples - m->current_sample;
    /* read audio */
    if (m->channels == 1) {
      m->ap->write_device(ad, (unsigned char *)input_buffer, samples_to_read * sizeof(short));
    } else {
      int i;

      /* read audio */
      for (i = 0; i < samples_to_read; i++) {
	output_buffer[i * 2    ] = input_buffer[i];
	output_buffer[i * 2 + 1] = input_buffer[i + AUDIO_READ_SIZE];
      }
      m->ap->write_device(ad, (unsigned char *)output_buffer, (samples_to_read << 1) * sizeof(short));
    }
    if (m->current_sample == 0) {
      timer_stop(m->timer);
      timer_start(m->timer);
    }
    m->current_sample += samples_to_read;
  }

  m->ap->sync_device(ad);
  m->ap->close_device(ad);
  info->ad = NULL;

  debug_message(__FUNCTION__ " exiting.\n");

  pthread_exit((void *)PLAY_OK);
}

static int
get_audio_time(Movie *m, AudioDevice *ad)
{
  if (ad && m->ap->bytes_written)
    return (int)((double)m->ap->bytes_written(ad) / m->samplerate * 500 / m->channels);
  return (int)((double)m->current_sample * 1000 / m->samplerate);
}

static PlayerStatus
play_main(Movie *m, VideoWindow *vw)
{
  Libmpeg2_info *info = (Libmpeg2_info *)m->movie_private;
  Image *p = info->p;
  int i = 0;
  int video_time, audio_time;

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

  if (demultiplexer_get_eof(info->demux)) {
    stop_movie(m);
    return PLAY_OK;
  }

  if (!m->has_video)
    return PLAY_OK;

  pthread_mutex_lock(&info->update_mutex);

  video_time = m->current_frame * 1000 / m->framerate;
  if (m->has_audio) {
    audio_time = get_audio_time(m, info->ad);
    //debug_message("v: %d a=%d (%d frame)\n", (int)timer_get_milli(m->timer), audio_time, m->current_frame);

    /* if too fast to display, wait before render */
    while (video_time > audio_time)
      audio_time = get_audio_time(m, info->ad);

    /* skip if delayed */
    i = (audio_time * m->framerate / 1000) - m->current_frame - 1;
    if (i > 0) {
      debug_message("dropped %d frames\n", i);
      /* drop */
    }
  } else {
    /* if too fast to display, wait before render */
    while (video_time > timer_get_milli(m->timer)) ;

    /* skip if delayed */
    i = (timer_get_milli(m->timer) * m->framerate / 1000) - m->current_frame - 1;
    if (i > 0) {
      debug_message("dropped %d frames\n", i);
      /* drop */
    }
  }

  /* tell video thread to continue decoding */
  pthread_cond_signal(&info->update_cond);
  pthread_mutex_unlock(&info->update_mutex);

  m->render_frame(vw, m, p);

  return PLAY_OK;
}

static PlayerStatus
pause_movie(Movie *m)
{
  switch (m->status) {
  case _PLAY:
    m->status = _PAUSE;
    timer_pause(m->timer);
    return PLAY_OK;
  case _PAUSE:
    m->status = _PLAY;
    timer_restart(m->timer);
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
  Libmpeg2_info *info = (Libmpeg2_info *)m->movie_private;
  void *v, *a;

  debug_message(__FUNCTION__ "()\n");

  switch (m->status) {
  case _PLAY:
    m->status = _STOP;
    timer_stop(m->timer);
    break;
  case _PAUSE:
  case _STOP:
    return PLAY_OK;
  case _UNLOADED:
    return PLAY_ERROR;
  default:
    return PLAY_ERROR;
  }

  demultiplexer_stop(info->demux);

  pthread_mutex_lock(&info->update_mutex);
  pthread_cond_signal(&info->update_cond);
  pthread_mutex_unlock(&info->update_mutex);
  if (info->video_thread) {
    pthread_join(info->video_thread, &v);
    info->video_thread = 0;
  }

  if (info->audio_thread) {
    pthread_join(info->audio_thread, &a);
    info->audio_thread = 0;
  }

  return PLAY_OK;
}

static void
unload_movie(Movie *m)
{
  Libmpeg2_info *info = (Libmpeg2_info *)m->movie_private;

  debug_message(__FUNCTION__ "()\n");

  stop_movie(m);

  if (info) {
    if (info->p)
      image_destroy(info->p);

    pthread_mutex_destroy(&info->update_mutex);
    pthread_cond_destroy(&info->update_cond);

    debug_message(__FUNCTION__ ": closing libmpeg2 file\n");
    /* close */

    debug_message(__FUNCTION__ ": freeing info\n");
    free(info);
    debug_message(__FUNCTION__ ": all Ok\n");
  }
}

/* methods */

static PlayerStatus
identify(Movie *m, Stream *st)
{
  unsigned char buf[3];

  if (stream_read(st, buf, 3) != 3)
    return PLAY_NOT;
  if (buf[0] || buf[1] || buf[2] != 1)
    return PLAY_NOT;

  return PLAY_OK;
}

static PlayerStatus
load(VideoWindow *vw, Movie *m, Stream *st)
{
  debug_message("libmpeg2 player: load() called\n");

#ifdef IDENTIFY_BEFORE_PLAY
  {
    PlayerStatus status;

    if ((status = identify(m, st)) != PLAY_OK)
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
