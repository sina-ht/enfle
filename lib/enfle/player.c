/*
 * player.c -- player plugin interface
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Oct 12 23:35:42 2001.
 * $Id: player.c,v 1.13 2001/10/14 12:32:37 sian Exp $
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

#include "player.h"
#include "player-plugin.h"
#include "utils/misc.h"
#include "utils/libstring.h"

int
player_identify(EnflePlugins *eps, Movie *m, Stream *st, Config *c)
{
  Dlist *dl;
  Dlist_data *dd;
  PluginList *pl;
  Plugin *p;
  PlayerPlugin *pp;
  char *ext, *pluginname;

  pl = eps->pls[ENFLE_PLUGIN_PLAYER];

  if ((ext = misc_get_ext(st->path, 1))) {
    String *s;

    s = string_create();
    string_catf(s, "/enfle/plugins/player/assoc/%s", ext);
    pluginname = config_get_str(c, string_get(s));
    string_destroy(s);
    if (pluginname) {
      if ((p = pluginlist_get(pl, pluginname))) {
	pp = plugin_get(p);
	stream_rewind(st);
	debug_message(__FUNCTION__ ": try %s (assoc'd with %s)\n", pluginname, ext);
	free(ext);
	if (pp->identify(m, st, c, NULL) == PLAY_OK) {
	  m->format = pluginname;
	  return 1;
	}
	debug_message(__FUNCTION__ ": %s failed.\n", pluginname);
	return 0;
      } else {
	show_message(__FUNCTION__ ": %s (assoc'd with %s) not found.\n", pluginname, ext);
      }
    }
    free(ext);
  }

  dl = pluginlist_list(pl);
  dlist_iter(dl, dd) {
    pluginname = hash_key_key(dlist_data(dd));
    if ((p = pluginlist_get(pl, pluginname)) == NULL)
      fatal(1, "BUG: %s player plugin not found but in list.\n", pluginname);
    pp = plugin_get(p);

    stream_rewind(st);
    if (pp->identify(m, st, c, NULL) == PLAY_OK) {
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
