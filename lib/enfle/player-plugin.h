/*
 * player-plugin.h -- player plugin interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Thu Oct 12 19:16:02 2000.
 * $Id: player-plugin.h,v 1.3 2000/10/12 15:47:02 sian Exp $
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
  PlayerStatus (*load)(UIData *, Movie *, Stream *);
} PlayerPlugin;

void *plugin_entry(void);
void plugin_exit(void *);

#endif
