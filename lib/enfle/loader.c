/*
 * loader.c -- loader plugin interface
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Thu Aug  8 00:17:51 2002.
 * $Id: loader.c,v 1.24 2002/08/07 15:34:20 sian Exp $
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

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "loader.h"
#include "loader-plugin.h"
#include "utils/misc.h"
#include "utils/libstring.h"

int
loader_identify(EnflePlugins *eps, Image *ip, Stream *st, VideoWindow *vw, Config *c)
{
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
	  ip->format_detail = NULL;
	  if (lp->identify(ip, st, vw, c, lp->image_private) == LOAD_OK) {
	    ip->format = strdup(pluginname);
	    free(ext);
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

  {
    void *k;
    unsigned int kl;

    ip->format_detail = NULL;
    pluginlist_iter(pl, k, kl, p) {
      lp = plugin_get(p);
      //debug_message("loader: identify: try %s\n", (char *)k);
      stream_rewind(st);
      if (lp->identify(ip, st, vw, c, lp->image_private) == LOAD_OK) {
	ip->format = (char *)k;
	pluginlist_move_to_top;
	return 1;
      }
      //debug_message("loader: identify: %s: failed\n", (char *)k);
    }
    pluginlist_iter_end;
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
