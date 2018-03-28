/*
 * libmpeg2.c -- libmpeg2 player plugin, which exploits libmpeg2.
 * (C)Copyright 2000-2003 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Wed Dec 28 01:15:54 2005.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <inttypes.h>

#define REQUIRE_STRING_H
#define REQUIRE_UNISTD_H
#include "compat.h"
#include "common.h"

#ifndef USE_PTHREAD
#  error pthread is mandatory for libmpeg2_old plugin
#endif

#include <pthread.h>

#include "enfle/player-plugin.h"
#define VIDEO_OUT_NEED_VO_CLOSE
#include "libmpeg2_vo.h"
#include "mpeg2.old/mm_accel.h"

static const unsigned int types =
  (IMAGE_I420 |
   IMAGE_RGBA32 | IMAGE_BGRA32 |
   IMAGE_RGB24 | IMAGE_BGR24 |
   IMAGE_BGR_WITH_BITMASK);

DECLARE_PLAYER_PLUGIN_METHODS;

static PlayerStatus play(Movie *);
static PlayerStatus pause_movie(Movie *);
static PlayerStatus stop_movie(Movie *);

static PlayerPlugin plugin = {
  .type = ENFLE_PLUGIN_PLAYER,
  .name = "LibMPEG2_OLD",
  .description = "LibMPEG2_OLD Player plugin version 0.5 with integrated libmpeg2(mpeg2dec-0.2.1)",
  .author = "Hiroshi Takekawa",

  .identify = identify,
  .load = load
};

ENFLE_PLUGIN_ENTRY(player_libmpeg2_old)
{
  PlayerPlugin *pp;

  if ((pp = (PlayerPlugin *)calloc(1, sizeof(PlayerPlugin))) == NULL)
    return NULL;
  memcpy(pp, &plugin, sizeof(PlayerPlugin));

  return (void *)pp;
}

ENFLE_PLUGIN_EXIT(player_libmpeg2_old, p)
{
  free(p);
}

/* for internal use */

static PlayerStatus
load_movie(VideoWindow *vw, Movie *m, Stream *st, Config *c)
{
  Libmpeg2_info *info;
  Image *p;

  if ((info = calloc(1, sizeof(Libmpeg2_info))) == NULL) {
    show_message("LibMPEG2: %s: No enough memory.\n", __FUNCTION__);
    return PLAY_ERROR;
  }

  info->vw = vw;
  info->c = c;

  pthread_mutex_init(&info->update_mutex, NULL);
  pthread_cond_init(&info->update_cond, NULL);

  m->requested_type = video_window_request_type(vw, types, &m->direct_decode);
  if (!m->direct_decode) {
    show_message_fnc("Cannot do direct decoding...\n");
    return PLAY_ERROR;
  }
  debug_message("LibMPEG2: requested type: %s direct\n", image_type_to_string(m->requested_type));

  if (m->requested_type == _I420) {
    info->use_xv = 1;
  } else {
    info->use_xv = 0;
  }

  info->demux = demultiplexer_mpeg_create();
  demultiplexer_mpeg_set_input(info->demux, st);
  if (!demultiplexer_old_examine(info->demux))
    return PLAY_NOT;
  info->nastreams = demultiplexer_mpeg_naudios(info->demux);
  info->nvstreams = demultiplexer_mpeg_nvideos(info->demux);
  //info->nvstreams = 0;

  if (info->nastreams == 0 && info->nvstreams == 0)
    return PLAY_NOT;

  m->has_audio = 0;
  if (info->nastreams) {
    if (m->ap == NULL)
      show_message("Audio not played.\n");
    else {
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

    if (info->use_xv) {
      if ((info->vo = vo_enfle_yuv_open(vw, m, p)) == NULL)
	goto error;
    } else {
      if ((info->vo = vo_enfle_rgb_open(vw, m, p)) == NULL)
	goto error;
    }
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
      return PLAY_ERROR;
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
    unsigned int accel;

    if ((info->vstream = fifo_create()) == NULL)
      return PLAY_ERROR;
    fifo_set_max(info->vstream, 256);
    demultiplexer_mpeg_set_vst(info->demux, info->vstream);
    accel = mm_accel();
    vo_accel(accel);
    mpeg2old_init(&info->mpeg2dec, accel, info->vo);
    pthread_create(&info->video_thread, NULL, play_video, m);
  }

  if (m->has_audio) {
    m->has_audio = 1;
    if ((info->astream = fifo_create()) == NULL)
      return PLAY_ERROR;
    fifo_set_max(info->astream, 64);
    demultiplexer_mpeg_set_ast(info->demux, info->astream);
    InitMP3(&info->mp);
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

#if 0
static inline unsigned long
get_timestamp(unsigned char *p)
{
  unsigned long t;

  t = (((((p[0] << 8) | p[1]) << 8) | p[2]) << 8) | p[3];

  return t;
}

static inline unsigned char *
get_ptr(unsigned char *p)
{
  unsigned long t;

  /* XXX: should be much more portable */
#ifdef WORDS_BIGENDIAN
  t = (((((p[0] << 8) | p[1]) << 8) | p[2]) << 8) | p[3];
#else
  t = (((((p[3] << 8) | p[2]) << 8) | p[1]) << 8) | p[0];
#endif

  return (unsigned char *)t;
}
#endif

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
  void *data;
  unsigned int old_nframe;
  int nframe_decoded;
  DemuxedPacket *dp;
  FIFO_destructor destructor;
#ifdef USE_TS
  unsigned long pts, dts, size;
#endif

  debug_message_fn("()\n");

  while (m->status == _PLAY) {
    if (!fifo_get(info->vstream, &data, &destructor)) {
      show_message_fnc("fifo_get() failed.\n");
    } else {
      dp = (DemuxedPacket *)data;
#ifdef USE_TS
      switch (dp->pts_dts_flag) {
      case 2:
	pts = dp->pts;
	dts = pts;
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
#ifdef DEBUG
      {
	unsigned int psec, pusec, dsec, dusec;
	if (pts != -1) {
	  TS_TO_CLOCK(psec, pusec, pts);
	  TS_TO_CLOCK(dsec, dusec, dts);
	  debug_message_fnc("pts %d.%d dts %d.%d\n", psec, pusec, dsec, dusec);
	}
      }
#endif
#endif
      old_nframe = m->current_frame;
      nframe_decoded = mpeg2old_decode_data(&info->mpeg2dec, dp->data, dp->data + dp->size);
      /* Be sure m->current_frame is right when dropping. */
      m->current_frame = old_nframe + nframe_decoded;
      destructor(dp);
    }
  }

  debug_message_fnc("mpeg2_close() ");
  mpeg2old_close(&info->mpeg2dec);
  debug_message("OK\n");

  debug_message_fnc("vo_close() ");
  vo_close(info->vo);
  debug_message("OK\n");

  debug_message_fnc("exiting.\n");

  return (void *)PLAY_OK;
}

#define MP3_DECODE_BUFFER_SIZE 16384

static void *
play_audio(void *arg)
{
  Movie *m = arg;
  Libmpeg2_info *info = (Libmpeg2_info *)m->movie_private;
  AudioDevice *ad;
  char output_buffer[MP3_DECODE_BUFFER_SIZE];
  int ret, write_size;
  int param_is_set = 0;
  void *data;
  DemuxedPacket *dp;
  FIFO_destructor destructor;
#ifdef USE_TS
  unsigned long pts, dts, size;
#endif

  debug_message_fn("()\n");

  if ((ad = m->ap->open_device(NULL, info->c)) == NULL) {
    err_message("Cannot open device. Audio disabled.\n");
  }
  info->ad = ad;

  while (m->status == _PLAY) {
    if (!fifo_get(info->astream, &data, &destructor)) {
      debug_message_fnc("fifo_get() failed.\n");
      break;
    }
    dp = (DemuxedPacket *)data;
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
      ret = decodeMP3(&info->mp, dp->data, dp->size,
		      output_buffer, MP3_DECODE_BUFFER_SIZE, &write_size);
      if (!param_is_set) {
	m->sampleformat = _AUDIO_FORMAT_S16_LE;
	m->channels = info->mp.fr.stereo;
	m->samplerate = freqs[info->mp.fr.sampling_frequency];
	debug_message_fnc("mp3: (%d format) %d ch %d Hz\n", m->sampleformat, m->channels, m->samplerate);
	m->sampleformat_actual = m->sampleformat;
	m->channels_actual = m->channels;
	m->samplerate_actual = m->samplerate;
	if (!m->ap->set_params(ad, &m->sampleformat_actual, &m->channels_actual, &m->samplerate_actual))
	  warning_fnc("set_params went wrong: (%d format) %d ch %d Hz\n", m->sampleformat_actual, m->channels_actual, m->samplerate_actual);
	param_is_set++;
      }
      while (ret == MP3_OK) {
	m->ap->write_device(ad, (unsigned char *)output_buffer, write_size);
	ret = decodeMP3(&info->mp, NULL, 0,
			output_buffer, MP3_DECODE_BUFFER_SIZE, &write_size);
      }
    }
    destructor(dp);
  }

  if (ad) {
    debug_message_fnc("sync_device()...");
    m->ap->sync_device(ad);
    debug_message("OK\n");
    debug_message_fnc("close_device()...");
    m->ap->close_device(ad);
    info->ad = NULL;
    debug_message("OK\n");
  }

  debug_message_fnc("ExitMP3()...");
  ExitMP3(&info->mp);
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
  static int dropping = 0;

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
    if (info->astream && fifo_is_empty(info->astream))
      /* Audio existed, but over. */
      m->has_audio = 2;
    if ((!info->vstream || fifo_is_empty(info->vstream)) &&
	(!info->astream || fifo_is_empty(info->astream))) {
      stop_movie(m);
      return PLAY_OK;
    }
  }

  if (info->to_render == 0)
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

  pthread_mutex_lock(&info->update_mutex);

  video_time = m->current_frame * 1000 / m->framerate;
  if (m->has_audio == 1 && info->ad) {
    audio_time = get_audio_time(m, info->ad);
    //debug_message("r: %d v: %d a: %d (%d frame)\n", (int)timer_get_milli(m->timer), video_time, audio_time, m->current_frame);

    /* if too fast to display, wait before render */
    while (video_time > audio_time)
      audio_time = get_audio_time(m, info->ad);

    m->render_frame(vw, m, p);
    info->to_render--;

    /* skip if delayed */
    i = (audio_time * m->framerate / 1000) - m->current_frame - 1;
    if (i > 0) {
      if (!dropping) {
	debug_message("drop on\n");
	dropping++;
      }
      mpeg2old_drop(&info->mpeg2dec, 1);
    } else {
      if (dropping) {
	debug_message("drop off\n");
	dropping--;
      }
      mpeg2old_drop(&info->mpeg2dec, 0);
    }
  } else {
    //debug_message("r: %d v: %d a: %d (%d frame)\n", (int)timer_get_milli(m->timer), video_time, audio_time, m->current_frame);

    /* if too fast to display, wait before render */
    while (video_time > timer_get_milli(m->timer)) ;

    m->render_frame(vw, m, p);
    info->to_render--;

    /* skip if delayed */
    i = (timer_get_milli(m->timer) * m->framerate / 1000) - m->current_frame - 1;
    if (i > 0) {
      if (!dropping) {
	debug_message("drop on\n");
	dropping++;
      }
      mpeg2old_drop(&info->mpeg2dec, 1);
    } else {
      if (dropping) {
	debug_message("drop off\n");
	dropping--;
      }
      mpeg2old_drop(&info->mpeg2dec, 0);
    }
  }

  /* tell video thread to continue decoding */
  if (info->to_render == 0)
    pthread_cond_signal(&info->update_cond);
  pthread_mutex_unlock(&info->update_mutex);

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
    info->to_render = 0;
    debug_message_fnc("waiting for joining (video).\n");
    pthread_mutex_lock(&info->update_mutex);
    pthread_cond_signal(&info->update_cond);
    pthread_mutex_unlock(&info->update_mutex);
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

  if (m->has_video && info->vstream) {
    fifo_destroy(info->vstream);
    info->vstream = NULL;
    demultiplexer_mpeg_set_vst(info->demux, NULL);
  }
  if (m->has_audio && info->astream) {
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
    pthread_mutex_destroy(&info->update_mutex);
    pthread_cond_destroy(&info->update_cond);

    free(info);
    m->movie_private = NULL;
  }
}

/* methods */

DEFINE_PLAYER_PLUGIN_IDENTIFY(m, st, c, priv)
{
  unsigned char buf[3];

  if (stream_read(st, buf, 3) != 3)
    return PLAY_NOT;
  if (buf[0] || buf[1] || buf[2] != 1)
    return PLAY_NOT;

  return PLAY_OK;
}

DEFINE_PLAYER_PLUGIN_LOAD(vw, m, st, c, priv)
{
  debug_message("libmpeg2_old player: load() called\n");

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

  return load_movie(vw, m, st, c);
}
