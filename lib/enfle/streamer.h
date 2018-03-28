/*
 * streamer.h -- streamer header
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Feb  9 12:28:30 2002.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _STREAMER_H
#define _STREAMER_H

#include "stream.h"
#include "utils/libconfig.h"
#include "enfle-plugins.h"

int streamer_identify(EnflePlugins *, Stream *, char *, Config *);
int streamer_open(EnflePlugins *, Stream *, char *, char *);

#endif
