/*
 * timer_gettimeofday.c -- timer gettimeofday() implementation
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Sep 18 13:53:02 2001.
 * $Id: timer_gettimeofday.c,v 1.4 2001/09/18 05:22:24 sian Exp $
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

#include <sys/time.h>
#include <stdlib.h>

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "timer.h"
#include "timer_impl.h"
#include "timer_gettimeofday.h"

static Timer *create(Timer *);
static void destroy(Timer *);
static void start(Timer *);
static void pause(Timer *);
static void restart(Timer *);
static void stop(Timer *);
static Timer_t get_micro(Timer *);

struct time_data {
  double base;
  double now;
};

static Timer_impl timer_impl_gettimeofday = {
  impl_data: NULL,

  create:    create,
  destroy:   destroy,
  reset:     NULL,
  start:     start,
  pause:     pause,
  restart:   restart,
  stop:      stop,
  get_micro: get_micro,
};

Timer_impl *
timer_gettimeofday(void)
{
  Timer_impl *impl;

  if ((impl = calloc(1, sizeof(Timer_impl))) == NULL)
    return NULL;
  memcpy(impl, &timer_impl_gettimeofday, sizeof(Timer_impl));

  return impl;
}

static Timer *
create(Timer *timer)
{
  if ((timer->impl->impl_data = calloc(1, sizeof(struct time_data))) == NULL) {
    timer->destroy(timer);
    return NULL;
  }

  return timer;
}

static void
destroy(Timer *timer)
{
  if (timer->impl->impl_data)
    free(timer->impl->impl_data);
  free(timer->impl);
}

static double
get_time(void)
{
  struct timeval tv;

  gettimeofday(&tv, NULL);
  return (double)((double)tv.tv_sec * 1.0e6 + (double)(tv.tv_usec));
}

static void
start(Timer *timer)
{
  struct time_data *time = (struct time_data *)timer->impl->impl_data;

  time->base = get_time();
}

static void
pause(Timer *timer)
{
  struct time_data *time = (struct time_data *)timer->impl->impl_data;

  time->now = get_time();
}

static void
restart(Timer *timer)
{
  struct time_data *time = (struct time_data *)timer->impl->impl_data;

  time->base = get_time();
}

static void
stop(Timer *timer)
{
  struct time_data *time = (struct time_data *)timer->impl->impl_data;

  time->now = get_time();
}

static Timer_t
get_micro(Timer *timer)
{
  struct time_data *time = (struct time_data *)timer->impl->impl_data;

  /* If Timer_t is double, this is correct. */
  return (Timer_t)(time->now - time->base);
}
