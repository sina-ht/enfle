/*
 * opendivx.c -- opendivx player plugin, which exploits libdivxdecore
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Oct  8 14:23:05 2001.
 * $Id: opendivx.c,v 1.16 2001/10/09 00:55:55 sian Exp $
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
 *
 * This plugin uses libdivxdecore developed by Project Mayo and its
 * contributors. This plugin itself doesn't include any part of codes
 * by others. I'm not sure this plugin is regarded as "Larger
 * Work". But even if so, consistent with OpenDivX-LICENSE, this
 * plugin can be and is released under the GNU General Public
 * License. The OpenDivX license can be found in the top of this
 * source tree, or
 * http://www.projectmayo.com/projects/divx_open_license.txt
 */

#include <stdio.h>
#include <stdlib.h>
#define LINUX
#include <decore.h>

#include "common.h"

#ifndef USE_PTHREAD
#  error pthread is mandatory for opendivx plugin
#endif

#include <pthread.h>

#include "utils/timer.h"
#include "enfle/player-plugin.h"
#include "demultiplexer/demultiplexer_avi.h"
#include "mpglib/mpg123.h"
#include "mpglib/mpglib.h"

typedef struct _opendivx_info {
  Config *c;
  Demultiplexer *demux;
  DEC_MEM_REQS dec_memreqs;
  DEC_PARAM dec_param;
  DEC_SET   dec_set;
  DEC_FRAME dec_frame;
  Image *p;
  AudioDevice *ad;
  FIFO *vstream, *astream;
  struct mpstr mp;
  int output_format;
  int use_xv;
  int eof;
  int drop;
  pthread_mutex_t update_mutex;
  pthread_cond_t update_cond;
  int nvstreams;
  int nvstream;
  pthread_t video_thread;
  int nastreams;
  int nastream;
  pthread_t audio_thread;
} OpenDivX_info;

static const unsigned int types =
  (IMAGE_I420 | IMAGE_UYVY | IMAGE_YUY2 |
   IMAGE_BGRA32 | IMAGE_BGR24 |
   IMAGE_ARGB32 | IMAGE_RGB24 |
   IMAGE_BGR_WITH_BITMASK | IMAGE_RGB_WITH_BITMASK);

DECLARE_PLAYER_PLUGIN_METHODS;

static PlayerStatus play(Movie *);
static PlayerStatus pause_movie(Movie *);
static PlayerStatus stop_movie(Movie *);

static PlayerPlugin plugin = {
  type: ENFLE_PLUGIN_PLAYER,
  name: "OpenDivX",
  description: "OpenDivX Player plugin version 0.3 with libdivxdecore",
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
  OpenDivX_info *info;
  AVIInfo *aviinfo;
  Image *p;

  if ((info = calloc(1, sizeof(OpenDivX_info))) == NULL) {
    show_message("OpenDivX: " __FUNCTION__ ": No enough memory.\n");
    return PLAY_ERROR;
  }

  info->c = c;

  pthread_mutex_init(&info->update_mutex, NULL);
  pthread_cond_init(&info->update_cond, NULL);

  m->requested_type = video_window_request_type(vw, types, &m->direct_decode);
  if (!m->direct_decode) {
    show_message(__FUNCTION__ ": Cannot direct decoding...\n");
    return PLAY_ERROR;
  }
  debug_message("OpenDivX: requested type: %s direct\n", image_type_to_string(m->requested_type));

  info->demux = demultiplexer_avi_create();
  demultiplexer_avi_set_input(info->demux, st);
  if (!demultiplexer_examine(info->demux))
    return PLAY_NOT;
  info->nvstreams = demultiplexer_avi_nvideos(info->demux);
  info->nastreams = demultiplexer_avi_naudios(info->demux);
  aviinfo = demultiplexer_avi_info(info->demux);

  if (info->nastreams == 0 && info->nvstreams == 0)
    return PLAY_NOT;
  if (info->nvstreams != 1)
    return PLAY_NOT;

  m->has_audio = 0;
  if (info->nastreams > 0) {
    if (m->ap == NULL)
      show_message("Audio not played.\n");
    else {
      /* XXX: stream should be selectable */
      info->nastream = 1;
      demultiplexer_avi_set_audio(info->demux, info->nastream);

      m->sampleformat = _AUDIO_FORMAT_S16_LE;
      m->channels = aviinfo->nchannels;
      m->samplerate = aviinfo->samples_per_sec;
      m->num_of_samples = aviinfo->num_of_samples;

      show_message("audio(%d streams): format(%d): %d ch rate %d kHz %d samples\n", info->nastreams, m->sampleformat, m->channels, m->samplerate, m->num_of_samples);
      if (m->ap->bytes_written == NULL)
	show_message("audio sync may be incorrect.\n");
      m->has_audio = 1;
    }
  } else {
    debug_message("No audio streams.\n");
  }

  if (info->nvstreams) {
    m->has_video = 1;
    info->nvstream = 0;
    demultiplexer_avi_set_video(info->demux, info->nvstream);

    m->width = aviinfo->width;
    m->height = aviinfo->height;
    m->framerate = aviinfo->framerate;
    m->num_of_frames = aviinfo->num_of_frames;
  } else {
    m->width = 128;
    m->height = 128;
    m->num_of_frames = 1;
  }

  p = info->p = image_create();
  video_window_calc_magnified_size(vw, m->width, m->height, &p->magnified.width, &p->magnified.height);

  if (info->use_xv) {
    m->rendering_width  = m->width;
    m->rendering_height = m->height;
  } else {
    m->rendering_width  = p->magnified.width;
    m->rendering_height = p->magnified.height;
  }

  debug_message("video(%d streams): (%d,%d) -> (%d,%d) %f fps %d frames\n", info->nvstreams,
		m->width, m->height, m->rendering_width, m->rendering_height,
		m->framerate, m->num_of_frames);

  p->width = m->rendering_width;
  p->height = m->rendering_height;
  p->type = m->requested_type;
  if ((p->rendered.image = memory_create()) == NULL)
    goto error;
  memory_request_type(p->rendered.image, video_window_preferred_memory_type(vw));

  switch (m->requested_type) {
  case _I420:
    p->bits_per_pixel = 12;
    p->bytes_per_line = p->width * 3 / 2;
    info->use_xv = 1;
    info->output_format = DEC_420;
    break;
  case _UYVY:
    p->bits_per_pixel = 16;
    p->bytes_per_line = p->width << 1;
    info->use_xv = 1;
    info->output_format = DEC_UYVY;
    break;
  case _YUY2:
    p->bits_per_pixel = 16;
    p->bytes_per_line = p->width << 1;
    info->use_xv = 1;
    info->output_format = DEC_YUY2;
    break;
  default:
    info->use_xv = 0;
    switch (vw->bits_per_pixel) {
    case 32:
      p->depth = 24;
      p->bits_per_pixel = 32;
      p->bytes_per_line = p->width * 4;
      info->output_format = (p->type == _BGRA32) ? DEC_RGB32_INV : DEC_RGB32;
      break;
    case 24:
      p->depth = 24;
      p->bits_per_pixel = 24;
      p->bytes_per_line = p->width * 3;
      info->output_format = (p->type == _BGR24) ? DEC_RGB24_INV : DEC_RGB24;
      break;
    case 16:
      p->depth = 16;
      p->bits_per_pixel = 16;
      p->bytes_per_line = p->width * 2;
      info->output_format = (p->type == _BGR_WITH_BITMASK) ? DEC_RGB565_INV : DEC_RGB565;
      break;
    case 15:
      p->depth = 15;
      p->bits_per_pixel = 16;
      p->bytes_per_line = p->width * 2;
      info->output_format = (p->type == _BGR_WITH_BITMASK) ? DEC_RGB555_INV : DEC_RGB555;
      break;
    default:
      show_message("Cannot render bpp %d\n", vw->bits_per_pixel);
      return PLAY_ERROR;
    }
  }

  if (memory_alloc(p->rendered.image, p->bytes_per_line * p->height) == NULL)
    goto error;

  m->movie_private = (void *)info;
  m->st = st;
  m->status = _STOP;

  m->initialize_screen(vw, m, m->rendering_width, m->rendering_height);

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
  OpenDivX_info *info = (OpenDivX_info *)m->movie_private;
  DEC_MEM_REQS *dec_memreqs = &info->dec_memreqs;
  DEC_PARAM *dec_param = &info->dec_param;

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
    if ((info->vstream = fifo_create()) == NULL)
      return PLAY_ERROR;
    demultiplexer_avi_set_vst(info->demux, info->vstream);

    dec_param->x_dim = m->width;
    dec_param->y_dim = m->height;
    dec_param->output_format = info->output_format;
    dec_param->time_incr = 15;

    decore((long)info, DEC_OPT_MEMORY_REQS, dec_param, dec_memreqs);

    dec_param->buffers.mp4_edged_ref_buffers = malloc(dec_memreqs->mp4_edged_ref_buffers_size);
    dec_param->buffers.mp4_edged_for_buffers = malloc(dec_memreqs->mp4_edged_for_buffers_size);
    dec_param->buffers.mp4_edged_back_buffers = malloc(dec_memreqs->mp4_edged_back_buffers_size);
    dec_param->buffers.mp4_display_buffers = malloc(dec_memreqs->mp4_display_buffers_size);
    dec_param->buffers.mp4_state = malloc(dec_memreqs->mp4_state_size);
    dec_param->buffers.mp4_tables = malloc(dec_memreqs->mp4_tables_size);
    dec_param->buffers.mp4_stream = malloc(dec_memreqs->mp4_stream_size);
    dec_param->buffers.mp4_reference = malloc(dec_memreqs->mp4_reference_size);

    memset(dec_param->buffers.mp4_state, 0, dec_memreqs->mp4_state_size);
    memset(dec_param->buffers.mp4_tables, 0, dec_memreqs->mp4_tables_size);
    memset(dec_param->buffers.mp4_stream, 0, dec_memreqs->mp4_stream_size);
    memset(dec_param->buffers.mp4_reference, 0, dec_memreqs->mp4_reference_size);

    decore((long)info, DEC_OPT_INIT, dec_param, NULL);

    info->dec_set.postproc_level = 0;
    decore((long)info, DEC_OPT_SETPP, &info->dec_set, NULL);

    pthread_create(&info->video_thread, NULL, play_video, m);
  }

  if (m->has_audio) {
    if ((info->astream = fifo_create()) == NULL)
      return PLAY_ERROR;
    demultiplexer_avi_set_ast(info->demux, info->astream);
    InitMP3(&info->mp);
    pthread_create(&info->audio_thread, NULL, play_audio, m);
  }

  demultiplexer_start(info->demux);

  return PLAY_OK;
}

static void *
play_video(void *arg)
{
  Movie *m = arg;
  OpenDivX_info *info = (OpenDivX_info *)m->movie_private;
  void *data;
  AVIPacket *ap;
  FIFO_destructor destructor;

  debug_message(__FUNCTION__ "()\n");

  while (m->status == _PLAY) {
    if (m->current_frame >= m->num_of_frames) {
      info->eof = 1;
      break;
    }
    
    if (!fifo_get(info->vstream, &data, &destructor)) {
      show_message(__FUNCTION__ ": fifo_get() failed.\n");
    } else {      
      if ((ap = (AVIPacket *)data) == NULL || ap->data == NULL) {
	info->eof = 1;
	break;
      }
      pthread_mutex_lock(&info->update_mutex);
      info->dec_frame.length = ap->size;
      info->dec_frame.bitstream = ap->data;
      info->dec_frame.bmp = memory_ptr(info->p->rendered.image);
      info->dec_frame.render_flag = 1;
      decore((long)info, DEC_OPT_FRAME, &info->dec_frame, NULL);
      destructor(ap);
      m->current_frame++;

      /* demultiplexer should seek to next key frame... */
      for (; info->drop; info->drop--) {
	if (!fifo_get(info->vstream, &data, &destructor)) {
	  show_message(__FUNCTION__ ": fifo_get() failed.\n");
	} else {
	  if ((ap = (AVIPacket *)data) == NULL || ap->data == NULL) {
	    info->eof = 1;
	    pthread_mutex_unlock(&info->update_mutex);
	    break;
	  }
	  info->dec_frame.length = ap->size;
	  info->dec_frame.bitstream = ap->data;
	  info->dec_frame.bmp = memory_ptr(info->p->rendered.image);
	  info->dec_frame.render_flag = 1;
	  decore((long)info, DEC_OPT_FRAME, &info->dec_frame, NULL);
	  destructor(ap);
	  m->current_frame++;
	}
      }
      pthread_cond_wait(&info->update_cond, &info->update_mutex);
      pthread_mutex_unlock(&info->update_mutex);
    }
  }

  debug_message(__FUNCTION__ " exiting.\n");
  pthread_exit((void *)PLAY_OK);
}

#define MP3_DECODE_BUFFER_SIZE 16384

static void *
play_audio(void *arg)
{
  Movie *m = arg;
  OpenDivX_info *info = (OpenDivX_info *)m->movie_private;
  AudioDevice *ad;
  unsigned char output_buffer[MP3_DECODE_BUFFER_SIZE];
  int ret, write_size;
  int param_is_set = 0;
  void *data;
  AVIPacket *ap;
  FIFO_destructor destructor;

  debug_message(__FUNCTION__ "()\n");

  if ((ad = m->ap->open_device(NULL, info->c)) == NULL) {
    show_message("Cannot open device.\n");
    ExitMP3(&info->mp);
    pthread_exit((void *)PLAY_ERROR);
  }
  info->ad = ad;

  while (m->status == _PLAY) {
    if (!fifo_get(info->astream, &data, &destructor)) {
      show_message(__FUNCTION__ ": fifo_get() failed.\n");
    } else {
      ap = (AVIPacket *)data;
      ret = decodeMP3(&info->mp, ap->data, ap->size,
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
	ret = decodeMP3(&info->mp, NULL, 0,
			output_buffer, MP3_DECODE_BUFFER_SIZE, &write_size);
      }
      destructor(ap);
    }
  }

  m->ap->sync_device(ad);
  m->ap->close_device(ad);
  info->ad = NULL;

  ExitMP3(&info->mp);

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
  OpenDivX_info *info = (OpenDivX_info *)m->movie_private;
  Image *p = info->p;
  int video_time, audio_time;
  int i = 0;

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

  if (!m->has_video)
    return PLAY_OK;

  pthread_mutex_lock(&info->update_mutex);

  video_time = m->current_frame * 1000 / m->framerate;
  if (m->has_audio) {
    audio_time = get_audio_time(m, info->ad);
    /* if too fast to display, wait before render */
    while (video_time > audio_time)
      audio_time = get_audio_time(m, info->ad);

    /* skip if delayed */
    i = (get_audio_time(m, info->ad) * m->framerate / 1000) - m->current_frame - 1;
    if (i > 0) {
      debug_message("dropped %d frames(v: %d a: %d)\n", i, video_time, audio_time);
      info->drop = i;
    }
  } else {
    /* if too fast to display, wait before render */
    while (video_time > timer_get_milli(m->timer)) ;

    /* skip if delayed */
    i = (timer_get_milli(m->timer) * m->framerate / 1000) - m->current_frame - 1;
    if (i > 0) {
      debug_message("dropped %d frames(v: %d t: %d)\n", i, video_time, (int)timer_get_milli(m->timer));
      info->drop = i;
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
  OpenDivX_info *info = (OpenDivX_info *)m->movie_private;
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

  decore((long)info, DEC_OPT_RELEASE, NULL, NULL);

  free(info->dec_param.buffers.mp4_edged_ref_buffers);
  free(info->dec_param.buffers.mp4_edged_for_buffers);
  free(info->dec_param.buffers.mp4_edged_back_buffers);
  free(info->dec_param.buffers.mp4_display_buffers);
  free(info->dec_param.buffers.mp4_state);
  free(info->dec_param.buffers.mp4_tables);
  free(info->dec_param.buffers.mp4_stream);
  free(info->dec_param.buffers.mp4_reference);

  return PLAY_OK;
}

static void
unload_movie(Movie *m)
{
  OpenDivX_info *info = (OpenDivX_info *)m->movie_private;

  debug_message(__FUNCTION__ "()\n");

  stop_movie(m);

  if (info) {
    if (info->p)
      image_destroy(info->p);
    demultiplexer_destroy(info->demux);
    pthread_mutex_destroy(&info->update_mutex);
    pthread_cond_destroy(&info->update_cond);
    debug_message(__FUNCTION__ ": freeing info\n");
    free(info);
    debug_message(__FUNCTION__ ": all Ok\n");
  }
}

/* methods */

DEFINE_PLAYER_PLUGIN_IDENTIFY(m, st, c, priv)
{
  unsigned char buf[16];

  if (stream_read(st, buf, 16) != 16)
    return PLAY_NOT;

  if (memcmp(buf, "RIFF", 4))
    return PLAY_NOT;
  if (memcmp(buf + 8, "AVI ", 4))
    return PLAY_NOT;

  return PLAY_OK;
}

DEFINE_PLAYER_PLUGIN_LOAD(vw, m, st, c, priv)
{
  debug_message("OpenDivX: " __FUNCTION__ "() called\n");

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
