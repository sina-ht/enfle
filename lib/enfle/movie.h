/*
 * movie.h -- movie interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Mar  4 22:35:02 2002.
 * $Id: movie.h,v 1.18 2002/03/04 20:22:42 sian Exp $
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
#include "video-plugin.h"
#include "audio-plugin.h"
#include "stream.h"
#include "utils/timer.h"
#include "utils/timer_gettimeofday.h"

typedef enum {
  _STOP,
  _PAUSE,
  _PLAY,
  _UNLOADED
} MovieStatus;

struct _movie {
  Stream *st;
  AudioPlugin *ap;
  MovieStatus status;
  ImageType requested_type;
  Timer *timer;
  void *movie_private;
  int width, height;
  int rendering_width, rendering_height;
  int has_video;
  unsigned int current_frame, num_of_frames;
  float framerate;
  int play_every_frame;
  int direct_decode;
  int has_audio;
  unsigned int current_sample, num_of_samples;
  AudioFormat sampleformat, sampleformat_actual;
  int channels, channels_actual, samplerate, samplerate_actual;
  char *format;

  /* These are callback functions which may or should be provided by UI. */
  int (*initialize_screen)(VideoWindow *, Movie *, int, int);
  int (*render_frame)(VideoWindow *, Movie *, Image *);
  int (*pause_usec)(unsigned int);

  /* These are methods. */
  void (*unload)(Movie *);
  void (*set_play_every_frame)(Movie *, int);
  int (*get_play_every_frame)(Movie *);
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
#define movie_set_play_every_frame(m, f) (m)->set_play_every_frame((m), (f))
#define movie_get_play_every_frame(m) (m)->get_play_every_frame((m))
#define movie_destroy(m) (m)->destroy((m))

Movie *movie_create(void);

#endif
