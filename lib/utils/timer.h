/*
 * timer.h -- timer abstraction layer header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Dec 14 18:48:40 2021.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _TIMER_H
#define _TIMER_H

typedef struct _timer Timer;
typedef enum {
 _TIMER_STOP = 0,
 _TIMER_RUNNING
} TimerStatus;
typedef double Timer_t;

#include "timer_impl.h"

struct _timer {
  Timer_impl *impl;
  Timer_t time;
  Timer_t previous_time;
  TimerStatus status;

  void (*destroy)(Timer *);
  void (*reset)(Timer *);
  void (*start)(Timer *);
  void (*pause)(Timer *);
  void (*restart)(Timer *);
  void (*stop)(Timer *);
  Timer_t (*get_micro)(Timer *);
  Timer_t (*get_difference_micro)(Timer *);
  Timer_t (*get_milli)(Timer *);
  Timer_t (*get_difference_milli)(Timer *);
  Timer_t (*get)(Timer *);
  Timer_t (*get_difference)(Timer *);
};

Timer *enfle_timer_create(Timer_impl *);

#define timer_status(t) (t)->status
#define timer_is_running(t) ((t)->status == _TIMER_RUNNING)

#define timer_destroy(t) (t)->destroy((t))
#define timer_reset(t) (t)->reset((t))
#define timer_start(t) (t)->start((t))
#define timer_pause(t) (t)->pause((t))
#define timer_restart(t) (t)->restart((t))
#define timer_stop(t) (t)->stop((t))
#define timer_get_micro(t) (t)->get_micro((t))
#define timer_get_difference_micro(t) (t)->get_difference_micro((t))
#define timer_get_milli(t) (t)->get_milli((t))
#define timer_get_difference_milli(t) (t)->get_difference_milli((t))
#define timer_get(t) (t)->get((t))
#define timer_get_difference(t) (t)->get_difference((t))

#endif
