/*
 * rgbparse.h -- RGB database parser header
 * (C)Copyright 1999, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Feb 15 03:53:27 2002.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _RGBPARSE_H
#define _RGBPARSE_H

#include "utils/hash.h"

typedef struct {
  int n;
  int c[3];
} NColor;

Hash *rgbparse(char *);

#endif
