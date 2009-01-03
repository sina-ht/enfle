/*
 * movie.c -- movie interface
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Dec 28 10:29:23 2008.
 * $Id: movie.c,v 1.13 2009/01/03 15:35:57 sian Exp $
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
#define REQUIRE_STRING_H
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
static void destroy(Movie *);
static void lock(Movie *);
static void unlock(Movie *);

static Movie template = {
  .initialize_screen = initialize_screen,
  .render_frame = render_frame,
  .pause_usec = pause_usec,
  .unload = unload,
  .destroy = destroy,
  .lock = lock,
  .unlock = unlock,
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
destroy(Movie *m)
{
  unload(m);
  if (m->timer)
    timer_destroy(m->timer);
  free(m);
}

static void
lock(Movie *m)
{
  pthread_mutex_lock(&m->mutex);
}

static void
unlock(Movie *m)
{
  pthread_mutex_unlock(&m->mutex);
}
