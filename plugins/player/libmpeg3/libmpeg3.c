/*
 * libmpeg3.c -- libmpeg3 player plugin, which exploits libmpeg3.
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Jan 12 20:56:46 2001.
 * $Id: libmpeg3.c,v 1.20 2001/01/12 13:19:37 sian Exp $
 *
 * NOTES: 
 *  This plugin is not fully enfle plugin compatible, because stream
 *  cannot be used for input. Stream's path is used as input filename.
 *
 *  Requires libmpeg3 1.2.1 (or later).
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
#include <mpeg3/libmpeg3.h>
// pthread.h is included in mpeg3/video/slice.h

#include "common.h"

#ifndef USE_PTHREAD
#  error pthread is mandatory for libmpeg3 plugin
#endif

#include "utils/timer.h"
#include "utils/timer_gettimeofday.h"
#include "enfle/player-plugin.h"

typedef struct _libmpeg3_info {
  mpeg3_t *file;
  Image *p;
  unsigned char **lines;
  int nvstreams;
  int nvstream;
  int frametime;
  int rendering_type;
  pthread_mutex_t update_mutex;
  pthread_cond_t update_cond;
  pthread_t video_thread;
  pthread_t audio_thread;
  int nastreams;
  int nastream;
} LibMPEG3_info;

static const unsigned int types =
  (IMAGE_RGBA32 | IMAGE_BGRA32 | IMAGE_RGB24 | IMAGE_BGR24 | IMAGE_BGR_WITH_BITMASK);

static PlayerStatus identify(Movie *, Stream *);
static PlayerStatus load(VideoWindow *, Movie *, Stream *);

static PlayerStatus play(Movie *);
static PlayerStatus pause_movie(Movie *);
static PlayerStatus stop_movie(Movie *);

static PlayerPlugin plugin = {
  type: ENFLE_PLUGIN_PLAYER,
  name: "LibMPEG3",
  description: "LibMPEG3 Player plugin version 0.3.3",
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
  LibMPEG3_info *info;
  Image *p;
  int i, Bpp;

  if ((info = calloc(1, sizeof(LibMPEG3_info))) == NULL) {
    show_message("LibMPEG3: " __FUNCTION__ ": No enough memory.\n");
    return PLAY_ERROR;
  }

  if ((info->file = mpeg3_open(st->path)) == NULL) {
    free(info);
    return PLAY_ERROR;
  }

  pthread_mutex_init(&info->update_mutex, NULL);
  pthread_cond_init(&info->update_cond, NULL);

  m->requested_type = video_window_request_type(vw, types, &m->direct_decode);
  if (!m->direct_decode) {
    show_message(__FUNCTION__ ": Cannot direct decoding...\n");
    return PLAY_ERROR;
  }
  debug_message("LibMPEG3: requested type: %s direct\n", image_type_to_string(m->requested_type));

  /* can set how many CPUs you want to use. */
  /* mpeg3_set_cpus(info->file, 2); */
  /* mpeg3_set_mmx(info->file, 1); */

  if (!mpeg3_has_video(info->file)) {
    show_message("This stream has no video stream.\n");
    goto error;
  }

  if (mpeg3_has_audio(info->file)) {
    if (m->ap == NULL) {
      show_message("Audio not played.\n");
      info->nastreams = -mpeg3_total_astreams(info->file);
    }
    else {
      info->nastreams = mpeg3_total_astreams(info->file);
      /* XXX: stream should be selectable */
      info->nastream = 0;

      m->sampleformat = _AUDIO_FORMAT_S16_LE;
      m->channels = mpeg3_audio_channels(info->file, info->nastream);
      m->samplerate = mpeg3_sample_rate(info->file, info->nastream);
      m->num_of_samples = mpeg3_audio_samples(info->file, info->nastream);
    }
  } else {
    debug_message("No audio streams.\n");
  }

  if ((info->nvstreams = mpeg3_total_vstreams(info->file)) > 1) {
    show_message("There are %d video streams in this whole stream.\n", info->nvstreams);
    show_message("Only the first video stream will be played(so far). Sorry.\n");
  }

  /* XXX: stream should be selectable */
  info->nvstream = 0;

  m->width = mpeg3_video_width(info->file, info->nvstream);
  m->height = mpeg3_video_height(info->file, info->nvstream);
  m->framerate = mpeg3_frame_rate(info->file, 0);
  m->num_of_frames = mpeg3_video_frames(info->file, 0);

  info->frametime = 1000 / m->framerate;

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

  debug_message("libmpeg3 player: (%d,%d) -> (%d,%d) %f fps %d frames\n",
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
    switch (p->type) {
    case _RGBA32:
      info->rendering_type = MPEG3_RGBA8888;
      break;
    case _BGRA32:
      info->rendering_type = MPEG3_BGRA8888;
      break;
    default:
      show_message(__FUNCTION__": requested type is %s.\n", image_type_to_string(p->type));
      return PLAY_ERROR;
    }
    p->depth = 24;
    p->bits_per_pixel = 32;
    p->bytes_per_line = p->width * 4;
    break;
  case 24:
    switch (p->type) {
    case _RGB24:
      info->rendering_type = MPEG3_RGB888;
      break;
    case _BGR24:
      info->rendering_type = MPEG3_BGR888;
      break;
    default:
      show_message(__FUNCTION__": requested type is %s.\n", image_type_to_string(p->type));
      return PLAY_ERROR;
    }
    p->depth = 24;
    p->bits_per_pixel = 24;
    p->bytes_per_line = p->width * 3;
    break;
  case 16:
    switch (p->type) {
    case _RGB_WITH_BITMASK:
    case _BGR_WITH_BITMASK:
      info->rendering_type = MPEG3_RGB565;
      break;
    default:
      show_message(__FUNCTION__": requested type is %s.\n", image_type_to_string(p->type));
      return PLAY_ERROR;
    }
    p->depth = 16;
    p->bits_per_pixel = 16;
    p->bytes_per_line = p->width * 2;
    break;
  default:
    show_message("Cannot render bpp %d\n", vw->bits_per_pixel);
    return PLAY_ERROR;
  }

  if ((info->lines = calloc(m->rendering_height, sizeof(unsigned char *))) == NULL)
    goto error;

  Bpp = vw->bits_per_pixel >> 3;
  /* extra 4 bytes are needed for MMX routine */
  if (memory_alloc(p->rendered.image, p->width * p->height * Bpp + 4) == NULL)
    goto error;

  for (i = 0; i < m->rendering_height; i++)
    info->lines[i] = memory_ptr(p->rendered.image) + i * p->width * Bpp;

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
static void *play_audio(void *);

static PlayerStatus
play(Movie *m)
{
  LibMPEG3_info *info = (LibMPEG3_info *)m->movie_private;

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

  mpeg3_set_frame(info->file, 0, info->nvstream);
  mpeg3_set_sample(info->file, 0, info->nastream);
  m->current_frame = 0;
  m->current_sample = 0;
  timer_start(m->timer);

  pthread_create(&info->video_thread, NULL, play_video, m);
  if (info->nastreams > 0)
    pthread_create(&info->audio_thread, NULL, play_audio, m);

  return PLAY_OK;
}

static void *
play_video(void *arg)
{
  Movie *m = arg;
  LibMPEG3_info *info = (LibMPEG3_info *)m->movie_private;
  int decode_error;

  while (m->status == _PLAY) {
    pthread_mutex_lock(&info->update_mutex);
    pthread_cond_wait(&info->update_cond, &info->update_mutex);
    decode_error =
      (mpeg3_read_frame(info->file, info->lines,
			0, 0,
			m->width, m->height,
			m->rendering_width, m->rendering_height,
			info->rendering_type, info->nvstream) == -1) ? 0 : 1;
    pthread_mutex_unlock(&info->update_mutex);
  }

  pthread_exit((void *)PLAY_OK);
}

#define AUDIO_READ_SIZE 2048
#define AUDIO_WRITE_SIZE 4096

static void *
play_audio(void *arg)
{
  Movie *m = arg;
  LibMPEG3_info *info = (LibMPEG3_info *)m->movie_private;
  AudioDevice *ad;
  static short input_buffer[AUDIO_WRITE_SIZE];
  static short output_buffer[AUDIO_WRITE_SIZE];
  int samples_to_read;
  int i;

  if ((ad = m->ap->open_device(NULL, m->c)) == NULL) {
    show_message("Cannot open device\n");
    pthread_exit((void *)PLAY_ERROR);
  }

  if (!m->ap->set_params(ad, &m->sampleformat, &m->channels, &m->samplerate))
    show_message("Some params are set wrong.\n");

  show_message("audio(%d streams): format(%d): %d ch rate %d kHz %d samples\n", info->nastreams, m->sampleformat, m->channels, m->samplerate, m->num_of_samples);

  samples_to_read = AUDIO_READ_SIZE;

  while (m->status == _PLAY) {
    if (m->current_sample >= m->num_of_samples) {
      m->ap->close_device(ad);
      pthread_exit((void *)PLAY_OK);
    }
    if (m->num_of_samples < m->current_sample + samples_to_read)
      samples_to_read = m->num_of_samples - m->current_sample;
    mpeg3_read_audio(info->file, NULL, input_buffer, 0, samples_to_read, info->nastream);
    if (m->channels == 1) {
      m->ap->write_device(ad, (unsigned char *)input_buffer, samples_to_read * sizeof(short));
    } else {
      mpeg3_reread_audio(info->file, NULL, input_buffer + AUDIO_READ_SIZE, 1, samples_to_read, info->nastream);
      for (i = 0; i < samples_to_read; i++) {
	output_buffer[i * 2    ] = input_buffer[i];
	output_buffer[i * 2 + 1] = input_buffer[i + AUDIO_READ_SIZE];
      }
      m->ap->write_device(ad, (unsigned char *)output_buffer, (samples_to_read << 1) * sizeof(short));
    }
    m->current_sample += samples_to_read;
  }
  m->ap->close_device(ad);
  pthread_exit((void *)PLAY_OK);
}

static PlayerStatus
play_main(Movie *m, VideoWindow *vw)
{
  LibMPEG3_info *info = (LibMPEG3_info *)m->movie_private;
  Image *p = info->p;
  int due_time;
  int time_elapsed;

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

  pthread_mutex_lock(&info->update_mutex);

  time_elapsed = timer_get_milli(m->timer);
  due_time = (m->current_frame + 1) * info->frametime;

  //debug_message("v: %d %d (%d frame)\n", time_elapsed, due_time, m->current_frame);

  if (time_elapsed < due_time) {
    /* too fast to display, wait then render */
    m->pause_usec((due_time - time_elapsed) * 1000);
    m->render_frame(vw, m, p);
  } else if (time_elapsed > due_time + info->frametime) {
    /* too late, drop several frames */
    int ndrop = (time_elapsed / info->frametime) - m->current_frame;
    //debug_message("drop %d frames ", ndrop);
    mpeg3_drop_frames(info->file, ndrop, info->nvstream);
    //debug_message("OK\n");
  } else {
    /* just in time to render */
    m->render_frame(vw, m, p);
  }

  m->current_frame = mpeg3_get_frame(info->file, info->nvstream);

  pthread_cond_signal(&info->update_cond);
  pthread_mutex_unlock(&info->update_mutex);

  if (m->current_frame >= m->num_of_frames)
    stop_movie(m);

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
  LibMPEG3_info *info = (LibMPEG3_info *)m->movie_private;
  void *v, *a;

  debug_message("LibMPEG3: " __FUNCTION__ "()\n");

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

  pthread_cond_signal(&info->update_cond);
  pthread_join(info->video_thread, &v);
  info->video_thread = 0;
  if (info->audio_thread) {
    pthread_join(info->audio_thread, &a);
    info->audio_thread = 0;
  }

  return PLAY_OK;
}

static void
unload_movie(Movie *m)
{
  LibMPEG3_info *info = (LibMPEG3_info *)m->movie_private;

  stop_movie(m);

  if (info) {
    if (info->lines)
      free(info->lines);
    if (info->p)
      image_destroy(info->p);

    mpeg3_close(info->file);

    pthread_mutex_destroy(&info->update_mutex);
    pthread_cond_destroy(&info->update_cond);

    free(info);
  }

  m->status = _UNLOADED;
}

/* methods */

static PlayerStatus
identify(Movie *m, Stream *st)
{
  if (st->path)
    return mpeg3_check_sig(st->path) ? PLAY_OK : PLAY_NOT;
  return PLAY_NOT;
}

static PlayerStatus
load(VideoWindow *vw, Movie *m, Stream *st)
{
  debug_message("libmpeg3 player: load() called\n");

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
