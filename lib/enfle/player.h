/*
 * player.h -- player plugin interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Nov  4 04:59:29 2000.
 * $Id: player.h,v 1.7 2000/11/04 17:31:28 sian Exp $
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
#include "video.h"
#include "player-plugin.h"

struct _player {
  int (*identify)(EnflePlugins *, Movie *, Stream *);
  PlayerStatus (*load_movie)(EnflePlugins *, VideoWindow *, char *, Movie *, Stream *);
};

#define player_identify(l, eps, p, s) (l)->identify((eps), (p), (s))
#define player_load_movie(l, eps, vw, n, p, s) (l)->load_movie((eps), (vw), (n), (p), (s))
#define player_destroy(p) if ((p)) free((p))

Player *player_create(void);

#endif
