/*
 * movie.h -- movie interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Thu Oct 12 19:21:45 2000.
 * $Id: movie.h,v 1.3 2000/10/12 15:47:02 sian Exp $
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

#include "image.h"
#include "stream.h"
#include "ui-plugin.h"

typedef enum {
  _STOP,
  _PAUSE,
  _PLAY,
  _UNLOADED
} MovieStatus;

struct _movie {
  Stream *st;
  MovieStatus status;
  void *movie_private;
  int width, height;
  int nthframe;
  char *format;

  /* These are callback functions which may or should be provided by UI. */
  int (*initialize_screen)(UIData *, Movie *, int, int);
  int (*render_frame)(UIData *, Movie *, Image *);
  int (*pause_usec)(unsigned int);

  /* This is a method. */
  void (*unload)(Movie *);
  void (*destroy)(Movie *);

  /* These are implemented by movie plugin. */
  void *(*get_screen)(Movie *);
  int (*play)(Movie *);
  int (*play_main)(Movie *, UIData *);
  int (*pause_movie)(Movie *);
  int (*stop)(Movie *);
  void (*unload_movie)(Movie *);
};

#define movie_get_screen(m) (m)->get_screen((m))
#define movie_play(m) (m)->play((m))
#define movie_play_main(m, u) (m)->play_main((m), (u))
#define movie_pause(m) (m)->pause_movie((m))
#define movie_stop(m) (m)->stop((m))
#define movie_unload(m) (m)->unload((m))
#define movie_destroy(m) (m)->destroy((m))

Movie *movie_create(void);

#endif
