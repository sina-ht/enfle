/*
 * movie.c -- movie interface
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Oct 10 05:09:58 2000.
 * $Id: movie.c,v 1.1 2000/10/09 20:19:44 sian Exp $
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

#include <stdlib.h>

#define REQUIRE_UNISTD_H
#include "compat.h"

#ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
#endif

#include "common.h"

#include "movie.h"

static int render_frame(Movie *, Image *);
static int pause_usec(unsigned int);

static int play(Movie *);
static int pause_movie(Movie *);
static int stop(Movie *);
static void destroy(Movie *);

static Movie template = {
  render_frame: render_frame,
  pause_usec: pause_usec,
  play: play,
  pause_movie: pause_movie,
  stop: stop,
  destroy: destroy
};

Movie *
movie_create(void)
{
  Movie *m;

  if ((m = calloc(1, sizeof(Movie))) == NULL)
    return NULL;
  memcpy(m, &template, sizeof(Movie));

  return m;
}

/* default callback functions */

static int
render_frame(Movie *m, Image *p)
{
  /* do nothing */
  return 1;
}

static int
pause_usec(unsigned int usec)
{
  struct timeval tv;

  /* wait usec, if possible... */
  tv.tv_sec  = usec / 1000000;
  tv.tv_usec = usec % 1000000;
  select(0, NULL, NULL, NULL, &tv);

  return 1;
}

/* methods */

static int
play(Movie *m)
{
  return 0;
}

static int
pause_movie(Movie *m)
{
  return 0;
}

static int
stop(Movie *m)
{
  return 0;
}

static void
destroy(Movie *m)
{
  free(m);
}
