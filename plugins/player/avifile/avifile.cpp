/*
 * avifile.cpp -- avifile player plugin, which exploits avifile.
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Jun 18 05:32:17 2001.
 * $Id: avifile.cpp,v 1.11 2001/06/17 20:56:31 sian Exp $
 *
 * NOTES: 
 *  This plugin is not fully enfle plugin compatible, because stream
 *  cannot be used for input. Stream's path is used as input filename.
 *
 *  Requires avifile.
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
#include <errno.h>

#include <avifile.h>

#include "common.h"

#ifndef USE_PTHREAD
#  error pthread is mandatory for avifile plugin
#endif

#define FCC(a,b,c,d) ((((((d << 8) | c) << 8) | b) << 8) | a)
#define FCC_YUY2 FCC('Y', 'U', 'Y', '2')
#define FCC_YV12 FCC('Y', 'V', '1', '2')

extern "C" {
#include "enfle/memory.h"
#include "enfle/player-plugin.h"
}

typedef enum _decoding_state {
  _DECODING,
  _DECODED
} DecodingState;

typedef struct _avifile_info {
  int eof;
  int frametime;
  int use_xv;
  Image *p;
  CImage *ci;
  IAviReadFile *rf;
  IAviReadStream *stream;
  IAviReadStream *audiostream;
  MainAVIHeader hdr;
  BITMAPINFOHEADER bmih;
  WAVEFORMATEX owf;
  pthread_t video_thread;
  pthread_t audio_thread;
  pthread_mutex_t decoding_state_mutex;
  DecodingState ds;
  pthread_cond_t decoding_cond;
  pthread_cond_t decoded_cond;
} AviFile_info;

static const unsigned int types =
  (IMAGE_RGBA32 | IMAGE_BGRA32 | IMAGE_RGB24 | IMAGE_BGR24 | IMAGE_BGR_WITH_BITMASK);

extern "C" {
static PlayerStatus identify(Movie *, Stream *);
static PlayerStatus load(VideoWindow *, Movie *, Stream *);
}

static PlayerStatus play(Movie *);
static PlayerStatus pause_movie(Movie *);
static PlayerStatus stop_movie(Movie *);

static PlayerPlugin plugin = {
  type: ENFLE_PLUGIN_PLAYER,
  name: "AviFile",
  description: (const unsigned char *)"AviFile Player plugin version 0.5",
  author: (const unsigned char *)"Hiroshi Takekawa",
  identify: identify,
  load: load
};

extern "C" {
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
}

/* for internal use */

PlayerStatus
load_movie(VideoWindow *vw, Movie *m, Stream *st)
{
  AviFile_info *info = (AviFile_info *)m->movie_private;
  IAviReadFile *rf;
  IAviReadStream *audiostream, *stream;
  int result;
  int tmp_types;

  pthread_mutex_init(&info->decoding_state_mutex, NULL);
  pthread_cond_init(&info->decoding_cond, NULL);
  pthread_cond_init(&info->decoded_cond, NULL);

  rf = info->rf = CreateIAviReadFile(st->path);
  if (!rf)
    goto error;
  if (!rf->StreamCount() || rf->GetFileHeader(&info->hdr))
    goto error;
  m->num_of_frames = info->hdr.dwTotalFrames;

  m->has_video = 0;
  m->has_audio = 0;
  if ((audiostream = info->audiostream = rf->GetStream(0, IAviReadStream::Audio))) {
    if (m->ap == NULL)
      show_message("Audio plugin is not available.\n");
    else {
      if ((result = audiostream->StartStreaming()) != 0) {
	show_message("audio: StartStream() failed.\n");
	goto error;
      }
      debug_message("Get output format.\n");
      audiostream->GetOutputFormat(&info->owf, sizeof info->owf);
      m->sampleformat = _AUDIO_FORMAT_S16_LE;
      m->channels = info->owf.nChannels;
      m->samplerate = info->owf.nSamplesPerSec;
      //m->num_of_samples = info->owf.nSamplesPerSec * len;
      show_message("audio: format(%d, %d bits per sample): %d ch rate %d kHz\n", m->sampleformat, info->owf.wBitsPerSample, m->channels, m->samplerate);
      m->has_audio = 1;
    }
  } else {
    show_message("AviFile: No audio.\n");
  }

  Image *p;

  tmp_types = types;
  p = info->p = image_create();
  info->use_xv = 0;
  if ((stream = info->stream = rf->GetStream(0, IAviReadStream::Video))) {
    if ((result = stream->StartStreaming()) != 0) {
      show_message("video: StartStream() failed.\n");
      goto error;
    }

    IVideoDecoder::CAPS caps = stream->GetDecoder()->GetCapabilities();
    if (caps & IVideoDecoder::CAP_YUY2) {
      debug_message("Good, YUV422 available.\n");
      tmp_types |= IMAGE_YUV422;
    }
    if (caps & IVideoDecoder::CAP_YV12) {
      debug_message("Good, YUV420P available.\n");
      tmp_types |= IMAGE_YUV420_PLANAR;
    }
    if (types == tmp_types)
      debug_message("YUV422, YUV420P not available, using RGB.\n");
    m->requested_type = video_window_request_type(vw, tmp_types, &m->direct_decode);
    debug_message("AviFile: requested type: %s direct\n", image_type_to_string(m->requested_type));
    if (!m->direct_decode) {
      show_message("AviFile: load_movie: Cannot direct decoding...\n");
      goto error;
    }
    switch (m->requested_type) {
    case _YUV422:
    case _YVU422:
      debug_message("SetDestFmt(0, FCC_YUY2);\n");
      result = stream->GetDecoder()->SetDestFmt(0, FCC_YUY2);
      info->use_xv = 1;
      break;
    case _YUV420P:
    case _YVU420P:
      debug_message("SetDestFmt(0, FCC_YV12);\n");
      result = stream->GetDecoder()->SetDestFmt(0, FCC_YV12);
      info->use_xv = 1;
      break;
    default:
      debug_message("SetDestFmt(%d);\n", vw->bits_per_pixel);
      result = stream->GetDecoder()->SetDestFmt(vw->bits_per_pixel);
      break;
    }
    if (result != 0) {
      show_message("SetDestFmt() failed.\n");
      goto error;
    }
    stream->GetVideoFormatInfo(&info->bmih);
    m->width  = info->bmih.biWidth;
    m->height = info->bmih.biHeight;
    info->frametime = (int)(stream->GetFrameTime() * 1000);
    m->framerate = 1000 / info->frametime;
    m->has_video = 1;

    video_window_calc_magnified_size(vw, m->width, m->height, &p->magnified.width, &p->magnified.height);

    if (info->use_xv) {
      m->rendering_width  = m->width;
      m->rendering_height = m->height;
    } else {
      m->rendering_width  = p->magnified.width;
      m->rendering_height = p->magnified.height;
    }

    debug_message("AviFile: s (%d,%d) r (%d,%d) d (%d,%d) %f fps %d frames\n",
		  m->width, m->height, m->rendering_width, m->rendering_height,
		  p->magnified.width, p->magnified.height,
		  m->framerate, m->num_of_frames);
  } else {
    show_message("AviFile: No video.\n");
  }

  p->width = m->rendering_width;
  p->height = m->rendering_height;
  p->type = m->requested_type;
  if ((p->rendered.image = memory_create()) == NULL)
    goto error;
  memory_request_type(p->rendered.image, video_window_preferred_memory_type(vw));

  if (info->use_xv) {
    switch (p->type) {
    case _YUV422:
    case _YVU422:
      p->bits_per_pixel = 16;
      p->bytes_per_line = p->width << 1;
      break;
    case _YUV420P:
    case _YVU420P:
      p->bits_per_pixel = 12;
      p->bytes_per_line = p->width * 3 / 2;
      break;
    default:
      show_message("Cannot render %s\n", image_type_to_string(p->type));
      return PLAY_ERROR;
    }
  } else {
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
  }
  memory_alloc(p->rendered.image, p->bytes_per_line * p->height);

  m->st = st;
  m->status = _STOP;

  debug_message("AviFile: initializing screen.\n");

  m->initialize_screen(vw, m, p->magnified.width, p->magnified.height);

  play(m);

  return PLAY_OK;

 error:
  if (rf)
    delete rf;
  if (info)
    free(info);
  return PLAY_ERROR;
}

static void *play_video(void *);
static void *play_audio(void *);

static PlayerStatus
play(Movie *m)
{
  AviFile_info *info = (AviFile_info *)m->movie_private;

  debug_message("AviFile: play()\n");

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

  if (m->has_video) {
    debug_message("AviFile: rewind(video)\n");
    info->stream->Seek((unsigned int)0);
  }
  if (m->has_audio) {
    debug_message("AviFile: rewind(audio)\n");
    info->audiostream->Seek((unsigned int)0);
    m->current_sample = 0;
  }
  info->eof = 0;
  timer_start(m->timer);

  info->ds = _DECODING;
  if (m->has_video)
    pthread_create(&info->video_thread, NULL, play_video, m);
  if (m->has_audio)
    pthread_create(&info->audio_thread, NULL, play_audio, m);

  return PLAY_OK;
}

static void *
play_video(void *arg)
{
  Movie *m = (Movie *)arg;
  AviFile_info *info = (AviFile_info *)m->movie_private;

  debug_message("AviFile: play_video()\n");

  while (m->status == _PLAY) {
    pthread_mutex_lock(&info->decoding_state_mutex);
    while (info->ds != _DECODING)
      pthread_cond_wait(&info->decoding_cond, &info->decoding_state_mutex);
    if (info->stream->Eof()) {
      info->eof = 1;
      pthread_mutex_unlock(&info->decoding_state_mutex);
      break;
    }
    info->stream->ReadFrame();
    info->ci = info->stream->GetFrame();
    memcpy(memory_ptr(info->p->rendered.image), info->ci->data(), info->ci->bpl() * info->ci->height());
    info->ds = _DECODED;
    pthread_cond_signal(&info->decoded_cond);
    pthread_mutex_unlock(&info->decoding_state_mutex);
  }

  debug_message("AviFile: play_video() exit\n");

  pthread_exit((void *)PLAY_OK);
}

#define AUDIO_BUFFER_SIZE 20000

static void *
play_audio(void *arg)
{
  Movie *m = (Movie *)arg;
  AviFile_info *info = (AviFile_info *)m->movie_private;
  AudioDevice *ad;
  unsigned int samples, ocnt;
  int samples_to_read;
  int i;

  debug_message("AviFile: play_audio()\n");

  if ((ad = m->ap->open_device(NULL, m->c)) == NULL) {
    show_message("Cannot open device.\n");
    pthread_exit((void *)PLAY_ERROR);
  }

  if (!m->ap->set_params(ad, &m->sampleformat, &m->channels, &m->samplerate))
    show_message("Some params are set wrong.\n");

  unsigned char *input_buffer = new unsigned char [AUDIO_BUFFER_SIZE];

  samples_to_read = AUDIO_BUFFER_SIZE;
  while (m->status == _PLAY) {
    if (info->audiostream->Eof())
      break;
    samples = ocnt = 0;
    info->audiostream->ReadFrames(input_buffer, samples_to_read, samples_to_read, samples, ocnt);
    //debug_message("read %d samples (%d bytes)\n", samples, ocnt);
    m->ap->write_device(ad, input_buffer, ocnt);
    if (m->current_sample == 0) {
      timer_stop(m->timer);
      timer_start(m->timer);
    }
    m->current_sample += samples;
  }
  m->ap->close_device(ad);
  delete input_buffer;

  debug_message("AviFile: play_audio() exit\n");

  pthread_exit((void *)PLAY_OK);
}

PlayerStatus
play_main(Movie *m, VideoWindow *vw)
{
  AviFile_info *info = (AviFile_info *)m->movie_private;
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

  if (info->eof) {
    stop_movie(m);
    return PLAY_OK;
  }

  time_elapsed = (int)timer_get_milli(m->timer);
  m->current_frame = info->stream->GetPos();
  due_time = (int)(info->stream->GetTime() * 1000);
  //debug_message("v: %d %d (%d frame)\n", time_elapsed, due_time, m->current_frame);

  if (info->ci) {
    pthread_mutex_lock(&info->decoding_state_mutex);
    while (info->ds != _DECODED)
      pthread_cond_wait(&info->decoded_cond, &info->decoding_state_mutex);

    /* if too fast to display, wait before render */
    if (time_elapsed < due_time)
      m->pause_usec((due_time - time_elapsed) * 1000);

    /* skip if delayed */
    while (info->stream->GetTime() * 1000 < timer_get_milli(m->timer) - info->frametime) {
      info->ds = _DECODING;
      pthread_cond_signal(&info->decoding_cond);
      pthread_mutex_unlock(&info->decoding_state_mutex);
      pthread_mutex_lock(&info->decoding_state_mutex);
      while (info->ds != _DECODED)
	pthread_cond_wait(&info->decoded_cond, &info->decoding_state_mutex);
      pthread_mutex_unlock(&info->decoding_state_mutex);
    }

    info->ds = _DECODING;
    pthread_cond_signal(&info->decoding_cond);
    pthread_mutex_unlock(&info->decoding_state_mutex);

    m->render_frame(vw, m, p);
  }

  return PLAY_OK;
}

static PlayerStatus
pause_movie(Movie *m)
{
  AviFile_info *info = (AviFile_info *)m->movie_private;

  debug_message("AviFile: pause_movie()\n");

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
  AviFile_info *info = (AviFile_info *)m->movie_private;
  void *v, *a;

  debug_message("AviFile: stop_movie()\n");

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

  pthread_cond_signal(&info->decoding_cond);
  if (info->video_thread) {
    pthread_join(info->video_thread, &v);
    info->video_thread = 0;
  }
  if (info->audio_thread) {
    pthread_join(info->audio_thread, &v);
    info->audio_thread = 0;
  }

  return PLAY_OK;
}

static void
unload_movie(Movie *m)
{
  AviFile_info *info = (AviFile_info *)m->movie_private;

  debug_message("AviFile: unload_movie()\n");

  stop_movie(m);

  if (info) {
    if (info->rf)
      delete info->rf;
    if (info->p)
      image_destroy(info->p);
    pthread_mutex_destroy(&info->decoding_state_mutex);
    pthread_cond_destroy(&info->decoding_cond);
    pthread_cond_destroy(&info->decoded_cond);
    free(info);
  }
}

/* methods */

extern "C" {

static PlayerStatus
identify(Movie *m, Stream *st)
{
  unsigned char buf[16];
  IAviReadFile *rf;
  IAviReadStream *audiostream, *stream;
  int result;

  /* see if this is asf by extension... */
  if (strlen(st->path) < 4 || strcasecmp(st->path + strlen(st->path) - 4, ".asf")) {
    if (stream_read(st, buf, 16) != 16)
      return PLAY_NOT;

    if (memcmp(buf, "RIFF", 4))
      return PLAY_NOT;
    if (memcmp(buf + 8, "AVI ", 4))
      return PLAY_NOT;
  }

  debug_message("AviFile: identify: identified as avi.\n");

  /* see if this avi is supported by avifile */
  rf = CreateIAviReadFile(st->path);
  if (!rf)
    goto not;
  if (!rf->StreamCount())
    goto not;

  if ((audiostream = rf->GetStream(0, IAviReadStream::Audio))) {
    if ((result = audiostream->StartStreaming()) != 0)
      goto not;
  }

  if ((stream = rf->GetStream(0, IAviReadStream::Video))) {
    if ((result = stream->StartStreaming()) != 0)
      goto not;
  }
  delete rf;

  return PLAY_OK;

 not:
  debug_message("AviFile: identify: but not supported by avifile.\n");
  if (rf)
    delete rf;
  return PLAY_NOT;
}

static PlayerStatus
load(VideoWindow *vw, Movie *m, Stream *st)
{
  AviFile_info *info;

  debug_message("AviFile: load() called\n");

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

  if ((info = (AviFile_info *)calloc(1, sizeof(AviFile_info))) == NULL) {
    show_message("AviFile: load: No enough memory.\n");
    return PLAY_ERROR;
  }
  m->movie_private = (void *)info;

  return load_movie(vw, m, st);
}
}
