/*
 * player-plugin.h -- player plugin interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Feb 10 00:03:17 2004.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

// Far from complete...

#ifndef _PLAYER_PLUGIN_H
#define _PLAYER_PLUGIN_H

#include "player-extra.h"
#include "enfle-plugin.h"
#include "stream.h"
#include "movie.h"

typedef struct _player_plugin {
  ENFLE_PLUGIN_COMMON_DATA;

  PlayerStatus (*identify)(Movie *, Stream *, Config *, void *);
  PlayerStatus (*load)(VideoWindow *, Movie *, Stream *, Config *, void *);
} PlayerPlugin;

#define DECLARE_PLAYER_PLUGIN_METHODS \
 static PlayerStatus identify(Movie *, Stream *, Config *, void *); \
 static PlayerStatus load(VideoWindow *, Movie *, Stream *, Config *, void *)

#define DEFINE_PLAYER_PLUGIN_IDENTIFY(m, st, c, priv) \
 static PlayerStatus \
 identify(Movie * m , Stream * st , Config * c , void * priv )
#define DEFINE_PLAYER_PLUGIN_LOAD(vw, m, st, c, priv) \
 static PlayerStatus \
 load(VideoWindow * vw , Movie * m , Stream * st , Config * c , void * priv )

#ifndef STATIC
ENFLE_PLUGIN_ENTRIES;
#endif

#endif
