/*
 * player.h -- player plugin interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Thu Oct 12 19:18:03 2000.
 * $Id: player.h,v 1.3 2000/10/12 15:47:02 sian Exp $
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

#include "pluginlist.h"
#include "stream.h"
#include "movie.h"
#include "player-plugin.h"

struct _player {
  PluginList *pl;

  char *(*load)(Player *, Plugin *);
  int (*unload)(Player *, char *);
  int (*identify)(Player *, Movie *, Stream *);
  PlayerStatus (*load_movie)(Player *, UIData *, char *, Movie *, Stream *);
  void (*destroy)(Player *);
  Dlist *(*get_names)(Player *);
  unsigned char *(*get_description)(Player *, char *);
  unsigned char *(*get_author)(Player *, char *);
};

#define player_load(l, p) (l)->load((l), (p))
#define player_unload(l, n) (l)->unload((l), (n))
#define player_identify(l, p, s) (l)->identify((l), (p), (s))
#define player_load_movie(l, u, n, p, s) (l)->load_movie((l), (u), (n), (p), (s))
#define player_destroy(l) (l)->destroy((l))
#define player_get_names(l) (l)->get_names((l))
#define player_get_description(l, n) (l)->get_description((l), (n))
#define player_get_author(l, n) (l)->get_author((l), (n))

Player *player_create(void);

#endif
