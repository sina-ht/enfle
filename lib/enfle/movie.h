/*
 * movie.h -- movie interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Dec 19 00:16:53 2000.
 * $Id: movie.h,v 1.9 2000/12/18 17:00:13 sian Exp $
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

#include "libconfig.h"
#include "image.h"
#include "video-plugin.h"
#include "audio-plugin.h"
#include "stream.h"
#include "timer.h"
#include "timer_gettimeofday.h"

typedef enum {
  _STOP,
  _PAUSE,
  _PLAY,
  _UNLOADED
} MovieStatus;

struct _movie {
  Stream *st;
  Config *c;
  AudioPlugin *ap;
  MovieStatus status;
  ImageType requested_type;
  Timer *timer;
  void *movie_private;
  int width, height;
  int rendering_width, rendering_height;
  int current_frame, previous_frame, num_of_frames;
  float framerate;
  int play_every_frame;
  int direct_decode;
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
  void *(*get_screen)(Movie *);
  int (*play)(Movie *);
  int (*play_main)(Movie *, VideoWindow *);
  int (*pause_movie)(Movie *);
  int (*stop)(Movie *);
  void (*unload_movie)(Movie *);
};

#define movie_get_screen(m) (m)->get_screen((m))
#define movie_play(m) (m)->play((m))
#define movie_play_main(m, vw) (m)->play_main((m), (vw))
#define movie_pause(m) (m)->pause_movie((m))
#define movie_stop(m) (m)->stop((m))
#define movie_unload(m) (m)->unload((m))
#define movie_set_play_every_frame(m, f) (m)->set_play_every_frame((m), (f))
#define movie_get_play_every_frame(m) (m)->get_play_every_frame((m))
#define movie_destroy(m) (m)->destroy((m))

Movie *movie_create(Config *);

#endif
