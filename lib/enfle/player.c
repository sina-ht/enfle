/*
 * player.c -- player plugin interface
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Jul  3 20:26:02 2001.
 * $Id: player.c,v 1.12 2001/07/10 12:59:45 sian Exp $
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

#define REQUIRE_FATAL
#include "common.h"

#include "stream.h"
#include "player.h"

int
player_identify(EnflePlugins *eps, Movie *m, Stream *st, Config *c)
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

    pluginname = hash_key_key(dlist_data(dd));
    if ((p = pluginlist_get(pl, pluginname)) == NULL)
      fatal(1, "BUG: %s player plugin not found but in list.\n", pluginname);
    lp = plugin_get(p);

    stream_rewind(st);
    if (lp->identify(m, st, c, NULL) == PLAY_OK) {
      m->format = pluginname;
      dlist_move_to_top(dl, dd);
      return 1;
    }
  }

  return 0;
}

PlayerStatus
player_load(EnflePlugins *eps, VideoWindow *vw, char *pluginname, Movie *m, Stream *st, Config *c)
{
  Plugin *p;
  PlayerPlugin *pp;

  if ((p = pluginlist_get(eps->pls[ENFLE_PLUGIN_PLAYER], pluginname)) == NULL)
    return 0;
  pp = plugin_get(p);

  stream_rewind(st);
  return pp->load(vw, m, st, c, NULL);
}
