/*
 * streamer.c -- streamer plugin interface
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Aug 25 07:29:35 2001.
 * $Id: streamer.c,v 1.8 2001/08/26 00:51:21 sian Exp $
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

#include "streamer.h"
#include "streamer-plugin.h"

int
streamer_identify(EnflePlugins *eps, Stream *s, char *filepath)
{
  Dlist *dl;
  Dlist_data *dd;
  PluginList *pl;

  pl = eps->pls[ENFLE_PLUGIN_STREAMER];
  dl = pluginlist_list(pl);
  dlist_iter(dl, dd) {
    Plugin *p;
    StreamerPlugin *stp;
    char *pluginname;

    pluginname = hash_key_key(dlist_data(dd));
    if ((p = pluginlist_get(pl, pluginname)) == NULL)
      fatal(1, "BUG: %s streamer plugin not found but in list.\n", pluginname);
    stp = plugin_get(p);

    if (stp->identify(s, filepath) == STREAM_OK) {
      s->format = strdup(pluginname);
      dlist_move_to_top(dl, dd);
      return 1;
    }
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
