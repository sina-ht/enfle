/*
 * player.h -- player plugin interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Oct 17 22:48:09 2000.
 * $Id: player.h,v 1.4 2000/10/17 14:04:01 sian Exp $
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

#ifndef _PLAYER_H
#define _PLAYER_H

typedef struct _player Player;

#include "enfle-plugins.h"
#include "stream.h"
#include "movie.h"
#include "player-plugin.h"

struct _player {
  int (*identify)(EnflePlugins *, Movie *, Stream *);
  PlayerStatus (*load_movie)(EnflePlugins *, UIData *, char *, Movie *, Stream *);
};

#define player_identify(l, eps, p, s) (l)->identify((eps), (p), (s))
#define player_load_movie(l, eps, u, n, p, s) (l)->load_movie((eps), (u), (n), (p), (s))

Player *player_create(void);

#endif
