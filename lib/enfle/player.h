/*
 * player.h -- player plugin interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Oct  9 01:54:41 2000.
 * $Id: player.h,v 1.1 2000/10/08 17:38:15 sian Exp $
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

#include "pluginlist.h"
#include "stream.h"
#include "image.h"
#include "player-plugin.h"

typedef struct _player Player;
struct _player {
  PluginList *pl;

  char *(*load)(Player *, Plugin *);
  int (*unload)(Player *, char *);
  int (*identify)(Player *, Image *, Stream *);
  PlayerStatus (*play)(Player *, char *, Image *, Stream *);
  void (*destroy)(Player *);
  Dlist *(*get_names)(Player *);
  unsigned char *(*get_description)(Player *, char *);
  unsigned char *(*get_author)(Player *, char *);
};

#define player_load(l, p) (l)->load((l), (p))
#define player_unload(l, n) (l)->unload((l), (n))
#define player_identify(l, p, s) (l)->identify((l), (p), (s))
#define player_play(l, n, p, s) (l)->play((l), (n), (p), (s))
#define player_destroy(l) (l)->destroy((l))
#define player_get_names(l) (l)->get_names((l))
#define player_get_description(l, n) (l)->get_description((l), (n))
#define player_get_author(l, n) (l)->get_author((l), (n))

Player *player_create(void);

#endif
