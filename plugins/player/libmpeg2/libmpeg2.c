/*
 * libmpeg2.c -- libmpeg2 player plugin, which exploits libmpeg2.
 * (C)Copyright 2000-2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Feb 13 00:11:28 2004.
 * $Id: libmpeg2.c,v 1.52 2004/02/14 05:11:51 sian Exp $
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

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#ifndef USE_PTHREAD
#  error pthread is mandatory for libmpeg2 plugin
#endif

#include <pthread.h>

#include "enfle/player-plugin.h"
#include "enfle/audiodecoder.h"
#include "enfle/videodecoder.h"
#include "demultiplexer/demultiplexer_old_mpeg.h"

typedef struct _libmpeg2_info {
  EnflePlugins *eps;
  Image *p;
  Config *c;
  AudioDevice *ad;
  AudioDecoder *adec;
  VideoDecoder *vdec;
  Demultiplexer_old *demux;
  int to_skip;
  int use_xv;
  int if_initialized;
  int nvstreams;
  int nvstream;
  FIFO *vstream;
  pthread_t video_thread;
  int nastreams;
  int nastream;
  FIFO *astream;
  pthread_t audio_thread;
} Libmpeg2_info;

static const unsigned int types =
  (IMAGE_I420 |
   IMAGE_BGRA32 | IMAGE_RGBA32 |
   IMAGE_RGB24 | IMAGE_BGR24 |
   IMAGE_BGR_WITH_BITMASK);

DECLARE_PLAYER_PLUGIN_METHODS;

static PlayerStatus play(Movie *);
static PlayerStatus pause_movie(Movie *);
static PlayerStatus stop_movie(Movie *);

static PlayerPlugin plugin = {
  type: ENFLE_PLUGIN_PLAYER,
  name: "LibMPEG2",
  description: "LibMPEG2 Player plugin version 0.5",
  author: "Hiroshi Takekawa",
  identify: identify,
  load: load
};

ENFLE_PLUGIN_ENTRY(player_libmpeg2)
{
  PlayerPlugin *pp;

  if ((pp = (PlayerPlugin *)calloc(1, sizeof(PlayerPlugin))) == NULL)
    return NULL;
  memcpy(pp, &plugin, sizeof(PlayerPlugin));

  return (void *)pp;
}

ENFLE_PLUGIN_EXIT(player_libmpeg2, p)
{
  free(p);
}

/* for internal use */

static PlayerStatus
load_movie(VideoWindow *vw, Movie *m, Stream *st, Config *c, EnflePlugins *eps)
{
  Libmpeg2_info *info;
  Image *p;

  if ((info = calloc(1, sizeof(Libmpeg2_info))) == NULL) {
    show_message_fnc("No enough memory.\n");
    return PLAY_ERROR;
  }

  info->eps = eps;
  info->c = c;

  m->requested_type = video_window_request_type(vw, types, &m->direct_decode);
  if (!m->direct_decode) {
    err_message_fnc("Cannot direct decoding...\n");
    goto error;
  }
  debug_message("requested type: %s direct\n", image_type_to_string(m->requested_type));

  info->use_xv = (m->requested_type == _I420) ? 1 : 0;

  info->demux = demultiplexer_mpeg_create();
  demultiplexer_mpeg_set_input(info->demux, st);
  if (!demultiplexer_old_examine(info->demux)) {
    demultiplexer_old_destroy(info->demux);
    free(info);
    m->movie_private = NULL;
    return PLAY_NOT;
  }
  info->nastreams = demultiplexer_mpeg_naudios(info->demux);
  info->nvstreams = demultiplexer_mpeg_nvideos(info->demux);

  if (info->nastreams == 0 && info->nvstreams == 0)
    return PLAY_NOT;

  m->has_audio = 0;
  if (info->nastreams > 0) {
    if (m->ap == NULL)
      warning("Audio not played.\n");
    else {
      /* XXX: stream should be selectable */
      info->nastream = 0;
      demultiplexer_mpeg_set_audio(info->demux, info->nastream);

      show_message("audio(%d streams)\n", info->nastreams);

      if (m->ap->bytes_written == NULL)
	warning("audio sync may be incorrect.\n");
      m->has_audio = 1;
    }
  } else {
    debug_message("No audio streams.\n");
  }

  p = info->p = image_create();
  p->type = m->requested_type;
  if ((image_rendered_image(p) = memory_create()) == NULL)
    goto error;

  memory_request_type(image_rendered_image(p), video_window_preferred_memory_type(vw));

  m->has_video = 1;
  if (!info->nvstreams) {
    m->has_video = 0;
    m->width = 120;
    m->height = 80;
    m->framerate = 0;
    m->num_of_frames = 1;
    m->rendering_width = m->width;
    m->rendering_height = m->height;
    show_message("This stream has no video stream.\n");
    info->nvstream = -1;
    demultiplexer_mpeg_set_video(info->demux, info->nvstream);
  } else {
    if (info->nvstreams > 1) {
      show_message("There are %d video streams in this whole stream.\n", info->nvstreams);
      show_message("Only the first video stream will be played(so far). Sorry.\n");
    }

    /* XXX: stream should be selectable */
    info->nvstream = 0;
    demultiplexer_mpeg_set_video(info->demux, info->nvstream);

    show_message("video(%d streams)\n", info->nvstreams);
  }

  if (info->use_xv) {
    p->bits_per_pixel = 12;
    image_bpl(p) = image_width(p) * 1.5;
  } else {
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
      goto error;
    }
  }

  m->movie_private = (void *)info;
  m->st = st;
  m->status = _STOP;

  return play(m);

 error:
  if (info->demux)
    demultiplexer_old_destroy(info->demux);
  free(info);
  m->movie_private = NULL;
  return PLAY_ERROR;
}

static void *play_video(void *);
static void *play_audio(void *);

static PlayerStatus
play(Movie *m)
{
  Libmpeg2_info *info = (Libmpeg2_info *)m->movie_private;

  debug_message_fn("()\n");

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

  m->current_frame = 0;
  m->current_sample = 0;
  timer_start(m->timer);
  demultiplexer_old_set_eof(info->demux, 0);
  demultiplexer_old_rewind(info->demux);

  if (m->has_video) {
    if ((info->vstream = fifo_create()) == NULL)
      return PLAY_ERROR;
    fifo_set_max(info->vstream, 2048);
    demultiplexer_mpeg_set_vst(info->demux, info->vstream);
    pthread_create(&info->video_thread, NULL, play_video, m);
  }

  if (m->has_audio) {
    m->has_audio = 1;
    if ((info->astream = fifo_create()) == NULL)
      return PLAY_ERROR;
    fifo_set_max(info->astream, 128);
    demultiplexer_mpeg_set_ast(info->demux, info->astream);
    pthread_create(&info->audio_thread, NULL, play_audio, m);
  }

  demultiplexer_old_start(info->demux);

  return PLAY_OK;
}

static int
get_audio_time(Movie *m, AudioDevice *ad)
{
  if (ad && m->ap->bytes_written)
    return (int)((double)m->ap->bytes_written(ad) / m->samplerate * 500 / m->channels);
  return (int)((double)m->current_sample * 1000 / m->samplerate);
}

//#define USE_TS
#undef USE_TS

#ifdef USE_TS
#define TS_BASE 90000
#define TS_TO_CLOCK(sec, usec, ts) \
  sec  =  ts / TS_BASE; \
  usec = (ts % TS_BASE) * (1000000 / TS_BASE);
#endif

static void *
play_video(void *arg)
{
  Movie *m = arg;
  Libmpeg2_info *info = (Libmpeg2_info *)m->movie_private;
  VideoDecoderStatus vds;
  void *data;
  DemuxedPacket *dp = NULL;
  FIFO_destructor destructor;
  int used;
#ifdef USE_TS
  unsigned long pts, dts, size;
#if defined(DEBUG)
  unsigned int psec, pusec, dsec, dusec;
#endif
#endif

  debug_message_fn("()\n");

  info->vdec = videodecoder_create(info->eps, "LibMPEG2");
  if (info->vdec == NULL) {
    warning_fnc("videodecoder plugin not found.\n");
    return (void *)VD_ERROR;
  }

  vds = VD_OK;
  while (m->status == _PLAY) {
    while (m->status == _PLAY && vds == VD_OK)
      vds = videodecoder_decode(info->vdec, m, info->p, NULL, 0, NULL);
    if (vds == VD_NEED_MORE_DATA) {
      if (!fifo_get(info->vstream, &data, &destructor)) {
	debug_message_fnc("fifo_get() failed.\n");
	goto quit;
      }
      if (dp)
	destructor(dp);
      dp = (DemuxedPacket *)data;
#if defined(USE_TS)
      pts = dp->pts;
      dts = dp->dts;
#if defined(DEBUG)
      if (pts != -1) {
	TS_TO_CLOCK(psec, pusec, pts);
	TS_TO_CLOCK(dsec, dusec, dts);
	debug_message_fnc("pts %d.%d dts %d.%d flag(%d)\n", psec, pusec, dsec, dusec, dp->pts_dts_flag);
      }
#endif
#endif
      vds = videodecoder_decode(info->vdec, m, info->p, dp->data, dp->size, &used);
      if (used != dp->size)
	warning_fnc("videodecoder_decode didn't consumed all %d bytes, but %d bytes\n", used, dp->size);
    } else {
      err_message_fnc("videodecoder_decode returned %d\n", vds);
      break;
    }
  }

 quit:
  if (dp)
    destructor(dp);

  debug_message_fnc("videodecoder_destroy() ");
  videodecoder_destroy(info->vdec);
  debug_message("OK\n");

  debug_message_fnc("exit\n");

  return (void *)(vds == VD_ERROR ? PLAY_ERROR : PLAY_OK);
}

static void *
play_audio(void *arg)
{
  Movie *m = arg;
  Libmpeg2_info *info = (Libmpeg2_info *)m->movie_private;
  AudioDevice *ad;
  AudioDecoderStatus ads;
  unsigned int used = 0, u;
  void *data;
  DemuxedPacket *dp = NULL;
  FIFO_destructor destructor;
#ifdef USE_TS
  unsigned long pts, dts, size;
#endif

  debug_message_fn("()\n");

  // XXX: should be selectable
  //info->adec = audiodecoder_create(info->eps, "mpglib");
  info->adec = audiodecoder_create(info->eps, "mad");
  if (info->adec == NULL) {
    err_message("audiodecoder plugin not found. Audio disabled.\n");
    return (void *)PLAY_OK;
  }
  if ((ad = m->ap->open_device(NULL, info->c)) == NULL) {
    err_message("Cannot open device. Audio disabled.\n");
    return (void *)PLAY_OK;
  }
  info->ad = ad;

  while (m->status == _PLAY) {
    if (!dp || dp->size == used) {
      if (!fifo_get(info->astream, &data, &destructor)) {
	debug_message_fnc("fifo_get() failed.\n");
	break;
      }
      if (dp)
	destructor(dp);
      dp = (DemuxedPacket *)data;
      used = 0;
    }
    if (ad) {
#ifdef USE_TS
      switch (dp->pts_dts_flag) {
      case 2:
	pts = dp->pts;
	dts = -1;
	size = dp->size;
	break;
      case 3:
	pts = dp->pts;
	dts = dp->dts;
	size = dp->size;
	break;
      default:
	pts = dts = -1;
	size = dp->size;
	break;
      }
#endif
      ads = audiodecoder_decode(info->adec, m, ad, dp->data + used, dp->size - used, &u);
      used += u;
      while (ads == AD_OK) {
	u = 0;
	ads = audiodecoder_decode(info->adec, m, ad, NULL, 0, &u);
	if (u > 0)
	  debug_message_fnc("u should be zero.\n");
      }
      if (ads != AD_NEED_MORE_DATA) {
	err_message_fnc("error in audio decoding.\n");
	break;
      }
    }
  }

  if (dp)
    destructor(dp);

  if (ad) {
    m->ap->sync_device(ad);
    m->ap->close_device(ad);
    info->ad = NULL;
  }

  debug_message_fnc("audiodecoder_destroy()... ");
  audiodecoder_destroy(info->adec);
  debug_message("OK. exit.\n");

  return (void *)PLAY_OK;
}

static PlayerStatus
play_main(Movie *m, VideoWindow *vw)
{
  Libmpeg2_info *info = (Libmpeg2_info *)m->movie_private;
  Image *p = info->p;
  int i;
  int video_time, audio_time;

  switch (m->status) {
  case _PLAY:
    break;
  case _RESIZING:
    {
      unsigned int dw, dh;

      video_window_calc_magnified_size(vw, info->use_xv, m->width, m->height, &dw, &dh);
      video_window_resize(vw, dw, dh);

      if (info->use_xv) {
	m->rendering_width  = m->width;
	m->rendering_height = m->height;
      } else {
	m->rendering_width  = dw;
	m->rendering_height = dh;
      }
      m->status = _PLAY;
    }
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

  if (!m->has_video)
    return PLAY_OK;

  if (demultiplexer_old_get_eof(info->demux)) {
    if (info->astream && fifo_is_empty(info->astream)) {
      /* Audio existed, but over. */
      if (m->has_audio != 2) {
	debug_message_fnc("Audio over.\n");
	m->has_audio = 2;
	fifo_invalidate(info->astream);
      }
    }
    if ((!info->vstream || fifo_is_empty(info->vstream)) &&
	(!info->astream || fifo_is_empty(info->astream))) {
      stop_movie(m);
      return PLAY_OK;
    }
  }

  if (info->vdec == NULL)
    return PLAY_OK;

  if (info->vdec->to_render == 0)
    return PLAY_OK;

  if (!info->if_initialized && m->width && m->height) {
    unsigned int dw, dh;

    image_rendered_width(p) = m->width;
    image_rendered_height(p) = m->height;
    video_window_calc_magnified_size(vw, info->use_xv, m->width, m->height, &dw, &dh);
    m->initialize_screen(vw, m, dw, dh);

    if (info->use_xv) {
      m->rendering_width  = m->width;
      m->rendering_height = m->height;
    } else {
      m->rendering_width  = dw;
      m->rendering_height = dh;
    }
    info->if_initialized++;
  }

  pthread_mutex_lock(&info->vdec->update_mutex);

  video_time = m->current_frame * 1000 / m->framerate;
  if (m->has_audio == 1 && info->ad) {
    audio_time = get_audio_time(m, info->ad);
    //debug_message("r: %d v: %d a: %d (%d frame)\n", (int)timer_get_milli(m->timer), video_time, audio_time, m->current_frame);

    /* if too fast to display, wait before render */
    while (video_time > audio_time)
      audio_time = get_audio_time(m, info->ad);

    m->render_frame(vw, m, p);

    i = (audio_time * m->framerate / 1000) - m->current_frame - 1;
  } else {
    //debug_message("r: %d v: %d a: %d (%d frame)\n", (int)timer_get_milli(m->timer), video_time, audio_time, m->current_frame);

    /* if too fast to display, wait before render */
    while (video_time > timer_get_milli(m->timer)) ;

    m->render_frame(vw, m, p);

    i = (timer_get_milli(m->timer) * m->framerate / 1000) - m->current_frame - 1;
  }

  info->vdec->to_render--;

  /* skip if delayed */
  if (i > 0) {
    info->to_skip = info->to_skip ? 0 : 1;
    debug_message(info->to_skip ? "skip on\n" : "skip off\n");
  }

  /* tell video thread to continue decoding */
  if (info->vdec->to_render == 0)
    pthread_cond_signal(&info->vdec->update_cond);
  pthread_mutex_unlock(&info->vdec->update_mutex);

  return PLAY_OK;
}

static PlayerStatus
pause_movie(Movie *m)
{
  switch (m->status) {
  case _PLAY:
  case _RESIZING:
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
    warning("Unknown status %d\n", m->status);
    return PLAY_ERROR;
  }

  return PLAY_ERROR;
}

static PlayerStatus
stop_movie(Movie *m)
{
  Libmpeg2_info *info = (Libmpeg2_info *)m->movie_private;

  debug_message_fn("()\n");

  switch (m->status) {
  case _PLAY:
  case _RESIZING:
    m->status = _STOP;
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

  timer_stop(m->timer);

  if (info->vstream)
    fifo_invalidate(info->vstream);
  if (info->video_thread) {
    info->vdec->to_render = 0;
    debug_message_fnc("waiting for joining (video).\n");
    pthread_cond_signal(&info->vdec->update_cond);
    pthread_join(info->video_thread, NULL);
    info->video_thread = 0;
    debug_message_fnc("joined (video).\n");
  }

  if (info->astream)
    fifo_invalidate(info->astream);
  if (info->audio_thread) {
    debug_message_fnc("waiting for joining (audio).\n");
    pthread_join(info->audio_thread, NULL);
    info->audio_thread = 0;
    debug_message_fnc("joined (audio).\n");
  }

  if (info->demux) {
    debug_message_fnc("waiting for demultiplexer to stop.\n");
    demultiplexer_old_stop(info->demux);
    debug_message_fnc("demultiplexer stopped\n");
  }

  if (info->vstream) {
    fifo_destroy(info->vstream);
    info->vstream = NULL;
    demultiplexer_mpeg_set_vst(info->demux, NULL);
  }
  if (info->astream) {
    fifo_destroy(info->astream);
    info->astream = NULL;
    demultiplexer_mpeg_set_ast(info->demux, NULL);
  }

  debug_message_fnc("OK\n");

  return PLAY_OK;
}

static void
unload_movie(Movie *m)
{
  Libmpeg2_info *info = (Libmpeg2_info *)m->movie_private;

  debug_message_fn("()\n");

  stop_movie(m);

  if (info) {
    if (info->p)
      image_destroy(info->p);
    if (info->demux)
      demultiplexer_old_destroy(info->demux);
    free(info);
    m->movie_private = NULL;
  }

  debug_message_fn("() Ok\n");
}

/* methods */

DEFINE_PLAYER_PLUGIN_IDENTIFY(m, st, c, eps)
{
  unsigned char buf[3];

  if (stream_read(st, buf, 3) != 3)
    return PLAY_NOT;
  if (buf[0] || buf[1] || buf[2] != 1)
    return PLAY_NOT;

  return PLAY_OK;
}

DEFINE_PLAYER_PLUGIN_LOAD(vw, m, st, c, eps)
{
  debug_message_fn("()\n");

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

  return load_movie(vw, m, st, c, eps);
}
