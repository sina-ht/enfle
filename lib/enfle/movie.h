/*
 * movie.h -- movie interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Jul  3 17:01:52 2005.
 * $Id: movie.h,v 1.26 2005/07/08 18:14:27 sian Exp $
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

#ifndef _MOVIE_H
#define _MOVIE_H

typedef struct _movie Movie;

#include "utils/libconfig.h"
#include "image.h"
#include "player-extra.h"
#include "demultiplexer.h"
#include "fourcc.h"
#include "video-plugin.h"
#include "videodecoder.h"
#include "audio-plugin.h"
#include "audiodecoder.h"
#include "stream.h"
#include "utils/timer.h"
#include "utils/timer_gettimeofday.h"

typedef enum {
  _STOP,
  _PAUSE,
  _PLAY,
  _RESIZING,
  _UNLOADED
} MovieStatus;

struct rational {
  int den;
  int num;
};
#define rational_to_double(r) ((double)r.num / (double)r.den)
#define rational_set_0(r) do {r.num = 0; r.den = 1;} while(0)

struct _movie {
  void *movie_private;
  Stream *st;
  MovieStatus status;
  ImageType requested_type;
  Timer *timer;
  Demultiplexer *demux;
  int bitrate;

  int has_video;
  int width, height;
  int rendering_width, rendering_height;
  unsigned int current_frame, num_of_frames;
  struct rational framerate;
  int direct_decode;
  unsigned int v_fourcc;
  const char *v_codec_name;
  VideoDecoder *vdec;
  unsigned int out_fourcc;
  int out_bitcount;
  void *video_header;
  void *video_extradata;
  int video_extradata_size;

  int has_audio;
  unsigned int current_sample, num_of_samples;
  AudioFormat sampleformat, sampleformat_actual;
  int channels, channels_actual;
  unsigned int samplerate, samplerate_actual;
  char *player_name, *format;
  int timer_offset_set;
  int timer_offset;
  unsigned int a_fourcc;
  const char *a_codec_name;
  AudioDecoder *adec;
  AudioPlugin *ap;
  int block_align;
  void *audio_extradata;
  int audio_extradata_size;

  /* These are callback functions which may or should be provided by UI. */
  int (*initialize_screen)(VideoWindow *, Movie *, int, int);
  int (*render_frame)(VideoWindow *, Movie *, Image *);
  int (*pause_usec)(unsigned int);

  /* These are methods. */
  void (*unload)(Movie *);
  void (*destroy)(Movie *);

  /* These are implemented by movie plugin. */
  PlayerStatus (*play)(Movie *);
  PlayerStatus (*play_main)(Movie *, VideoWindow *);
  PlayerStatus (*pause_movie)(Movie *);
  PlayerStatus (*stop)(Movie *);
  void (*unload_movie)(Movie *);
};

#define movie_play(m) (m)->play((m))
#define movie_play_main(m, vw) (m)->play_main((m), (vw))
#define movie_pause(m) (m)->pause_movie((m))
#define movie_stop(m) (m)->stop((m))
#define movie_unload(m) (m)->unload((m))
#define movie_resize(m) (m)->status = _RESIZING
#define movie_destroy(m) (m)->destroy((m))

Movie *movie_create(void);

#endif
