/*
 * fifo.c -- First-In First-Out object
 * (C)Copyright 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Nov  7 01:03:47 2003.
 * $Id: fifo.c,v 1.11 2003/11/08 06:15:29 sian Exp $
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

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"
#include "fifo.h"

typedef struct _fifo_data FIFO_data;
struct _fifo_data {
  FIFO_destructor destructor;
  void *data;
  struct _fifo_data *next;
};

#define FIFO_DEFAULT_MAXDATA 100

static int put(FIFO *, void *, FIFO_destructor);
static int get(FIFO *, void **, FIFO_destructor *);
static int set_max(FIFO *, unsigned int);
static void emptify(FIFO *);
static void destroy(FIFO *);

static FIFO template = {
  ndata: 0,
  maxdata: FIFO_DEFAULT_MAXDATA,
  next_get: NULL,
  last_put: NULL,

  put: put,
  get: get,
  set_max: set_max,
  emptify: emptify,
  destroy: destroy
};

FIFO *
fifo_create(void)
{
  FIFO *f;

  if ((f = malloc(sizeof(FIFO))) == NULL)
    return NULL;
  memcpy(f, &template, sizeof(FIFO));

#ifdef USE_PTHREAD
  pthread_mutex_init(&f->lock, NULL);
  f->valid = 1;
#endif

  return f;
}

/* methods */

static int
put(FIFO *f, void *d, FIFO_destructor destructor)
{
  FIFO_data *fd;
  int ret = 0;

#ifdef USE_PTHREAD
  pthread_mutex_lock(&f->lock);
  if (f->maxdata)
    while (f->valid && f->ndata >= f->maxdata)
      pthread_cond_wait(&f->put_ok_cond, &f->lock);
  if (!f->valid) {
    pthread_mutex_unlock(&f->lock);
    return 0;
  }
#else
  if (f->maxdata && f->ndata >= f->maxdata)
    return 0;
#endif

  if ((fd = calloc(1, sizeof(FIFO_data))) == NULL)
    goto unlock_and_return;

  if (f->last_put)
    f->last_put->next = fd;
  else
    f->next_get = fd;

  fd->data = d;
  fd->destructor = destructor;

  f->last_put = fd;
  f->ndata++;

  ret = 1;

 unlock_and_return:
#ifdef USE_PTHREAD
  if (f->ndata > 0)
    pthread_cond_signal(&f->get_ok_cond);
  pthread_mutex_unlock(&f->lock);
#endif

  return ret;
}

int
get(FIFO *f, void **d_return, FIFO_destructor *destructor_r)
{
  FIFO_data *fd;
  int ret = 0;

#ifdef USE_PTHREAD
  pthread_mutex_lock(&f->lock);
  while (f->valid && f->ndata == 0)
    pthread_cond_wait(&f->get_ok_cond, &f->lock);
  if (!f->valid) {
    pthread_mutex_unlock(&f->lock);
    return 0;
  }
#endif
  if ((fd = f->next_get) == NULL)
    goto unlock_and_return2;

  *d_return = fd->data;
  *destructor_r = fd->destructor;
  if ((f->next_get = fd->next) == NULL)
    f->last_put = NULL;
  free(fd);
  f->ndata--;

  ret = 1;

 unlock_and_return2:
#ifdef USE_PTHREAD
  if (f->maxdata && f->ndata < f->maxdata)
    pthread_cond_signal(&f->put_ok_cond);
  pthread_mutex_unlock(&f->lock);
#endif

  return ret;
}

static int
set_max(FIFO *f, unsigned int m)
{
  if (!f)
    return 0;
  f->maxdata = m;

  return 1;
}

static void
emptify(FIFO *f)
{
  void *p;
  FIFO_destructor dest;

  debug_message_fnc("%d items (before emptify)\n", fifo_ndata(f));
  while (!fifo_is_empty(f)) {
    fifo_get(f, &p, &dest);
    dest(p);
  }
  debug_message_fnc("%d items left (after emptify)\n", fifo_ndata(f));
}

static void
destroy(FIFO *f)
{
  FIFO_data *m, *n;

#ifdef USE_PTHREAD
  f->valid = 0;
  pthread_mutex_lock(&f->lock);
  pthread_cond_signal(&f->put_ok_cond);
  pthread_cond_signal(&f->get_ok_cond);
  pthread_cond_destroy(&f->put_ok_cond);
  pthread_cond_destroy(&f->get_ok_cond);
  pthread_mutex_unlock(&f->lock);
  pthread_mutex_destroy(&f->lock);
#endif

  if (f) {
    for (m = f->next_get; m; m = n) {
      n = m->next;
      if (m->destructor)
	m->destructor(m->data);
      free(m);
    }
    free(f);
  }
}
