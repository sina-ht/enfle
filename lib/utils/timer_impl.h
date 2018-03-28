/*
 * timer_impl.h -- timer implementation layer
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Sep 17 17:24:33 2001.
 *
 * SPDX-License-Identifier: GPL-2.0-only
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
