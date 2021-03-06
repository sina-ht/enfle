/*
 * loader.c -- loader plugin interface
 * (C)Copyright 2000-2007 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue May 29 00:22:06 2007.
 *
 * SPDX-License-Identifier: GPL-2.0-only
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
  PluginList *pl = eps->pls[ENFLE_PLUGIN_LOADER];
  Plugin *p;
  LoaderPlugin *lp;
  char *ext, *pluginname, **pluginnames;
  int res;
  void *k;
  unsigned int kl;

  if ((ext = misc_str_tolower(misc_get_ext(st->path, 1)))) {
    String *s = string_create();

    string_catf(s, "/enfle/plugins/loader/assoc/%s", ext);
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
	  if ((lp = plugin_get(p)) == NULL) {
	    show_message_fnc("loader plugin %s autoloading failed.\n", pluginname);
	    pluginlist_delete(pl, pluginname);
	    break;
	  }

	  stream_rewind(st);
	  //debug_message_fnc("try %s (assoc'd with %s)\n", pluginname, ext);
	  ip->format_detail = NULL;
	  if (lp->identify(ip, st, vw, c, lp->image_private) == LOAD_OK) {
	    ip->format = pluginname;
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

  ip->format_detail = NULL;
  if (config_get_boolean(c, "/enfle/plugins/loader/scan_no_assoc", &res)) {
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
