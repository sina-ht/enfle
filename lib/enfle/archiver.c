/*
 * archiver.c -- archiver plugin interface
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Mar  9 23:12:54 2004.
 * $Id: archiver.c,v 1.15 2004/03/09 14:16:57 sian Exp $
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

#include "stream.h"
#include "archiver.h"
#include "archiver-plugin.h"
#include "utils/libstring.h"
#include "utils/misc.h"

int
archiver_identify(EnflePlugins *eps, Archive *a, Stream *st, Config *c)
{
  PluginList *pl = eps->pls[ENFLE_PLUGIN_ARCHIVER];
  Plugin *p;
  ArchiverPlugin *arp;
  char *ext, *pluginname, **pluginnames;
  int res;
  void *k;
  unsigned int kl;

  if ((ext = misc_str_tolower(misc_get_ext(st->path, 1)))) {
    String *s = string_create();

    string_catf(s, "/enfle/plugins/archiver/assoc/%s", ext);
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

  if (config_get_boolean(c, "/enfle/plugins/archiver/scan_no_assoc", &res)) {
    pluginlist_iter(pl, k, kl, p) {
      arp = plugin_get(p);
      //debug_message("archiver: identify: try %s\n", (char *)k);
      stream_rewind(st);
      if (arp->identify(a, st, arp->archiver_private) == OPEN_OK) {
	a->format = (char *)k;
	pluginlist_move_to_top;
	return 1;
      }
      //debug_message("archiver: identify: %s: failed\n", (char *)k);
    }
    pluginlist_iter_end;
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
