/*
 * generic.c -- generic player plugin
 * (C)Copyright 2000-2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Apr 30 04:09:59 2004.
 * $Id: generic.c,v 1.15 2004/04/29 19:13:38 sian Exp $
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
#  error pthread is mandatory for generic plugin
#endif

#include <pthread.h>

#include "enfle/player-plugin.h"
#include "enfle/demultiplexer.h"
#include "enfle/audiodecoder.h"
#include "enfle/videodecoder.h"
#include "enfle/fourcc.h"

typedef struct __generic_info {
  EnflePlugins *eps;
  Image *p;
  Config *c;
  AudioDevice *ad;
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
} generic_info;

DECLARE_PLAYER_PLUGIN_METHODS;

static PlayerStatus play(Movie *);
static PlayerStatus pause_movie(Movie *);
static PlayerStatus stop_movie(Movie *);

static PlayerPlugin plugin = {
  .type = ENFLE_PLUGIN_PLAYER,
  .name = "generic",
  .description = "generic Player plugin version 0.2.1",
  .author = "Hiroshi Takekawa",

  .identify = identify,
  .load = load
};

ENFLE_PLUGIN_ENTRY(player_generic)
{
  PlayerPlugin *pp;

  if ((pp = (PlayerPlugin *)calloc(1, sizeof(PlayerPlugin))) == NULL)
    return NULL;
  memcpy(pp, &plugin, sizeof(PlayerPlugin));

  return (void *)pp;
}

ENFLE_PLUGIN_EXIT(player_generic, p)
{
  free(p);
}

/* for internal use */

static PlayerStatus
load_movie(VideoWindow *vw, Movie *m, Stream *st, Config *c, EnflePlugins *eps)
{
  generic_info *info = (generic_info *)m->movie_private;
  Image *p;
  unsigned int types;

  if (info)
    err_message("info should be NULL.\n");

  if ((info = calloc(1, sizeof(*info))) == NULL) {
    err_message("%s: No enough memory.\n", __FUNCTION__);
    return PLAY_ERROR;
  }
  m->movie_private = (void *)info;

  demultiplexer_set_input(m->demux, st);
  info->nvstreams = demultiplexer_nvideos(m->demux);
  info->nastreams = demultiplexer_naudios(m->demux);

  info->eps = eps;
  info->c = c;

  if (info->nastreams == 0 && info->nvstreams == 0) {
    debug_message_fnc("No audio, No video...\n");
    return PLAY_NOT;
  }
  if (info->nvstreams > 1)
    warning_fnc("Video: %d streams, only the first stream will be played.\n", info->nvstreams);

  m->has_audio = 0;
  if (info->nastreams > 0) {
    if (m->ap == NULL)
      warning("Audio will not be played.\n");
    else {
      /* XXX: stream should be selectable */
      info->nastream = m->demux->astreams[0];
      demultiplexer_set_audio(m->demux, info->nastream);

      show_message("audio(%d streams)\n", info->nastreams);

      if (m->ap->bytes_written == NULL)
	warning("audio sync may be incorrect.\n");

      if ((m->a_codec_name = audiodecoder_codec_name(m->a_fourcc)) == NULL) {
	show_message("No audiodecoder for %X\n", m->a_fourcc);
	warning("Audio will not be played.\n");
      } else {
	m->has_audio = 1;
      }
    }
  } else {
    debug_message("No audio streams.\n");
  }

  m->has_video = 0;
  if (info->nvstreams > 0) {
    info->nvstream = m->demux->vstreams[0];
    demultiplexer_set_video(m->demux, info->nvstream);

    if ((m->v_codec_name = videodecoder_codec_name(m->v_fourcc)) == NULL) {
      show_message("No videodecoder for %X\n", m->v_fourcc);
      warning("Video will not be played.\n");
    } else {
      m->has_video = 1;
      if (videodecoder_query(info->eps, m, m->v_fourcc, &types, info->c) == 0) {
	show_message("videodecoder for %s not found\n", m->v_codec_name);
	return PLAY_NOT;
      }
    }
  } else {
    debug_message("No video streams.\n");
  }
  if (!m->has_video) {
    m->width = 128;
    m->height = 128;
    m->num_of_frames = 0;
    types =
      IMAGE_RGB_WITH_BITMASK | IMAGE_BGR_WITH_BITMASK |
      IMAGE_RGB24 | IMAGE_BGR24 |
      IMAGE_RGBA32 | IMAGE_ABGR32 |
      IMAGE_ARGB32 | IMAGE_BGRA32;
  }

  m->requested_type = video_window_request_type(vw, types, &m->direct_decode);
  if (!m->direct_decode) {
    err_message_fnc("Cannot direct decoding...\n");
    return PLAY_ERROR;
  }
  debug_message("requested type: %s direct\n", image_type_to_string(m->requested_type));

  p = info->p = image_create();
  image_width(p) = m->width;
  image_height(p) = m->height;

  switch (m->requested_type) {
  case _I420:
  case _YV12:
    p->bits_per_pixel = 12;
    image_bpl(p) = image_width(p) * 3 / 2;
    info->use_xv = 1;
    m->out_fourcc = FCC_I420;
    m->out_bitcount = 0;
    break;
  case _UYVY:
    p->bits_per_pixel = 16;
    image_bpl(p) = image_width(p) << 1;
    info->use_xv = 1;
    m->out_fourcc = FCC_UYVY;
    m->out_bitcount = 0;
    break;
  case _YUY2:
    p->bits_per_pixel = 16;
    image_bpl(p) = image_width(p) << 1;
    info->use_xv = 1;
    m->out_fourcc = FCC_YUY2;
    m->out_bitcount = 0;
    break;
  default:
    info->use_xv = 0;
    switch (m->requested_type) {
    case _RGB24:
    case _BGR24:
      p->depth = 24;
      p->bits_per_pixel = 24;
      image_bpl(p) = image_width(p) * 3;
      m->out_fourcc = FCC_RGB2;
      m->out_bitcount = 24;
      break;
    case _RGBA32:
    case _ABGR32:
    case _ARGB32:
    case _BGRA32:
      p->depth = 24;
      p->bits_per_pixel = 32;
      image_bpl(p) = image_width(p) * 4;
      m->out_fourcc = (p->type == _BGRA32) ? 0 : FCC_ABGR;
      m->out_fourcc = FCC_RGB2;
      m->out_bitcount = 32;
      break;
    default:
      err_message("Cannot render type %d\n", m->requested_type);
      return PLAY_ERROR;
    }
  }

  if (m->v_codec_name)
    show_message("video[%c%c%c%c(%08X):codec %s](%d streams)",
		  m->v_fourcc        & 0xff,
		 (m->v_fourcc >>  8) & 0xff,
		 (m->v_fourcc >> 16) & 0xff,
		 (m->v_fourcc >> 24) & 0xff,
		  m->v_fourcc,
		  m->v_codec_name,
		 info->nvstreams);
  else
    show_message("video");
  if (image_width(p) > 0)
    show_message(": (%d,%d) %f fps %d frames",
		 m->width, m->height, m->framerate, m->num_of_frames);
  show_message("\n");

  p->type = m->requested_type;

  if (info->use_xv) {
    if ((image_rendered_image(p) = memory_create()) == NULL) {
      err_message("No enough memory for image object.\n");
      goto error;
    }
    memory_request_type(image_rendered_image(p), video_window_preferred_memory_type(vw));
  } else {
    if ((image_image(p) = memory_create()) == NULL) {
      err_message("No enough memory for image object.\n");
      goto error;
    }
    memory_request_type(image_image(p), video_window_preferred_memory_type(vw));
  }

  m->st = st;
  m->status = _STOP;

  return play(m);

 error:
  if (m->demux) {
    demultiplexer_destroy(m->demux);
    m->demux = NULL;
  }
  free(info);
  m->movie_private = NULL;
  return PLAY_ERROR;
}

static void *play_video(void *);
static void *play_audio(void *);

static PlayerStatus
play(Movie *m)
{
  generic_info *info = (generic_info *)m->movie_private;

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
  demultiplexer_set_eof(m->demux, 0);
  demultiplexer_rewind(m->demux);

  if (m->has_video) {
    if ((info->vstream = fifo_create()) == NULL)
      return PLAY_ERROR;
    fifo_set_max(info->vstream, 8192);
    demultiplexer_set_vst(m->demux, info->vstream);
    pthread_create(&info->video_thread, NULL, play_video, m);
  }

  if (m->has_audio) {
    m->has_audio = 1;
    if ((info->astream = fifo_create()) == NULL)
      return PLAY_ERROR;
    fifo_set_max(info->astream, 128);
    demultiplexer_set_ast(m->demux, info->astream);
    pthread_create(&info->audio_thread, NULL, play_audio, m);
  }

  demultiplexer_start(m->demux);

  return PLAY_OK;
}

static int
get_audio_time(Movie *m, AudioDevice *ad)
{
  if (ad && m->ap->bytes_written)
    return (int)((double)m->ap->bytes_written(ad) / m->samplerate * 500 / m->channels);
  return (int)((double)m->current_sample * 1000 / m->samplerate);
}

#define TS_TO_CLOCK(sec, usec, ts, ts_base) \
  sec  =  ts / ts_base; \
  usec = (ts % ts_base) * (1000000 / ts_base);

static void *
play_video(void *arg)
{
  Movie *m = arg;
  generic_info *info = (generic_info *)m->movie_private;
  VideoDecoderStatus vds;
  void *data;
  DemuxedPacket *dp = NULL;
  FIFO_destructor destructor;
  int used;
  int is_key;

  debug_message_fn("()\n");

  if (videodecoder_select(info->eps, m, m->v_fourcc, info->c) == 0) {
    show_message("videodecoder for %s not found\n", m->v_codec_name);
    demultiplexer_destroy(m->demux);
    return PLAY_NOT;
  }

  if (m->vdec == NULL) {
    warning_fnc("videodecoder plugin not found.\n");
    return (void *)VD_ERROR;
  }
  if (!videodecoder_setup(m->vdec, m, info->p, m->width, m->height)) {
    err_message_fnc("videodecoder_setup() failed.\n");
    return (void *)VD_ERROR;
  }

  show_message("videodecoder %s\n", m->vdec->name);

  vds = VD_NEED_MORE_DATA;
  is_key = 1;
  while (m->status == _PLAY || m->status == _RESIZING) {
    while ((m->status == _PLAY || m->status == _RESIZING) && vds == VD_OK)
      vds = videodecoder_decode(m->vdec, m, info->p, NULL, 0, is_key, NULL);
    if (vds == VD_NEED_MORE_DATA) {
      if (!fifo_get(info->vstream, &data, &destructor)) {
	debug_message_fnc("fifo_get() failed.\n");
	goto quit;
      }
      if (dp)
	destructor(dp);
      dp = (DemuxedPacket *)data;

#if 0
      {
	unsigned long pts, dts, psec, pusec, dsec, dusec;
	pts = dp->pts;
	dts = dp->dts;
	if (pts != -1) {
	  TS_TO_CLOCK(psec, pusec, pts, dp->ts_base);
	  TS_TO_CLOCK(dsec, dusec, dts, dp->ts_base);
	  debug_message_fnc("pts %ld.%ld dts %ld.%ld\n", psec, pusec, dsec, dusec);
	}
      }
#endif

      is_key = dp->is_key;
      vds = videodecoder_decode(m->vdec, m, info->p, dp->data, dp->size, is_key, &used);
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
  videodecoder_destroy(m->vdec);
  debug_message("OK\n");

  debug_message_fnc("exit\n");

  return (void *)(vds == VD_ERROR ? PLAY_ERROR : PLAY_OK);
}

static void *
play_audio(void *arg)
{
  Movie *m = arg;
  generic_info *info = (generic_info *)m->movie_private;
  AudioDecoderStatus ads;
  void *data;
  DemuxedPacket *dp = NULL;
  FIFO_destructor destructor;
  int used;
  AudioDevice *ad;

  debug_message_fn("()\n");

  if (audiodecoder_select(info->eps, m, m->a_fourcc, info->c) == 0) {
    show_message("audiodecoder for %s not found\n", m->a_codec_name);
    m->has_audio = 0;
    return (void *)PLAY_NOT;
  }

  if (m->adec == NULL) {
    err_message("audiodecoder plugin not found. Audio disabled.\n");
    m->has_audio = 0;
    return (void *)PLAY_OK;
  }
  if (!audiodecoder_setup(m->adec, m)) {
    err_message_fnc("audiodecoder_setup() failed.\n");
    return (void *)VD_ERROR;
  }
  if ((ad = m->ap->open_device(NULL, info->c)) == NULL) {
    err_message("Cannot open device. Audio disabled.\n");
    audiodecoder_destroy(m->adec);
    m->has_audio = 0;
    return (void *)PLAY_OK;
  }
  info->ad = ad;

  show_message("audiodecoder %s\n", m->adec->name);

  ads = AD_OK;
  while (m->status == _PLAY || m->status == _RESIZING) {
    while ((m->status == _PLAY || m->status == _RESIZING) && ads == AD_OK)
      ads = audiodecoder_decode(m->adec, m, ad, NULL, 0, NULL);
    if (ads == AD_NEED_MORE_DATA) {
      if (!fifo_get(info->astream, &data, &destructor)) {
	debug_message_fnc("fifo_get() failed.\n");
	goto quit;
      }
      if (dp)
	destructor(dp);
      dp = (DemuxedPacket *)data;

      ads = audiodecoder_decode(m->adec, m, ad, dp->data, dp->size, &used);
      if (used != dp->size)
	warning_fnc("audiodecoder_decode didn't consumed all %d bytes, but %d bytes\n", used, dp->size);
    } else {
      err_message_fnc("audiodecoder_decode returned %d\n", ads);
      break;
    }
  }

 quit:
  if (dp)
    destructor(dp);

  if (ad) {
    m->ap->sync_device(ad);
    m->ap->close_device(ad);
    info->ad = NULL;
  }

  debug_message_fnc("audiodecoder_destroy()... ");
  audiodecoder_destroy(m->adec);
  debug_message("OK. exit.\n");

  return (void *)PLAY_OK;
}

static PlayerStatus
play_main(Movie *m, VideoWindow *vw)
{
  generic_info *info = (generic_info *)m->movie_private;
  Image *p = info->p;
  int video_time, diff_time;
  int i = 0;

  switch (m->status) {
  case _PLAY:
    break;
  case _RESIZING:
    video_window_resize(vw, m->rendering_width, m->rendering_height);
    video_window_calc_magnified_size(vw, info->use_xv, m->width, m->height, &m->rendering_width, &m->rendering_height);

    if (info->use_xv) {
      image_rendered_width(p) = m->width;
      image_rendered_height(p) = m->height;
    } else {
      image_rendered_width(p) = m->rendering_width;
      image_rendered_height(p) = m->rendering_height;
    }
    m->status = _PLAY;
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

  if (demultiplexer_get_eof(m->demux)) {
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

  if (m->vdec == NULL)
    return PLAY_OK;

  if (m->vdec->to_render == 0)
    return PLAY_OK;

  pthread_mutex_lock(&m->vdec->update_mutex);

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

  video_time = m->current_frame * 1000 / m->framerate;
  if (m->has_audio == 1 && info->ad) {
    //debug_message("%d (v: %d a: %d)\n", m->current_frame, video_time, get_audio_time(m, info->ad));
    if ((diff_time = video_time - get_audio_time(m, info->ad)) >= 0) {
      /* if too fast to display, wait before render */
#if 1
      while (diff_time > 0) {
	struct timeval tv;

	tv.tv_sec = diff_time / 1000;
	tv.tv_usec = (diff_time % 1000) * 1000;
	select(0, NULL, NULL, NULL, &tv);
	diff_time = video_time - get_audio_time(m, info->ad);
	if (diff_time <= 1)
	  break;
      }
#else
      int audio_time = get_audio_time(m, info->ad);

      while (video_time > audio_time) {
	if (timer_get_milli(m->timer) > video_time + 10 * 1000 / m->framerate) {
	  warning_fnc("might have bad audio: %d (r: %d v: %d a: %d)\n", m->current_frame, (int)timer_get_milli(m->timer), video_time, audio_time);
	  break;
	}
	audio_time = get_audio_time(m, info->ad);
      }
#endif
    } else {
      /* skip if delayed */
      i = (get_audio_time(m, info->ad) * m->framerate / 1000) - m->current_frame - 1;
      if (i > 0) {
	//debug_message("dropped %d frames(v: %d a: %d)\n", i, video_time, audio_time);
	info->to_skip = i;
      }
    }
  } else {
    //debug_message("%d (r: %d v: %d)\n", m->current_frame, (int)timer_get_milli(m->timer), video_time);
    if ((diff_time = video_time - timer_get_milli(m->timer)) >= 0) {
      /* if too fast to display, wait before render */
#if 1
      while (diff_time > 0) {
	struct timeval tv;

	tv.tv_sec = diff_time / 1000;
	tv.tv_usec = (diff_time % 1000) * 1000;
	select(0, NULL, NULL, NULL, &tv);
	diff_time = video_time - timer_get_milli(m->timer);
	if (diff_time <= 1)
	  break;
      }
#else
      while (video_time > timer_get_milli(m->timer)) ;
#endif
    } else {
      /* skip if delayed */
      i = (timer_get_milli(m->timer) * m->framerate / 1000) - m->current_frame - 1;
      if (i > 0) {
	//debug_message("dropped %d frames(v: %d t: %d)\n", i, video_time, (int)timer_get_milli(m->timer));
	info->to_skip = i;
      }
    }
  }

  m->render_frame(vw, m, p);
  m->vdec->to_render--;

  /* tell video thread to continue decoding */
  if (m->vdec->to_render == 0)
    pthread_cond_signal(&m->vdec->update_cond);
  pthread_mutex_unlock(&m->vdec->update_mutex);

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
  generic_info *info = (generic_info *)m->movie_private;

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
    m->vdec->to_render = 0;
    debug_message_fnc("waiting for joining (video).\n");
    pthread_cond_signal(&m->vdec->update_cond);
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

  if (m->demux) {
    debug_message_fnc("waiting for demultiplexer to stop.\n");
    demultiplexer_stop(m->demux);
    debug_message_fnc("demultiplexer stopped\n");
  }

  if (info->vstream) {
    fifo_destroy(info->vstream);
    info->vstream = NULL;
    demultiplexer_set_vst(m->demux, NULL);
  }
  if (info->astream) {
    fifo_destroy(info->astream);
    info->astream = NULL;
    demultiplexer_set_ast(m->demux, NULL);
  }

  debug_message_fnc("OK\n");

  return PLAY_OK;
}

static void
unload_movie(Movie *m)
{
  generic_info *info = (generic_info *)m->movie_private;

  debug_message_fn("()\n");

  stop_movie(m);

  if (info) {
    if (info->p)
      image_destroy(info->p);
    if (m->demux)
      demultiplexer_destroy(m->demux);
    free(info);
    m->movie_private = NULL;
  }

  debug_message_fn("() Ok\n");
}

/* methods */

DEFINE_PLAYER_PLUGIN_IDENTIFY(m, st, c, eps)
{
  return PLAY_NOT;
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
