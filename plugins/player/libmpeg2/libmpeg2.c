/*
 * libmpeg2.c -- libmpeg2 player plugin, which exploits libmpeg2.
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Wed Sep 19 17:35:04 2001.
 * $Id: libmpeg2.c,v 1.23 2001/09/19 08:37:13 sian Exp $
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
#  error pthread is mandatory for libmpeg2 plugin
#endif

#include <pthread.h>

#include "utils/timer.h"
#include "enfle/player-plugin.h"
#include "libmpeg2_vo.h"
#include "mpeg2/mm_accel.h"

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
  type: ENFLE_PLUGIN_PLAYER,
  name: "LibMPEG2",
  description: "LibMPEG2 Player plugin version 0.2.1 with integrated libmpeg2(mpeg2dec-0.2.0)",
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
load_movie(VideoWindow *vw, Movie *m, Stream *st, Config *c)
{
  Libmpeg2_info *info;
  Image *p;

  if ((info = calloc(1, sizeof(Libmpeg2_info))) == NULL) {
    show_message("LibMPEG2: " __FUNCTION__ ": No enough memory.\n");
    return PLAY_ERROR;
  }

  info->vw = vw;
  info->c = c;

  pthread_mutex_init(&info->update_mutex, NULL);
  pthread_cond_init(&info->update_cond, NULL);

  m->requested_type = video_window_request_type(vw, types, &m->direct_decode);
  if (!m->direct_decode) {
    show_message(__FUNCTION__ ": Cannot do direct decoding...\n");
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
  if (!demultiplexer_examine(info->demux))
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
  if ((p->rendered.image = memory_create()) == NULL)
    goto error;
  memory_request_type(p->rendered.image, video_window_preferred_memory_type(vw));

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
    p->bytes_per_line = p->width * 1.5;
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
  demultiplexer_rewind(info->demux);

  if (m->has_video) {
    unsigned int accel;

    if ((info->vstream = fifo_create()) == NULL)
      return PLAY_ERROR;
    //fifo_set_max(info->vstream, 2000);
    demultiplexer_mpeg_set_vst(info->demux, info->vstream);
    accel = mm_accel();
    vo_accel(accel);
    mpeg2_init(&info->mpeg2dec, accel, info->vo);
    pthread_create(&info->video_thread, NULL, play_video, m);
  }

  if (m->has_audio) {
    if ((info->astream = fifo_create()) == NULL)
      return PLAY_ERROR;
    //fifo_set_max(info->vstream, 2000);
    demultiplexer_mpeg_set_ast(info->demux, info->astream);
    InitMP3(&info->mp);
    pthread_create(&info->audio_thread, NULL, play_audio, m);
  }

  demultiplexer_start(info->demux);

  return PLAY_OK;
}

static int
get_audio_time(Movie *m, AudioDevice *ad)
{
  if (ad && m->ap->bytes_written)
    return (int)((double)m->ap->bytes_written(ad) / m->samplerate * 500 / m->channels);
  return (int)((double)m->current_sample * 1000 / m->samplerate);
}

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

#define MPEG_VIDEO_DECODE_BUFFER_SIZE 2048
//#define MPEG_VIDEO_DECODE_BUFFER_SIZE 128

static void *
play_video(void *arg)
{
  Movie *m = arg;
  Libmpeg2_info *info = (Libmpeg2_info *)m->movie_private;
  void *data;
  MpegPacket *mp;
  //unsigned long pts, dts, size;
  int nframe_decoded;

  debug_message(__FUNCTION__ "()\n");

  while (m->status == _PLAY) {
    /* XXX: Should I use wait and singal? */
    if (fifo_get(info->vstream, &data)) {
      mp = (MpegPacket *)data;
#if 0
      switch (mp->pts_dts_flag) {
      case 2:
	pts = mp->pts;
	dts = -1;
	size = mp->size;
	break;
      case 3:
	pts = mp->pts;
	dts = mp->dts;
	size = mp->size;
	break;
      default:
	pts = dts = -1;
	size = mp->size;
	break;
      }
#endif
      nframe_decoded = mpeg2_decode_data(&info->mpeg2dec, mp->data, mp->data + mp->size);
      //debug_message(__FUNCTION__ ": %d frames decoded\n", nframe_decoded);
      free(mp->data);
      free(mp);
    }
  }

  debug_message(__FUNCTION__ ": mpeg2_close() ");
  mpeg2_close(&info->mpeg2dec);
  debug_message("OK\n");

  debug_message(__FUNCTION__ ": vo_close() ");
  vo_close(info->vo);
  debug_message("OK\n");

  debug_message(__FUNCTION__ ": exiting.\n");

  pthread_exit((void *)PLAY_OK);
}

#define MP3_DECODE_BUFFER_SIZE 16384

static void *
play_audio(void *arg)
{
  Movie *m = arg;
  Libmpeg2_info *info = (Libmpeg2_info *)m->movie_private;
  AudioDevice *ad;
  unsigned char output_buffer[MP3_DECODE_BUFFER_SIZE];
  //unsigned long pts, dts, size;
  int ret, write_size;
  int param_is_set = 0;
  void *data;
  MpegPacket *mp;

  debug_message(__FUNCTION__ "()\n");

  if ((ad = m->ap->open_device(NULL, info->c)) == NULL) {
    show_message("Cannot open device.\n");
    ExitMP3(&info->mp);
    pthread_exit((void *)PLAY_ERROR);
  }
  info->ad = ad;

  while (m->status == _PLAY) {
    /* XXX: Should I use wait and singal? */
    if (fifo_get(info->astream, &data)) {
      mp = (MpegPacket *)data;
#if 0
      switch (mp->pts_dts_flag) {
      case 2:
	pts = mp->pts;
	dts = -1;
	size = mp->size;
	break;
      case 3:
	pts = mp->pts;
	dts = mp->dts;
	size = mp->size;
	break;
      default:
	pts = dts = -1;
	size = mp->size;
	break;
      }
#endif
      ret = decodeMP3(&info->mp, mp->data, mp->size,
		      output_buffer, MP3_DECODE_BUFFER_SIZE, &write_size);
      if (!param_is_set) {
	m->sampleformat = _AUDIO_FORMAT_S16_LE;
	m->channels = info->mp.fr.stereo;
	m->samplerate = freqs[info->mp.fr.sampling_frequency];
	if (!m->ap->set_params(ad, &m->sampleformat, &m->channels, &m->samplerate))
	  show_message("Some params are set wrong.\n");
	param_is_set++;
      }
      while (ret == MP3_OK) {
	m->ap->write_device(ad, output_buffer, write_size);
	//debug_message(__FUNCTION__ ": %d bytes written.\n", write_size);
	ret = decodeMP3(&info->mp, NULL, 0,
			output_buffer, MP3_DECODE_BUFFER_SIZE, &write_size);
      }
      free(mp->data);
      free(mp);
    }
  }

  m->ap->sync_device(ad);
  m->ap->close_device(ad);
  info->ad = NULL;

  ExitMP3(&info->mp);

  debug_message(__FUNCTION__ " exiting.\n");

  pthread_exit((void *)PLAY_OK);
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
  case _PAUSE:
  case _STOP:
    return PLAY_OK;
  case _UNLOADED:
    return PLAY_ERROR;
  default:
    return PLAY_ERROR;
  }

  if (!m->has_video)
    return PLAY_OK;

  if (demultiplexer_get_eof(info->demux)) {
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
    m->initialize_screen(vw, m, m->rendering_width, m->rendering_height);
    video_window_calc_magnified_size(vw, m->width, m->height, &p->magnified.width, &p->magnified.height);

    if (info->use_xv) {
      m->rendering_width  = m->width;
      m->rendering_height = m->height;
    } else {
      m->rendering_width  = p->magnified.width;
      m->rendering_height = p->magnified.height;
    }
    info->if_initialized++;
  }

  pthread_mutex_lock(&info->update_mutex);

  video_time = m->current_frame * 1000 / m->framerate;
  if (m->has_audio == 1) {
    audio_time = get_audio_time(m, info->ad);
    // debug_message("r: %d v: %d a: %d (%d frame)\n", (int)timer_get_milli(m->timer), video_time, audio_time, m->current_frame);

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
      mpeg2_drop(&info->mpeg2dec, 1);
    } else {
      if (dropping) {
	debug_message("drop off\n");
	dropping--;
      }
      mpeg2_drop(&info->mpeg2dec, 0);
    }
  } else {
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
      mpeg2_drop(&info->mpeg2dec, 1);
    } else {
      if (dropping) {
	debug_message("drop off\n");
	dropping--;
      }
      mpeg2_drop(&info->mpeg2dec, 0);
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

  timer_stop(m->timer);

  debug_message(__FUNCTION__ ": waiting for demultiplexer thread to exit... \n");

  demultiplexer_stop(info->demux);
  if (m->has_video && info->vstream) {
    fifo_destroy(info->vstream);
    info->vstream = NULL;
  }
  if (m->has_audio && info->astream) {
    fifo_destroy(info->astream);
    info->astream = NULL;
  }

  debug_message(__FUNCTION__ ": waiting for video thread to exit... \n");

  if (info->video_thread) {
    //pthread_join(info->video_thread, NULL);
    pthread_cancel(info->video_thread);
    info->video_thread = 0;
  }

  debug_message(__FUNCTION__ ": waiting for audio thread to exit... \n");

  if (info->audio_thread) {
    pthread_join(info->audio_thread, NULL);
    info->audio_thread = 0;
  }

  debug_message(__FUNCTION__ ": OK\n");

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
    demultiplexer_destroy(info->demux);
    pthread_mutex_destroy(&info->update_mutex);
    pthread_cond_destroy(&info->update_cond);
    free(info);
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
  debug_message("libmpeg2 player: load() called\n");

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
