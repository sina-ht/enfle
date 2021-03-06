/*
 * timer_gettimeofday.c -- timer gettimeofday() implementation
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Mar  6 11:57:49 2004.
 *
 * SPDX-License-Identifier: GPL-2.0-only
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
  .impl_data = NULL,

  .create =    create,
  .destroy =   destroy,
  .reset =     NULL,
  .start =     start,
  .pause =     pause,
  .restart =   restart,
  .stop =      stop,
  .get_micro = get_micro,
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
  struct time_data *t = (struct time_data *)timer->impl->impl_data;

  t->base = get_time();
}

static void
pause(Timer *timer)
{
  struct time_data *t = (struct time_data *)timer->impl->impl_data;

  t->now = get_time();
}

static void
restart(Timer *timer)
{
  struct time_data *t = (struct time_data *)timer->impl->impl_data;

  t->base = t->now;
}

static void
stop(Timer *timer)
{
  struct time_data *t = (struct time_data *)timer->impl->impl_data;

  t->now = get_time();
}

static Timer_t
get_micro(Timer *timer)
{
  struct time_data *t = (struct time_data *)timer->impl->impl_data;

  /* If Timer_t is double, this is correct. */
  return (Timer_t)(t->now - t->base);
}
