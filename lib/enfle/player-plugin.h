/*
 * player-plugin.h -- player plugin interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Nov  4 04:58:41 2000.
 * $Id: player-plugin.h,v 1.6 2000/11/04 17:31:28 sian Exp $
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

// Far from complete...

#ifndef _PLAYER_PLUGIN_H
#define _PLAYER_PLUGIN_H

typedef enum _player_status PlayerStatus;

#include "enfle-plugin.h"
#include "stream.h"
#include "movie.h"

enum _player_status {
  PLAY_ERROR = -1,
  PLAY_NOT,
  PLAY_OK
};

typedef struct _player_plugin {
  ENFLE_PLUGIN_COMMON_DATA;

  PlayerStatus (*identify)(Movie *, Stream *);
  PlayerStatus (*load)(VideoWindow *, Movie *, Stream *);
} PlayerPlugin;

ENFLE_PLUGIN_ENTRIES;

#endif
