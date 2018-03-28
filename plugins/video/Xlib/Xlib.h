/*
 * Xlib.h -- Xlib Video plugin
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Aug 11 06:05:19 2001.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _XLIB_H
#define _XLIB_H

typedef struct {
  X11 *x11;
  VideoWindow *root;
  Config *c;
} Xlib_info;

#endif
