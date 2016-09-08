/*
 * timer.c -- timer implementation
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Mar  6 11:59:09 2004.
 * $Id: timer.c,v 1.9 2004/03/06 03:43:36 sian Exp $
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "timer.h"

static void destroy(Timer *);
static void reset(Timer *);
static void start(Timer *);
static void pause(Timer *);
static void restart(Timer *);
static void stop(Timer *);
static Timer_t get_micro(Timer *);
static Timer_t get_difference_micro(Timer *);
static Timer_t get_milli(Timer *);
static Timer_t get_difference_milli(Timer *);
static Timer_t get(Timer *);
static Timer_t get_difference(Timer *);

static Timer template = {
  .impl = NULL,
  .status = _TIMER_STOP,

  .destroy = destroy,
  .reset = reset,
  .start = start,
  .pause = pause,
  .restart = restart,
  .stop = stop,
  .get_micro = get_micro,
  .get_difference_micro = get_difference_micro,
  .get_milli = get_milli,
  .get_difference_milli = get_difference_milli,
  .get = get,
  .get_difference = get_difference
};

Timer *
enfle_timer_create(Timer_impl *impl)
{
  Timer *timer;

  if ((timer = calloc(1, sizeof(Timer))) == NULL)
    return NULL;
  memcpy(timer, &template, sizeof(Timer));

  timer->impl = impl;

  return timer->impl->create(timer);
}

static void
destroy(Timer *timer)
{
  timer->impl->destroy(timer);
  free(timer);
}

static void
reset(Timer *timer)
{
  timer->time = 0;
  timer->previous_time = 0;
  timer->status = _TIMER_STOP;
}

static void
start(Timer *timer)
{
  if (timer->status == _TIMER_RUNNING)
    return;
  reset(timer);
  timer->impl->start(timer);
  timer->status = _TIMER_RUNNING;
}

static void
pause(Timer *timer)
{
  if (timer->status == _TIMER_STOP)
    return;
  timer->status = _TIMER_STOP;
  timer->previous_time = timer->time;
  timer->impl->pause(timer);
  timer->time += timer->impl->get_micro(timer);
}

static void
restart(Timer *timer)
{
  if (timer->status == _TIMER_RUNNING)
    return;
  timer->status = _TIMER_RUNNING;
  timer->impl->restart(timer);
}

static void
stop(Timer *timer)
{
  if (timer->status == _TIMER_STOP)
    return;
  timer->status = _TIMER_STOP;
  timer->previous_time = timer->time;
  timer->impl->stop(timer);
  timer->time += timer->impl->get_micro(timer);
}

static Timer_t
get_micro(Timer *timer)
{
  Timer_t t;

  pause(timer);
  t = timer->time;
  restart(timer);

  return t;
}

static Timer_t
get_difference_micro(Timer *timer)
{
  return timer->time - timer->previous_time;
}

static Timer_t
get_milli(Timer *timer)
{
  Timer_t t;

  pause(timer);
  t = timer->time / 1000;
  restart(timer);

  return t;
}

static Timer_t
get_difference_milli(Timer *timer)
{
  return get_difference_micro(timer) / 1000;
}

static Timer_t
get(Timer *timer)
{
  Timer_t t;

  pause(timer);
  t = timer->time / 1000000;
  restart(timer);

  return t;
}

static Timer_t
get_difference(Timer *timer)
{
  return get_difference_milli(timer) / 1000;
}
