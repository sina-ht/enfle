/*
 * timer.h -- timer abstraction layer header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Oct 29 18:28:02 2000.
 * $Id: timer.h,v 1.2 2000/10/29 18:09:56 sian Exp $
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

#define timer_destroy(t) (t)->destroy((t))
#define timer_reset(t) (t)->reset((t))
#define timer_start(t) (t)->start((t))
#define timer_pause(t) (t)->pause((t))
#define timer_restart(t) (t)->restart((t))
#define timer_stop(t) (t)->stop((t))
#define timer_get_micro(t) (t)->get_micro((t))
#define timer_get_difference_micro(t) (t)->get_micro((t))
#define timer_get_milli(t) (t)->get_milli((t))
#define timer_get_difference_milli(t) (t)->get_milli((t))
#define timer_get(t) (t)->get((t))
#define timer_get_difference(t) (t)->get((t))

#endif
