/*
 * avifile.cpp -- avifile player plugin, which exploits avifile.
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Wed Mar 28 23:11:16 2018.
 *
 * NOTES: 
 *  This plugin is not fully enfle plugin compatible, because stream
 *  cannot be used for input. Stream's path is used as input filename.
 *
 *  Requires avifile.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#define DEBUG
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#include <avifile/avifile.h>
#include <avifile/except.h>
#include <avifile/version.h>
#if (AVIFILE_MAJOR_VERSION == 0 && AVIFILE_MINOR_VERSION == 6)
# include <avifile/avifmt.h>
#elif (AVIFILE_MAJOR_VERSION == 0 && AVIFILE_MINOR_VERSION >= 7) || (AVIFILE_MAJOR_VERSION > 0)
# include <avifile/infotypes.h>
# define USE_STREAM_INFO
#endif

#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"
#include "enfle/fourcc.h"

#ifndef USE_PTHREAD
#  error pthread is mandatory for avifile plugin
#endif

extern "C" {
#include "enfle/memory.h"
#include "enfle/player-plugin.h"
#include "utils/libstring.h"
}

typedef struct _avifile_info {
  int eof;
  int frametime;
  int use_xv;
  Image *p;
  Config *c;
  CImage *ci;
#ifdef USE_STREAM_INFO
  StreamInfo *si;
#else
  MainAVIHeader hdr;
#endif
  IAviReadFile *rf;
  IAviReadStream *stream;
  IAviReadStream *audiostream;
  BITMAPINFOHEADER bmih;
  WAVEFORMATEX owf;
  pthread_t video_thread;
  pthread_t audio_thread;
  pthread_mutex_t update_mutex;
  pthread_cond_t update_cond;
  int skip;
} AviFile_info;

static const unsigned int types =
  (IMAGE_RGBA32 | IMAGE_BGRA32 | IMAGE_RGB24 | IMAGE_BGR24 | IMAGE_BGR_WITH_BITMASK);

extern "C" {
DECLARE_PLAYER_PLUGIN_METHODS;
}

static PlayerStatus play(Movie *);
static PlayerStatus pause_movie(Movie *);
static PlayerStatus stop_movie(Movie *);

#define PLAYER_AVIFILE_PLUGIN_DESCRIPTION "AviFile Player plugin version 0.6"

static PlayerPlugin plugin = {
  type: ENFLE_PLUGIN_PLAYER,
  name: "AviFile",
  description: NULL,
  author: (const unsigned char *)"Hiroshi Takekawa",
  identify: identify,
  load: load
};

extern "C" {
ENFLE_PLUGIN_ENTRY(player_avifile)
{
  PlayerPlugin *pp;
  String *s;

  if ((pp = (PlayerPlugin *)calloc(1, sizeof(PlayerPlugin))) == NULL)
    return NULL;
  memcpy(pp, &plugin, sizeof(PlayerPlugin));
  s = string_create();
  string_set(s, (const char *)PLAYER_AVIFILE_PLUGIN_DESCRIPTION);
#ifdef AVIFILE_BETA_LEVEL
  string_catf(s, (const char *)" with avifile %d.%d.%d.%d", AVIFILE_MAJOR_VERSION, AVIFILE_MINOR_VERSION, AVIFILE_PATCHLEVEL, AVIFILE_BETA_LEVEL);
#else
  string_catf(s, (const char *)" with avifile %d.%d.%d", AVIFILE_MAJOR_VERSION, AVIFILE_MINOR_VERSION, AVIFILE_PATCHLEVEL);
#endif
  pp->description = (const unsigned char *)strdup((const char *)string_get(s));
  string_destroy(s);

  return (void *)pp;
}

ENFLE_PLUGIN_EXIT(player_avifile, p)
{
  PlayerPlugin *pp = (PlayerPlugin *)p;

  if (pp->description)
    free((unsigned char *)pp->description);
  free(pp);
}
}

#undef PLAYER_AVIFILE_PLUGIN_DESCRIPTION

/* for internal use */

static AviFile_info *
info_create(Movie *m)
{
  AviFile_info *info;

  if (!m->movie_private) {
    if ((info = (AviFile_info *)calloc(1, sizeof(AviFile_info))) == NULL) {
      show_message("AviFile: info_create: No enough memory.\n");
      return NULL;
    }
    m->movie_private = (void *)info;
  } else {
    info = (AviFile_info *)m->movie_private;
  }

  pthread_mutex_init(&info->update_mutex, NULL);
  pthread_cond_init(&info->update_cond, NULL);

  return info;
}

PlayerStatus
load_movie(VideoWindow *vw, Movie *m, Stream *st)
{
  AviFile_info *info = (AviFile_info *)m->movie_private;
  IAviReadFile *rf;
  IAviReadStream *audiostream, *stream;
  int result, dest_bpp = 0;
  int tmp_types;

  debug_message("AviFile: load_movie: try to CreateIAviReadFile().\n");
  if (!info->rf) {
    rf = info->rf = CreateIAviReadFile(st->path);
    debug_message("AviFile: load_movie: created.\n");
  } else {
    rf = info->rf;
    debug_message("AviFile: load_movie: using cache.\n");
  }

  if (!rf) {
    debug_message("AviFile: load_movie: rf == NULL.\n");
    goto error;
  }

  if (!rf->StreamCount()) {
    debug_message("AviFile: load_movie: No stream.\n");
    goto error;
  }

  m->has_video = 0;
  m->has_audio = 0;
  if ((audiostream = info->audiostream = rf->GetStream(0, IAviReadStream::Audio))) {
    if (m->ap == NULL) {
      show_message("Audio plugin is not available.\n");
      audiostream = NULL;
    } else {
      if ((result = audiostream->StartStreaming()) != 0) {
	debug_message("audio stream: StartStream() failed.\n");
	audiostream = NULL;
      } else {
	debug_message("Get output format.\n");
	audiostream->GetOutputFormat(&info->owf, sizeof info->owf);
	m->sampleformat = _AUDIO_FORMAT_S16_LE;
	m->channels = info->owf.nChannels;
	m->samplerate = info->owf.nSamplesPerSec;
	//m->num_of_samples = info->owf.nSamplesPerSec * len;
	show_message("audio: format(%d, %d bits per sample): %d ch rate %d kHz\n", m->sampleformat, info->owf.wBitsPerSample, m->channels, m->samplerate);
	m->has_audio = 1;
      }
    }
  }
  if (!audiostream)
    show_message("AviFile: No audio.\n");

  Image *p;

  tmp_types = types;
  p = info->p = image_create();
  info->use_xv = 0;
  if ((stream = info->stream = rf->GetStream(0, IAviReadStream::Video))) {

#ifdef USE_STREAM_INFO
    info->si = stream->GetStreamInfo();
    m->num_of_frames = info->si->GetStreamFrames();
#else
#if (AVIFILE_MAJOR_VERSION == 0 && AVIFILE_MINOR_VERSION == 6) || (AVIFILE_MAJOR_VERSION > 0)
    rf->GetHeader(&info->hdr, sizeof(info->hdr));
#else
    rf->GetFileHeader(&info->hdr);
#endif
    m->num_of_frames = info->hdr.dwTotalFrames;
#endif

    if ((result = stream->StartStreaming()) != 0) {
      debug_message("video stream: StartStream() failed.\n");
      stream = NULL;
    } else {
      IVideoDecoder::CAPS caps = stream->GetDecoder()->GetCapabilities();

      if (caps & IVideoDecoder::CAP_YUY2) {
	debug_message("Good, YUY2 available.\n");
	tmp_types |= IMAGE_YUY2;
      }
      if (caps & IVideoDecoder::CAP_YV12) {
	debug_message("Good, YV12 available.\n");
	tmp_types |= IMAGE_YV12;
      }
      if (caps & IVideoDecoder::CAP_IYUV) {
	debug_message("Good, IYUV(I420) available.\n");
	tmp_types |= IMAGE_I420;
      }
      if (caps & IVideoDecoder::CAP_UYVY) {
	debug_message("Good, UYVY available.\n");
	tmp_types |= IMAGE_UYVY;
      }
      if (types == tmp_types)
	debug_message("Neither YUY2, YV12, I420, nor UYUV is available, using RGB.\n");
      m->requested_type = video_window_request_type(vw, tmp_types, &m->direct_decode);
      debug_message("AviFile: requested type: %s direct\n", image_type_to_string(m->requested_type));
      if (!m->direct_decode) {
	show_message("AviFile: load_movie: Cannot direct decoding...\n");
	stream->StopStreaming();
	goto error;
      }
      switch (m->requested_type) {
      case _YUY2:
	debug_message("SetDestFmt(0, FCC_YUY2);\n");
	result = stream->GetDecoder()->SetDestFmt(0, FCC_YUY2);
	info->use_xv = 1;
	break;
      case _YV12:
	debug_message("SetDestFmt(0, FCC_YV12);\n");
	result = stream->GetDecoder()->SetDestFmt(0, FCC_YV12);
	info->use_xv = 1;
	break;
      case _I420:
	debug_message("SetDestFmt(0, FCC_IYUV);\n");
	result = stream->GetDecoder()->SetDestFmt(0, FCC_IYUV);
	info->use_xv = 1;
	break;
      case _UYVY:
	debug_message("SetDestFmt(0, FCC_UYVY);\n");
	result = stream->GetDecoder()->SetDestFmt(0, FCC_UYVY);
	info->use_xv = 1;
	break;
      default:
	if (vw->bits_per_pixel == 32) {
	  if ((result = stream->GetDecoder()->SetDestFmt(vw->bits_per_pixel)) != 0) {
	    /* XXX: hack */
	    debug_message("vw bpp == 32, but is not supported. Try 24.\n");
	    result = stream->GetDecoder()->SetDestFmt(24);
	    dest_bpp = 24;
	    m->requested_type = _RGB24;
	    m->direct_decode = 0;
	  } else {
	    dest_bpp = 32;
	  }
	} else {
	  debug_message("SetDestFmt(%d);\n", vw->bits_per_pixel);
	  result = stream->GetDecoder()->SetDestFmt(vw->bits_per_pixel);
	  dest_bpp = vw->bits_per_pixel;
	}
	break;
      }
      if (result != 0) {
	show_message("SetDestFmt() failed.\n");
	stream->StopStreaming();
	goto error;
      }

#ifdef USE_STREAM_INFO
      m->width  = info->si->GetVideoWidth();
      m->height = info->si->GetVideoHeight();
#else
#if (AVIFILE_MAJOR_VERSION == 0 && AVIFILE_MINOR_VERSION == 6) || (AVIFILE_MAJOR_VERSION > 0)
      stream->GetVideoFormatInfo(&info->bmih, sizeof(info->bmih));
#else
      stream->GetVideoFormatInfo(&info->bmih);
#endif
      m->width  = info->bmih.biWidth;
      m->height = info->bmih.biHeight;
#endif

      info->frametime = (int)(stream->GetFrameTime() * 1000);
      m->framerate = 1000 / info->frametime;
      m->has_video = 1;

      {
	unsigned int dw, dh;

	image_rendered_width(p) = m->width;
	image_rendered_height(p) = m->height;
	video_window_calc_magnified_size(vw, info->use_xv, m->width, m->height, &dw, &dh);

	if (info->use_xv) {
	  m->rendering_width  = m->width;
	  m->rendering_height = m->height;
	} else {
	  m->rendering_width  = dw;
	  m->rendering_height = dh;
	}
      }

      debug_message("AviFile: s (%d,%d) r (%d,%d) d (%d,%d) %f fps %d frames\n",
		    m->width, m->height, m->rendering_width, m->rendering_height,
		    image_rendered_width(p), image_rendered_height(p),
		    m->framerate, m->num_of_frames);
    }
  }
  if (!stream) {
    show_message("AviFile: No video.\n");
    m->rendering_width  = 120;
    m->rendering_height = 80;
    m->requested_type = _RGB24;
    dest_bpp = vw->bits_per_pixel;
  }

  image_width(p) = m->rendering_width;
  image_height(p) = m->rendering_height;
  p->type = m->requested_type;
  if (dest_bpp == 0)
    memory_request_type(image_rendered_image(p), video_window_preferred_memory_type(vw));

  if (info->use_xv) {
    switch (p->type) {
    case _YUY2:
    case _UYVY:
      p->bits_per_pixel = 16;
      image_bpl(p) = image_width(p) << 1;
      break;
    case _YV12:
    case _I420:
      p->bits_per_pixel = 12;
      image_bpl(p) = image_width(p) * 3 / 2;
      break;
    default:
      show_message("Cannot render %s\n", image_type_to_string(p->type));
      return PLAY_ERROR;
    }
  } else {
    switch (dest_bpp) {
    case 32:
      p->depth = 24;
      p->bits_per_pixel = 32;
      image_bpl(p) = image_width(p) * 4;
      break;
    case 24:
      p->depth = 24;
      p->bits_per_pixel = 24;
      image_bpl(p) = image_width(p) * 3;
      break;
    case 16:
      p->depth = 16;
      p->bits_per_pixel = 16;
      image_bpl(p) = image_width(p) * 2;
      break;
    default:
      show_message("Cannot render bpp %d\n", dest_bpp);
      return PLAY_ERROR;
    }
  }
  memory_alloc(image_rendered_image(p), image_bpl(p) * image_height(p));

  m->st = st;
  m->status = _STOP;

  debug_message("AviFile: initializing screen.\n");

  m->initialize_screen(vw, m, m->rendering_width, m->rendering_height);

  return play(m);

 error:
  if (rf)
    delete rf;
  if (info)
    free(info);
  m->status = _UNLOADED;
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

  if (m->has_video) {
    timer_start(m->timer);
    pthread_create(&info->video_thread, NULL, play_video, m);
  }
  if (m->has_audio)
    pthread_create(&info->audio_thread, NULL, play_audio, m);

  pthread_mutex_lock(&info->update_mutex);
  pthread_mutex_unlock(&info->update_mutex);

  if (m->has_video)
    debug_message(info->ci ? "OK\n" : "NG\n");

  return PLAY_OK;
}

static void *
play_video(void *arg)
{
  Movie *m = (Movie *)arg;
  AviFile_info *info = (AviFile_info *)m->movie_private;

  debug_message("AviFile: play_video()\n");

  while (m->status == _PLAY) {
    if (info->stream->Eof()) {
      info->eof = 1;
      break;
    }

    pthread_mutex_lock(&info->update_mutex);

    for (; info->skip; info->skip--)
      info->stream->ReadFrame();
    info->stream->ReadFrame();

    info->ci = info->stream->GetFrame();
#if (AVIFILE_MAJOR_VERSION == 0 && AVIFILE_MINOR_VERSION >= 6) || (AVIFILE_MAJOR_VERSION > 0)
    memcpy(memory_ptr(image_rendered_image(info->p)), info->ci->Data(), info->ci->Bpl() * info->ci->Height());
#else
    memcpy(memory_ptr(image_rendered_image(info->p)), info->ci->data(), info->ci->bpl() * info->ci->height());
#endif
    pthread_cond_wait(&info->update_cond, &info->update_mutex);
    pthread_mutex_unlock(&info->update_mutex);
  }

  debug_message("AviFile: play_video() exit\n");

  return (void *)PLAY_OK;
}

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

  if ((ad = m->ap->open_device(NULL, info->c)) == NULL) {
    show_message("Cannot open audio device.\n");
    return (void *)PLAY_ERROR;
  }

  m->sampleformat_actual = m->sampleformat;
  m->channels_actual = m->channels;
  m->samplerate_actual = m->samplerate;
  if (!m->ap->set_params(ad, &m->sampleformat_actual, &m->channels_actual, &m->samplerate_actual))
    show_message("Some params are set wrong.\n");

  while (m->status == _PLAY) {
    if (info->audiostream->Eof()) {
      if (!m->has_video)
	info->eof = 1;
      debug_message("AviFile: play_audio: EOF reached.\n");
      break;
    }
    samples = ocnt = 0;
    samples_to_read = info->audiostream->GetFrameSize();
    unsigned char *input_buffer = new unsigned char [samples_to_read];
    info->audiostream->ReadFrames(input_buffer, samples_to_read, samples_to_read, samples, ocnt);
    //debug_message("AviFile: play_audio: read %d samples (%d bytes)\n", samples, ocnt);
    if (m->current_sample == 0 && m->has_video)
      timer_start(m->timer);
    m->ap->write_device(ad, input_buffer, ocnt);
    delete input_buffer;
    m->current_sample += samples;
  }
  debug_message_fnc("sync_device()...");
  m->ap->sync_device(ad);
  debug_message("OK\n");
  debug_message_fnc("close_device()...");
  m->ap->close_device(ad);
  debug_message("OK\n");

  debug_message("AviFile: play_audio() exit\n");

  return (void *)PLAY_OK;
}

PlayerStatus
play_main(Movie *m, VideoWindow *vw)
{
  AviFile_info *info = (AviFile_info *)m->movie_private;
  Image *p = info->p;
  int due_time;

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
    return PLAY_ERROR;
  }

  if (info->eof) {
    stop_movie(m);
    return PLAY_OK;
  }

  if (!m->has_video)
    return PLAY_OK;

  m->current_frame = info->stream->GetPos();

  due_time = (int)(info->stream->GetTime() * 1000);
  //debug_message("v: %d %d (%d frame)\n", timer_get_milli(m->timer), due_time, m->current_frame);

  pthread_mutex_lock(&info->update_mutex);

  /* if too fast to display, wait before render */
  if (timer_get_milli(m->timer) < due_time) {
    int wait_time = (int)((due_time - timer_get_milli(m->timer)) * 1000);

    if (wait_time > 0) {
      //debug_message("wait %d usec.\n", wait_time);
      m->pause_usec(wait_time);
    }
  } else {
    info->skip = (int)((timer_get_milli(m->timer) - due_time) / info->frametime);
#ifdef DEBUG
    if (info->skip > 0)
      debug_message("Drop %d frames.\n", info->skip);
#endif
  }

  pthread_cond_signal(&info->update_cond);
  pthread_mutex_unlock(&info->update_mutex);

  m->render_frame(vw, m, p);

  return PLAY_OK;
}

static PlayerStatus
pause_movie(Movie *m)
{
  AviFile_info *info = (AviFile_info *)m->movie_private;

  debug_message("AviFile: pause_movie()\n");

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
  case _RESIZING:
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

  if (m->status == _UNLOADED)
    return;
  stop_movie(m);

  if (info) {
    if (info->audiostream)
      info->audiostream->StopStreaming();
    if (info->stream)
      info->stream->StopStreaming();
    if (info->rf)
      delete info->rf;
    if (info->p)
      image_destroy(info->p);
    pthread_mutex_destroy(&info->update_mutex);
    pthread_cond_destroy(&info->update_cond);
    free(info);
  }
}

/* methods */

extern "C" {

DEFINE_PLAYER_PLUGIN_IDENTIFY(m, st, c, priv)
{
  unsigned char buf[16];
  IAviReadFile *rf;
  IAviReadStream *audiostream, *stream;
  AviFile_info *info;
  int result;

  /* see if this is asf or wmv by extension... */
  if (strlen(st->path) < 4 ||
      (strcasecmp(st->path + strlen(st->path) - 4, ".asf") &&
       strcasecmp(st->path + strlen(st->path) - 4, ".wmv"))) {
    if (stream_read(st, buf, 16) != 16)
      return PLAY_NOT;

    if (memcmp(buf, "RIFF", 4))
      return PLAY_NOT;
    if (memcmp(buf + 8, "AVI ", 4))
      return PLAY_NOT;
  }

  debug_message("AviFile: identify: identified as AviFile(maybe).\n");

  info = info_create(m);

  /* see if this avi is supported by avifile */
  if (!info->rf) {
    debug_message("AviFile: identify: try to CreateIAviReadFile().\n");
    rf = info->rf = CreateIAviReadFile(st->path);
    debug_message("AviFile: identify: created.\n");
  } else {
    rf = info->rf;
    debug_message("AviFile: identify: using cache.\n");
  }

  if (!rf) {
    debug_message("AviFile: identify: rf == NULL.\n");
    goto not_avi;
  }
  if (!rf->StreamCount()) {
    debug_message("AviFile: identify: No stream.\n");
    goto not_avi;
  }

  if ((audiostream = rf->GetStream(0, IAviReadStream::Audio))) {
    if ((result = audiostream->StartStreaming()) != 0)
      show_message("AviFile: identify: Audio stream not played.\n");
    else
      audiostream->StopStreaming();
  }

  if ((stream = rf->GetStream(0, IAviReadStream::Video))) {
    if ((result = stream->StartStreaming()) != 0)
      show_message("AviFile: identify: Video stream not played.\n");
    else
      stream->StopStreaming();
  }

#if 0
  debug_message("AviFile: identify: deleting rf.\n");
  delete rf;
  info->rf = NULL;
  debug_message("AviFile: identify: deleted rf.\n");
#endif

  return PLAY_OK;

 not_avi:
  debug_message("AviFile: identify: but not supported by avifile.\n");
  if (audiostream)
    audiostream->StopStreaming();
  if (stream)
    stream->StopStreaming();
  if (rf)
    delete rf;
  return PLAY_NOT;
}

DEFINE_PLAYER_PLUGIN_LOAD(vw, m, st, c, priv)
{
  AviFile_info *info;

  debug_message("AviFile: load() called\n");

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

  info = info_create(m);
  info->c = c;

  return load_movie(vw, m, st);
}
}
