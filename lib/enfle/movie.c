/*
 * movie.c -- movie interface
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Nov  4 04:56:19 2000.
 * $Id: movie.c,v 1.7 2000/11/04 17:31:28 sian Exp $
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

#define REQUIRE_UNISTD_H
#include "compat.h"

#ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
#endif

#include "common.h"

#include "movie.h"

static int initialize_screen(VideoWindow *, Movie *, int, int);
static int render_frame(VideoWindow *, Movie *, Image *);
static int pause_usec(unsigned int);

static void unload(Movie *);
static void set_play_every_frame(Movie *, int);
static int get_play_every_frame(Movie *);
static void destroy(Movie *);

static Movie template = {
  initialize_screen: initialize_screen,
  render_frame: render_frame,
  pause_usec: pause_usec,
  unload: unload,
  set_play_every_frame: set_play_every_frame,
  get_play_every_frame: get_play_every_frame,
  destroy: destroy
};

Movie *
movie_create(void)
{
  Movie *m;

  if ((m = calloc(1, sizeof(Movie))) == NULL)
    return NULL;
  memcpy(m, &template, sizeof(Movie));

  m->timer = enfle_timer_create(timer_gettimeofday());

  return m;
}

/* default callback functions */

static int
initialize_screen(VideoWindow *vw, Movie *m, int w, int h)
{
  show_message("Movie screen: (%d x %d)\n", w, h);
  return 1;
}

static int
render_frame(VideoWindow *vw, Movie *m, Image *p)
{
  /* do nothing */
  return 1;
}

static int
pause_usec(unsigned int usec)
{
  struct timeval tv;

  tv.tv_sec  = usec / 1000000;
  tv.tv_usec = usec % 1000000;
  select(0, NULL, NULL, NULL, &tv);

  return 1;
}

/* methods */

static void
unload(Movie *m)
{
  if (m->status != _UNLOADED && m->unload_movie)
    m->unload_movie(m);
  m->status = _UNLOADED;
}

static void
set_play_every_frame(Movie *m, int f)
{
  m->play_every_frame = f;
}

static int
get_play_every_frame(Movie *m)
{
  return m->play_every_frame;
}

static void
destroy(Movie *m)
{
  unload(m);
  if (m->timer)
    timer_destroy(m->timer);
  free(m);
}
