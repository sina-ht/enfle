/*
 * streamer.c -- streamer plugin interface
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Mar  9 23:12:36 2004.
 * $Id: streamer.c,v 1.13 2004/03/09 14:16:57 sian Exp $
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

#include "streamer.h"
#include "streamer-plugin.h"
#include "utils/libstring.h"
#include "utils/misc.h"

int
streamer_identify(EnflePlugins *eps, Stream *st, char *filepath, Config *c)
{
  PluginList *pl = eps->pls[ENFLE_PLUGIN_STREAMER];
  Plugin *p;
  StreamerPlugin *stp;
  char *ext, *pluginname, **pluginnames;
  int res;
  void *k;
  unsigned int kl;

  if ((ext = misc_str_tolower(misc_get_ext(filepath, 1)))) {
    String *s = string_create();

    string_catf(s, "/enfle/plugins/streamer/assoc/%s", ext);
    pluginnames = config_get_list(c, string_get(s), &res);
    string_destroy(s);
    if (pluginnames) {
      int i = 0;

      while ((pluginname = pluginnames[i])) {
	if (strcmp(pluginname, ".") == 0) {
	  debug_message_fnc("Failed, no further try.\n");
	  return 0;
	}
	if ((p = pluginlist_get(pl, pluginname))) {
	  stp = plugin_get(p);
	  debug_message_fnc("try %s (assoc'd with %s)\n", pluginname, ext);
	  if (stp->identify(st, filepath) == STREAM_OK) {
	    st->format = strdup(pluginname);
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

  if (config_get_boolean(c, "/enfle/plugins/streamer/scan_no_assoc", &res)) {
    pluginlist_iter(pl, k, kl, p) {
      stp = plugin_get(p);
      //debug_message("streamer: identify: try %s\n", (char *)k);
      if (stp->identify(st, filepath) == STREAM_OK) {
	st->format = (char *)k;
	pluginlist_move_to_top;
	return 1;
      }
      //debug_message("streamer: identify: %s: failed\n", (char *)k);
    }
    pluginlist_iter_end;
  }

  return 0;
}

int
streamer_open(EnflePlugins *eps, Stream *s, char *pluginname, char *filepath)
{
  Plugin *p;
  StreamerPlugin *stp;

  if ((p = pluginlist_get(eps->pls[ENFLE_PLUGIN_STREAMER], pluginname)) == NULL)
    return 0;
  stp = plugin_get(p);

  s->path = strdup(filepath);
  return stp->open(s, filepath);
}
