/*
 * opendivx.c -- opendivx player plugin, which exploits libdivxdecore
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Jan 23 09:23:17 2001.
 * $Id: opendivx.c,v 1.1 2001/01/23 08:08:23 sian Exp $
 *
 *  Requires libdivxdecore
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
#include "divxdecore/decore.h"
#include "libavi.h"

#include "common.h"

#ifndef USE_PTHREAD
#  error pthread is mandatory for opendivx plugin
#endif

#include <pthread.h>

#include "utils/timer.h"
#include "enfle/player-plugin.h"

typedef struct _opendivx_info {
  RIFF_File *rf;
  RIFF_Chunk *rc;
  AviFileReader *afr;
  DEC_PARAM dec_param;
  DEC_SET   dec_set;
  DEC_FRAME dec_frame;
  Image *p;
  AudioDevice *ad;
  int eof;
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
  (IMAGE_ARGB32 | IMAGE_RGB24 | IMAGE_RGB_WITH_BITMASK);

static PlayerStatus identify(Movie *, Stream *);
static PlayerStatus load(VideoWindow *, Movie *, Stream *);

static PlayerStatus play(Movie *);
static PlayerStatus pause_movie(Movie *);
static PlayerStatus stop_movie(Movie *);

static PlayerPlugin plugin = {
  type: ENFLE_PLUGIN_PLAYER,
  name: "OpenDivX",
  description: "OpenDivX Player plugin version 0.1",
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
input_func(void *arg, void *ptr, int size)
{
  Stream *st = (Stream *)arg;

  return stream_read(st, ptr, size);
}

static int
seek_func(void *arg, unsigned int pos, int whence)
{
  Stream *st = (Stream *)arg;

  return stream_seek(st, pos, whence);
}

static PlayerStatus
load_movie(VideoWindow *vw, Movie *m, Stream *st)
{
  OpenDivX_info *info;
  Image *p;

  if ((info = calloc(1, sizeof(OpenDivX_info))) == NULL) {
    show_message("OpenDivX: load_movie: No enough memory.\n");
    return PLAY_ERROR;
  }

  info->rf = riff_file_create();
  riff_file_set_func_input(info->rf, input_func);
  riff_file_set_func_seek(info->rf, seek_func);
  riff_file_set_func_arg(info->rf, (void *)st);
  if (!riff_file_open(info->rf)) {
    show_message("riff_file_open() failed: %s\n", riff_file_get_errmsg(info->rf));
    return PLAY_ERROR;
  }

  info->afr = avifilereader_create();
  if (!avifilereader_open(info->afr, info->rf)) {
    show_message("avifilereader_open() failed.\n");
    return PLAY_ERROR;
  }

  pthread_mutex_init(&info->update_mutex, NULL);
  pthread_cond_init(&info->update_cond, NULL);

  m->requested_type = video_window_request_type(vw, types, &m->direct_decode);
  if (!m->direct_decode) {
    show_message(__FUNCTION__ ": Cannot direct decoding...\n");
    return PLAY_ERROR;
  }
  debug_message("OpenDivX: requested type: %s direct\n", image_type_to_string(m->requested_type));

  /* can set how many CPUs you want to use. */
  /* mpeg3_set_cpus(info->file, 2); */
  /* mpeg3_set_mmx(info->file, 1); */

  m->has_audio = 0;
  if (info->afr->header.dwStreams == 2) {
    if (m->ap == NULL)
      show_message("Audio not played.\n");
    else {
      info->nastreams = info->afr->header.dwStreams - 1;
      /* XXX: stream should be selectable */
      info->nastream = 0;

      m->sampleformat = _AUDIO_FORMAT_S16_LE;
      m->channels = info->afr->aformat.nChannels;
      m->samplerate = info->afr->aformat.nSamplesPerSec;
      m->num_of_samples = info->afr->aformat.cbSize;

      show_message("audio(%d streams): format(%d): %d ch rate %d kHz %d samples\n", info->nastreams, m->sampleformat, m->channels, m->samplerate, m->num_of_samples);
      if (m->ap->bytes_written == NULL)
	show_message("audio sync may be incorrect.\n");
      m->has_audio = 1;
      show_message("Audio support is not yet implemented.\n");
    }
  } else {
    debug_message("No audio streams.\n");
  }

  m->has_video = 1;
  info->nvstreams = 1;
  info->nvstream = 0;

  m->width = info->afr->vformat.biWidth;
  m->height = info->afr->vformat.biHeight;
  m->framerate = 1000000 / info->afr->header.dwMicroSecPerFrame;
  m->num_of_frames = info->afr->vsheader.dwLength;

  switch (vw->render_method) {
  case _VIDEO_RENDER_NORMAL:
    m->rendering_width  = m->width;
    m->rendering_height = m->height;
    break;
  case _VIDEO_RENDER_MAGNIFY_DOUBLE:
    m->rendering_width  = m->width  << 1;
    m->rendering_height = m->height << 1;
    break;
  default:
    m->rendering_width  = m->width;
    m->rendering_height = m->height;
    break;
  }

  debug_message("video(%d streams): (%d,%d) -> (%d,%d) %f fps %d frames\n", info->nvstreams,
		m->width, m->height, m->rendering_width, m->rendering_height,
		m->framerate, m->num_of_frames);

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
  free(info);
  return PLAY_ERROR;
}

static void *play_video(void *);
#if 0
static void *play_audio(void *);
#endif

static PlayerStatus
play(Movie *m)
{
  OpenDivX_info *info = (OpenDivX_info *)m->movie_private;

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
  info->eof = 0;

  info->dec_param.x_dim = m->width;
  info->dec_param.y_dim = m->height;
  info->dec_param.color_depth = info->p->bits_per_pixel;
  decore((long)info, DEC_OPT_INIT, &info->dec_param, NULL);

  info->dec_set.postproc_level = 0;
  decore((long)info, DEC_OPT_SETPP, &info->dec_set, NULL);

  if (m->has_video)
    pthread_create(&info->video_thread, NULL, play_video, m);
#if 0
  if (m->has_audio)
    pthread_create(&info->audio_thread, NULL, play_audio, m);
#endif

  return PLAY_OK;
}

static void *
play_video(void *arg)
{
  Movie *m = arg;
  OpenDivX_info *info = (OpenDivX_info *)m->movie_private;

  debug_message(__FUNCTION__ "()\n");

  while (m->status == _PLAY) {
    if (m->current_frame >= m->num_of_frames) {
      info->eof = 1;
      break;
    }

    pthread_mutex_lock(&info->update_mutex);

    info->rc = info->afr->vchunks[m->current_frame];
    riff_file_read_data(info->rf, info->rc);
    info->dec_frame.length = riff_chunk_get_size(info->rc);
    info->dec_frame.bitstream = riff_chunk_get_data(info->rc);
    info->dec_frame.bmp = memory_ptr(info->p->rendered.image);
    info->dec_frame.render_flag = 1;
    decore((long)info, 0, &info->dec_frame, NULL);
    riff_chunk_destroy(info->rc);
    m->current_frame++;
    //memcpy(memory_ptr(info->p->rendered.image), info->rgb, info->p->bytes_per_line * info->p->height);
    pthread_cond_wait(&info->update_cond, &info->update_mutex);
    pthread_mutex_unlock(&info->update_mutex);
  }

  debug_message(__FUNCTION__ " exiting.\n");
  pthread_exit((void *)PLAY_OK);
}

#if 0
#define AUDIO_READ_SIZE 2048
#define AUDIO_WRITE_SIZE 4096

static void *
play_audio(void *arg)
{
  Movie *m = arg;
  OpenDivX_info *info = (OpenDivX_info *)m->movie_private;
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
    mpeg3_read_audio(info->file, NULL, input_buffer, 0, samples_to_read, info->nastream);
    if (m->channels == 1) {
      m->ap->write_device(ad, (unsigned char *)input_buffer, samples_to_read * sizeof(short));
    } else {
      int i;

      mpeg3_reread_audio(info->file, NULL, input_buffer + AUDIO_READ_SIZE, 1, samples_to_read, info->nastream);
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
#endif

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
#if 0
  int i = 0;
#endif

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
    //debug_message("v: %d a=%d (%d frame)\n", (int)timer_get_milli(m->timer), audio_time, m->current_frame);

    /* if too fast to display, wait before render */
    while (video_time > audio_time)
      audio_time = get_audio_time(m, info->ad);

#if 0
    /* skip if delayed */
    i = (audio_time * m->framerate / 1000) - m->current_frame - 1;
    if (i > 0) {
      debug_message("dropped %d frames\n", i);
      m->current_frame += i;
    }
#endif
  } else {
    /* if too fast to display, wait before render */
    while (video_time > timer_get_milli(m->timer)) ;

#if 0
    /* skip if delayed */
    i = (timer_get_milli(m->timer) * m->framerate / 1000) - m->current_frame - 1;
    if (i > 0) {
      debug_message("dropped %d frames\n", i);
      m->current_frame += i;
    }
#endif
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

  return PLAY_OK;
}

static void
unload_movie(Movie *m)
{
  OpenDivX_info *info = (OpenDivX_info *)m->movie_private;

  debug_message(__FUNCTION__ "()\n");

  stop_movie(m);

  if (info) {
    if (info->afr)
      avifilereader_destroy(info->afr);
    if (info->p)
      image_destroy(info->p);

    pthread_mutex_destroy(&info->update_mutex);
    pthread_cond_destroy(&info->update_cond);

    debug_message(__FUNCTION__ ": closing opendivx file\n");
    /* close */
    riff_file_destroy(info->rf);

    debug_message(__FUNCTION__ ": freeing info\n");
    free(info);
    debug_message(__FUNCTION__ ": all Ok\n");
  }
}

/* methods */

static PlayerStatus
identify(Movie *m, Stream *st)
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

static PlayerStatus
load(VideoWindow *vw, Movie *m, Stream *st)
{
  debug_message("opendivx player: load() called\n");

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