/*
 * loader.c -- loader plugin interface
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Nov 18 15:12:32 2000.
 * $Id: loader.c,v 1.5 2000/11/20 12:55:18 sian Exp $
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

#include "common.h"

#include "loader.h"
#include "loader-plugin.h"

static int identify(EnflePlugins *, Image *, Stream *);
static LoaderStatus load_image(EnflePlugins *, char *, Image *, Stream *);

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
identify(EnflePlugins *eps, Image *ip, Stream *st)
{
  Dlist *dl;
  Dlist_data *dd;
  PluginList *pl;

  pl = eps->pls[ENFLE_PLUGIN_LOADER];
  dl = pluginlist_list(pl);
  dlist_iter(dl, dd) {
    Plugin *p;
    LoaderPlugin *lp;
    char *pluginname;

    pluginname = dlist_data(dd);
    if ((p = pluginlist_get(pl, pluginname)) == NULL) {
      fprintf(stderr, "BUG: %s loader plugin not found but in list.\n", pluginname);
      exit(-1);
    }
    lp = plugin_get(p);

    stream_rewind(st);

    debug_message("loader: identify: try %s\n", pluginname);

    if (lp->identify(ip, st, lp->private) == LOAD_OK) {
      ip->format = pluginname;
      dlist_move_to_top(dl, dd);
      return 1;
    }
  }

  return 0;
}

static LoaderStatus
load_image(EnflePlugins *eps, char *pluginname, Image *ip, Stream *st)
{
  Plugin *p;
  LoaderPlugin *lp;

  if ((p = pluginlist_get(eps->pls[ENFLE_PLUGIN_LOADER], pluginname)) == NULL)
    return 0;
  lp = plugin_get(p);

  if (ip->image) {
    free(ip->image);
    ip->image = NULL;
  }

  stream_rewind(st);
  return lp->load(ip, st, lp->private);
}
