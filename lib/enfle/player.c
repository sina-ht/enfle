/*
 * player.c -- player plugin interface
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Oct 21 02:55:48 2000.
 * $Id: player.c,v 1.5 2000/10/20 18:13:39 sian Exp $
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

#include <stdio.h>
#include <stdlib.h>

#include "common.h"

#include "stream.h"
#include "player.h"

static int identify(EnflePlugins *, Movie *, Stream *);
static PlayerStatus load_movie(EnflePlugins *, VideoWindow *, VideoPlugin *, char *, Movie *, Stream *);

static Player template = {
  identify: identify,
  load_movie: load_movie
};

Player *
player_create(void)
{
  Player *p;

  if ((p = (Player *)calloc(1, sizeof(Player))) == NULL)
    return NULL;
  memcpy(p, &template, sizeof(Player));

  return p;
}

/* methods */

static int
identify(EnflePlugins *eps, Movie *m, Stream *st)
{
  Dlist *dl;
  Dlist_data *dd;
  PluginList *pl;

  pl = eps->pls[ENFLE_PLUGIN_PLAYER];
  dl = pluginlist_list(pl);
  dlist_iter(dl, dd) {
    Plugin *p;
    PlayerPlugin *lp;
    char *pluginname;

    pluginname = dlist_data(dd);
    if ((p = pluginlist_get(pl, pluginname)) == NULL) {
      fprintf(stderr, "BUG: %s player plugin not found but in list.\n", pluginname);
      exit(-1);
    }
    lp = plugin_get(p);

    stream_rewind(st);
    if (lp->identify(m, st) == PLAY_OK) {
      m->format = pluginname;
      dlist_move_to_top(dl, dd);
      return 1;
    }
  }

  return 0;
}

static PlayerStatus
load_movie(EnflePlugins *eps, VideoWindow *vw, VideoPlugin *vp, char *pluginname, Movie *m, Stream *st)
{
  Plugin *p;
  PlayerPlugin *pp;

  if ((p = pluginlist_get(eps->pls[ENFLE_PLUGIN_PLAYER], pluginname)) == NULL)
    return 0;
  pp = plugin_get(p);

  stream_rewind(st);
  return pp->load(vw, vp, m, st);
}
