/*
 * movie.h -- movie interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Oct 10 05:07:42 2000.
 * $Id: movie.h,v 1.1 2000/10/09 20:19:44 sian Exp $
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

#include "image.h"

typedef struct _movie Movie;
struct _movie {
  int width, height;
  char *format;

  /* These are callback functions which are provided by UI. */
  int (*render_frame)(Movie *, Image *);
  int (*pause_usec)(unsigned int);

  /* These are methods. */
  int (*play)(Movie *);
  int (*pause_movie)(Movie *);
  int (*stop)(Movie *);
  void (*destroy)(Movie *);
};

#define movie_play(m) (m)->play((m))
#define movie_pause(m) (m)->pause_movie((m))
#define movie_stop(m) (m)->stop((m))
#define movie_destroy(m) (m)->destroy((m))

Movie *movie_create(void);

#endif
