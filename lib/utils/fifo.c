/*
 * fifo.c -- First-In First-Out object
 * (C)Copyright 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Jun 22 22:21:49 2001.
 * $Id: fifo.c,v 1.1 2001/06/22 17:34:43 sian Exp $
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

static int put(FIFO *, void *, FIFO_destructor);
static int get(FIFO *, void **);
static int set_max(FIFO *, unsigned int);
static void destroy(FIFO *);

static FIFO template = {
  ndata: 0,
  maxdata: 0,
  next_get: NULL,
  last_put: NULL,
  put: put,
  get: get,
  set_max: set_max,
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
#endif

  return f;
}

/* for internal use */

static inline FIFO_data *
fifo_data_create(void)
{
  return calloc(1, sizeof(FIFO_data));
}

/* methods */

static int
put(FIFO *f, void *d, FIFO_destructor destructor)
{
  FIFO_data *fd;
  int ret = 0;

#ifdef USE_PTHREAD
  pthread_mutex_lock(&f->lock);
#endif

  if (f->maxdata && f->ndata >= f->maxdata)
    goto unlock_and_return;

  if ((fd = fifo_data_create()) == NULL)
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
  pthread_mutex_unlock(&f->lock);
#endif

  return ret;
}

int
get(FIFO *f, void **d_return)
{
  FIFO_data *fd;
  int ret = 0;

#ifdef USE_PTHREAD
  pthread_mutex_lock(&f->lock);
#endif

  if ((fd = f->next_get) == NULL)
    goto unlock_and_return;

  *d_return = fd->data;
  if ((f->next_get = fd->next) == NULL)
    f->last_put = NULL;
  free(fd);
  f->ndata--;

  ret = 1;

 unlock_and_return:
#ifdef USE_PTHREAD
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
destroy(FIFO *f)
{
  FIFO_data *m, *n;

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
