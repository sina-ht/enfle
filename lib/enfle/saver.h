/*
 * saver.h -- Saver header
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Jul  3 20:19:46 2001.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _SAVER_H
#define _SAVER_H

#include "enfle-plugins.h"
#include "utils/libconfig.h"
#include "image.h"

int saver_save(EnflePlugins *, char *, Image *, FILE *, Config *, void *);
char *saver_get_ext(EnflePlugins *, char *, Config *);

#endif
