/*
 * player.h -- player plugin interface header
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Oct 12 23:35:19 2001.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _PLAYER_H
#define _PLAYER_H

#include "enfle-plugins.h"
#include "stream.h"
#include "movie.h"
#include "video.h"

int player_identify(EnflePlugins *, Movie *, Stream *, Config *);
PlayerStatus player_load(EnflePlugins *, VideoWindow *, char *, Movie *, Stream *, Config *);

#endif
