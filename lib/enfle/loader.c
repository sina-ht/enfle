/*
 * loader.c -- loader plugin interface
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Jun 18 20:24:20 2001.
 * $Id: loader.c,v 1.12 2001/06/18 16:23:47 sian Exp $
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

#include <stdio.h>
#include <stdlib.h>

#define REQUIRE_FATAL
#include "common.h"

#include "loader.h"
#include "loader-plugin.h"

static int identify(EnflePlugins *, Image *, Stream *, VideoWindow *, Config *);
static LoaderStatus load_image(EnflePlugins *, char *, Image *, Stream *, VideoWindow *, Config *);

static Loader template = {
  identify: identify,
  load_image: load_image
};

Loader *
loader_create(void)
{
  Loader *l;

  if ((l = (Loader *)calloc(1, sizeof(Loader))) == NULL)
    return NULL;
  memcpy(l, &template, sizeof(Loader));

  return l;
}

/* methods */

static int
identify(EnflePlugins *eps, Image *ip, Stream *st, VideoWindow *vw, Config *c)
{
  Dlist *dl;
  Dlist_data *dd;
  PluginList *pl;

  pl = eps->pls[ENFLE_PLUGIN_LOADER];
  dl = pluginlist_list(pl);
  ip->format_detail = NULL;
  dlist_iter(dl, dd) {
    Plugin *p;
    LoaderPlugin *lp;
    char *pluginname;

    pluginname = hash_key_key(dlist_data(dd));
    if ((p = pluginlist_get(pl, pluginname)) == NULL)
      fatal(1, "BUG: %s loader plugin not found but in list.\n", pluginname);
    lp = plugin_get(p);

    stream_rewind(st);

    //debug_message("loader: identify: try %s\n", pluginname);

    if (lp->identify(ip, st, vw, c, lp->private) == LOAD_OK) {
      ip->format = pluginname;
      dlist_move_to_top(dl, dd);
      return 1;
    }
  }

  return 0;
}

static LoaderStatus
load_image(EnflePlugins *eps, char *pluginname, Image *ip, Stream *st, VideoWindow *vw, Config *c)
{
  Plugin *p;
  LoaderPlugin *lp;

  if ((p = pluginlist_get(eps->pls[ENFLE_PLUGIN_LOADER], pluginname)) == NULL)
    return 0;
  lp = plugin_get(p);

  ip->format_detail = NULL;
  stream_rewind(st);
  return lp->load(ip, st, vw, c, lp->private);
}
