/*
 * loader.c -- loader plugin interface
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Feb  8 21:11:28 2002.
 * $Id: loader.c,v 1.20 2002/02/08 12:15:01 sian Exp $
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
#define REQUIRE_STRING_H
#include "compat.h"

#include "loader.h"
#include "loader-plugin.h"
#include "utils/misc.h"
#include "utils/libstring.h"

int
loader_identify(EnflePlugins *eps, Image *ip, Stream *st, VideoWindow *vw, Config *c)
{
  Dlist *dl;
  Dlist_data *dd;
  PluginList *pl;
  Plugin *p;
  LoaderPlugin *lp;
  char *ext, *pluginname, **pluginnames;
  int res;

  pl = eps->pls[ENFLE_PLUGIN_LOADER];

  if ((ext = misc_str_tolower(misc_get_ext(st->path, 1)))) {
    String *s;

    s = string_create();
    string_catf(s, "/enfle/plugins/loader/assoc/%s", ext);
    pluginnames = config_get_list(c, string_get(s), &res);
    string_destroy(s);
    if (pluginnames) {
      int i;

      i = 0;
      while ((pluginname = pluginnames[i])) {
	if ((p = pluginlist_get(pl, pluginname))) {
	  lp = plugin_get(p);
	  stream_rewind(st);
	  debug_message_fnc("try %s (assoc'd with %s)\n", pluginname, ext);
	  if (lp->identify(ip, st, vw, c, lp->image_private) == LOAD_OK) {
	    ip->format = strdup(pluginname);
	    free(ext);
	    misc_free_str_array(pluginnames);
	    return 1;
	  }
	  debug_message_fnc("%s failed.\n", pluginname);
	} else {
	  show_message_fnc("%s (assoc'd with %s) not found.\n", pluginname, ext);
	}
	i++;
      }
    }
    free(ext);
  }

  dl = pluginlist_list(pl);
  ip->format_detail = NULL;
  dlist_iter(dl, dd) {
    pluginname = hash_key_key(dlist_data(dd));
    if ((p = pluginlist_get(pl, pluginname)) == NULL)
      bug(1, "%s loader plugin not found but in list.\n", pluginname);
    lp = plugin_get(p);

    stream_rewind(st);
    //debug_message_fnc("try %s\n", pluginname);
    if (lp->identify(ip, st, vw, c, lp->image_private) == LOAD_OK) {
      ip->format = pluginname;
      dlist_move_to_top(dl, dd);
      return 1;
    }
  }

  return 0;
}

LoaderStatus
loader_load(EnflePlugins *eps, char *pluginname, Image *ip, Stream *st, VideoWindow *vw, Config *c)
{
  Plugin *p;
  LoaderPlugin *lp;

  if ((p = pluginlist_get(eps->pls[ENFLE_PLUGIN_LOADER], pluginname)) == NULL)
    return 0;
  lp = plugin_get(p);

  ip->format_detail = NULL;
  stream_rewind(st);
  return lp->load(ip, st, vw, c, lp->image_private);
}
