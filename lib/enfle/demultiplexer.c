/*
 * demultiplexer.c -- Demultiplexer abstraction layer
 * (C)Copyright 2001-2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Mar  9 22:53:25 2004.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "demultiplexer.h"
#include "demultiplexer-plugin.h"
#include "utils/misc.h"
#include "utils/libstring.h"

int
demultiplexer_identify(EnflePlugins *eps, Movie *m, Stream *st, Config *c)
{
  PluginList *pl = eps->pls[ENFLE_PLUGIN_DEMULTIPLEXER];
  Plugin *p;
  DemultiplexerPlugin *dp;
  char *ext, *pluginname, **pluginnames;
  int res;
  void *k;
  unsigned int kl;

  if ((ext = misc_str_tolower(misc_get_ext(st->path, 1)))) {
    String *s = string_create();

    string_catf(s, "/enfle/plugins/demultiplexer/assoc/%s", ext);
    pluginnames = config_get_list(c, string_get(s), &res);
    string_destroy(s);
    if (pluginnames) {
      int i = 0;

      while ((pluginname = pluginnames[i])) {
	if (strcmp(pluginname, ".") == 0) {
	  debug_message_fnc("Failed, no further try.\n");
	  free(ext);
	  return 0;
	}
	if ((p = pluginlist_get(pl, pluginname))) {
	  dp = plugin_get(p);
	  stream_rewind(st);
	  //debug_message_fnc("try %s (assoc'd with %s)\n", pluginname, ext);
	  if (dp->identify(st, c, NULL) == DEMULTIPLEX_OK) {
	    m->format = strdup(pluginname);
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

  if (config_get_boolean(c, "/enfle/plugins/demultiplexer/scan_no_assoc", &res)) {
    pluginlist_iter(pl, k, kl, p) {
      dp = plugin_get(p);
      //debug_message("demultiplexer: identify: try %s\n", (char *)k);
      stream_rewind(st);
      if (dp->identify(st, c, NULL) == DEMULTIPLEX_OK) {
	m->format = (char *)k;
	pluginlist_move_to_top;
	return 1;
      }
      //debug_message("demultiplexer: identify: %s: failed\n", (char *)k);
    }
    pluginlist_iter_end;
  }

  return 0;
}

Demultiplexer *
demultiplexer_examine(EnflePlugins *eps, char *pluginname, Movie *m, Stream *st, Config *c)
{
  Plugin *p;
  DemultiplexerPlugin *dp;

  if ((p = pluginlist_get(eps->pls[ENFLE_PLUGIN_DEMULTIPLEXER], pluginname)) == NULL)
    return 0;
  dp = plugin_get(p);

  stream_rewind(st);
  return dp->examine(m, st, c, NULL);
}

Demultiplexer *
_demultiplexer_create(void)
{
  Demultiplexer *demux;

  if ((demux = calloc(1, sizeof(Demultiplexer))) == NULL)
    return NULL;

  return demux;
}

void
_demultiplexer_destroy(Demultiplexer *demux)
{
  free(demux);
}

void
demultiplexer_destroy_packet(void *d)
{
  DemuxedPacket *dp = (DemuxedPacket *)d;

  if (dp) {
    if (dp->data)
      free(dp->data);
    free(dp);
  }
}
