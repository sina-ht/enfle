/*
 * fifo.h -- First-In First-Out object header
 * (C)Copyright 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Thu Sep 20 14:25:17 2001.
 * $Id: fifo.h,v 1.3 2001/09/20 05:29:03 sian Exp $
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

typedef struct _fifo FIFO;
struct _fifo {
#ifdef USE_PTHREAD
  pthread_mutex_t lock;
  pthread_cond_t put_ok_cond;
  pthread_cond_t get_ok_cond;
#endif
  unsigned int ndata;
  unsigned int maxdata;
  struct _fifo_data *next_get;
  struct _fifo_data *last_put;

  int (*put)(FIFO *, void *, FIFO_destructor);
  int (*get)(FIFO *, void **, FIFO_destructor *);
  int (*set_max)(FIFO *, unsigned int);
  void (*destroy)(FIFO *);
};

#define fifo_is_empty(f) ((f)->ndata == 0)
#define fifo_is_full(f) ((f)->maxdata > 0 && (f)->ndata >= (f)->maxdata)

#define fifo_put(f, d, dest) (f)->put((f), (d), (dest))
#define fifo_get(f, d, dest_r) (f)->get((f), (d), (dest_r))
#define fifo_set_max(f, m) (f)->set_max((f), (m))
#define fifo_destroy(f) (f)->destroy((f))

FIFO *fifo_create(void);

#endif
