/*
 * player-extra.h -- player plugin extra definition
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Thu Jan  4 04:09:44 2001.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _PLAYER_EXTRA_H
#define _PLAYER_EXTRA_H

enum _player_status {
  PLAY_ERROR = -1,
  PLAY_NOT,
  PLAY_OK
};

typedef enum _player_status PlayerStatus;

#endif
