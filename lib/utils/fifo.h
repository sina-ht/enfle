/*
 * fifo.h -- First-In First-Out object header
 * (C)Copyright 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Jun 22 21:24:19 2001.
 * $Id: fifo.h,v 1.1 2001/06/22 17:34:43 sian Exp $
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

#ifndef _FIFO_H
#define _FIFO_H

#ifdef USE_PTHREAD
#include <pthread.h>
#endif

typedef void (*FIFO_destructor)(void *);
typedef struct _fifo_data FIFO_data;
struct _fifo_data {
  FIFO_destructor destructor;
  void *data;
  struct _fifo_data *next;
};

typedef struct _fifo FIFO;
struct _fifo {
#ifdef USE_PTHREAD
  pthread_mutex_t lock;
#endif
  unsigned int ndata;
  unsigned int maxdata;
  struct _fifo_data *next_get;
  struct _fifo_data *last_put;

  int (*put)(FIFO *, void *, FIFO_destructor);
  int (*get)(FIFO *, void **);
  int (*set_max)(FIFO *, unsigned int);
  void (*destroy)(FIFO *);
};

#define fifo_put(f, d, dest) (f)->put((f), (d), (dest))
#define fifo_get(f, d) (f)->get((f), (d))
#define fifo_set_max(f, m) (f)->set_max((f), (m))
#define fifo_destroy(f) (f)->destroy((f))

FIFO *fifo_create(void);

#endif
