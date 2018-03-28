/*
 * effect.h -- Effect header
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Jul  7 23:05:07 2002.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _EFFECT_H
#define _EFFECT_H

#include "enfle-plugins.h"
#include "image.h"

int effect_call(EnflePlugins *, char *, Image *, int, int);

#endif
