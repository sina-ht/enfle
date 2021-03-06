/*
 * player.c -- player plugin interface
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Mar  9 22:53:18 2004.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "player.h"
#include "player-plugin.h"
#include "utils/misc.h"
#include "utils/libstring.h"

int
player_identify(EnflePlugins *eps, Movie *m, Stream *st, Config *c)
{
  PluginList *pl = eps->pls[ENFLE_PLUGIN_PLAYER];
  Plugin *p;
  PlayerPlugin *pp;
  char *ext, *pluginname, **pluginnames;
  int res;
  void *k;
  unsigned int kl;

  if ((ext = misc_get_ext(st->path, 1))) {
    String *s = string_create();

    string_catf(s, "/enfle/plugins/player/assoc/%s", ext);
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
	  pp = plugin_get(p);
	  stream_rewind(st);
	  debug_message_fnc("try %s (assoc'd with %s)\n", pluginname, ext);
	  if (pp->identify(m, st, c, eps) == PLAY_OK) {
	    m->player_name = strdup(pluginname);
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

  if (config_get_boolean(c, "/enfle/plugins/player/scan_no_assoc", &res)) {
    pluginlist_iter(pl, k, kl, p) {
      pp = plugin_get(p);
      //debug_message("player: identify: try %s\n", (char *)k);
      stream_rewind(st);
      if (pp->identify(m, st, c, eps) == PLAY_OK) {
	m->player_name = (char *)k;
	pluginlist_move_to_top;
	return 1;
      }
      //debug_message("player: identify: %s: failed\n", (char *)k);
    }
    pluginlist_iter_end;
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
  return pp->load(vw, m, st, c, eps);
}
