/*
 * archiver.c -- archiver plugin interface
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Feb  9 12:31:29 2002.
 * $Id: archiver.c,v 1.11 2002/02/09 03:45:28 sian Exp $
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
#define REQUIRE_FATAL
#include "common.h"

#include "stream.h"
#include "archiver.h"
#include "archiver-plugin.h"
#include "utils/libstring.h"
#include "utils/misc.h"

int
archiver_identify(EnflePlugins *eps, Archive *a, Stream *st, Config *c)
{
  Dlist *dl;
  Dlist_data *dd;
  PluginList *pl;
  Plugin *p;
  ArchiverPlugin *arp;
  char *ext, *pluginname, **pluginnames;
  int res;

  pl = eps->pls[ENFLE_PLUGIN_ARCHIVER];

  if ((ext = misc_str_tolower(misc_get_ext(st->path, 1)))) {
    String *s;

    s = string_create();
    string_catf(s, "/enfle/plugins/archiver/assoc/%s", ext);
    pluginnames = config_get_list(c, string_get(s), &res);
    string_destroy(s);
    if (pluginnames) {
      int i;

      i = 0;
      while ((pluginname = pluginnames[i])) {
	if ((p = pluginlist_get(pl, pluginname))) {
	  arp = plugin_get(p);
	  debug_message_fnc("try %s (assoc'd with %s)\n", pluginname, ext);
	  stream_rewind(st);
	  if (arp->identify(a, st, arp->archiver_private) == OPEN_OK) {
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

  dl = pluginlist_list(pl);
  dlist_iter(dl, dd) {
    pluginname = hash_key_key(dlist_data(dd));
    if ((p = pluginlist_get(pl, pluginname)) == NULL)
      bug(1, "%s archiver plugin not found but in list.\n", pluginname);
    arp = plugin_get(p);

    //debug_message("archiver: identify: try %s\n", pluginname);
    stream_rewind(st);
    if (arp->identify(a, st, arp->archiver_private) == OPEN_OK) {
      a->format = pluginname;
      dlist_move_to_top(dl, dd);
      return 1;
    }

    //debug_message("archiver: identify: %s: failed\n", pluginname);
  }

  return 0;
}

ArchiverStatus
archiver_open(EnflePlugins *eps, Archive *a, char *pluginname, Stream *st)
{
  Plugin *p;
  ArchiverPlugin *arp;

  if ((p = pluginlist_get(eps->pls[ENFLE_PLUGIN_ARCHIVER], pluginname)) == NULL)
    return 0;
  arp = plugin_get(p);

  stream_rewind(st);
  return arp->open(a, st, arp->archiver_private);
}
