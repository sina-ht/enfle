/*
 * timer_impl.h -- timer implementation layer
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Sep 17 17:24:33 2001.
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

#ifndef _TIMER_IMPL_H
#define _TIMER_IMPL_H

typedef struct _timer_impl {
  void *impl_data;

  Timer *(*create)(Timer *);
  void (*destroy)(Timer *);
  void (*reset)(Timer *);
  void (*start)(Timer *);
  void (*pause)(Timer *);
  void (*restart)(Timer *);
  void (*stop)(Timer *);
  Timer_t (*get_micro)(Timer *);
} Timer_impl;

#endif
