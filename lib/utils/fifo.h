/*
 * fifo.h -- First-In First-Out object header
 * (C)Copyright 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Jan 12 06:24:27 2004.
 *
 * SPDX-License-Identifier: GPL-2.0-only
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
  pthread_cond_t lock_cond;
  //pthread_cond_t put_ok_cond;
  //pthread_cond_t get_ok_cond;
  int valid;
#endif
  unsigned int ndata;
  unsigned int maxdata;
  struct _fifo_data *next_get;
  struct _fifo_data *last_put;

  int (*put)(FIFO *, void *, FIFO_destructor);
  int (*get)(FIFO *, void **, FIFO_destructor *);
  int (*set_max)(FIFO *, unsigned int);
  void (*invalidate)(FIFO *);
  void (*destroy)(FIFO *);
};

#define fifo_is_empty(f) ((f)->ndata == 0)
#define fifo_is_full(f) ((f)->maxdata > 0 && (f)->ndata >= (f)->maxdata)
#define fifo_ndata(f) (f)->ndata

#define fifo_put(f, d, dest) (f)->put((f), (d), (dest))
#define fifo_get(f, d, dest_r) (f)->get((f), (d), (dest_r))
#define fifo_set_max(f, m) (f)->set_max((f), (m))
#define fifo_invalidate(f) (f)->invalidate((f))
#define fifo_destroy(f) (f)->destroy((f))

FIFO *fifo_create(void);

#endif
