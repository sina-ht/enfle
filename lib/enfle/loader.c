/*
 * loader.c -- loader plugin interface
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Wed Sep 20 11:09:09 2000.
 * $Id: loader.c,v 1.1 2000/09/30 17:36:36 sian Exp $
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

#include "stream.h"
#include "loader.h"

static char *load(Loader *, Plugin *);
static int unload(Loader *, char *);
static int identify(Loader *, Image *, Stream *);
static LoaderStatus load_image(Loader *, char *, Image *, Stream *);
static void destroy(Loader *);
static Dlist *get_names(Loader *);
static unsigned char *get_description(Loader *, char *);
static unsigned char *get_author(Loader *, char *);

static Loader loader_template = {
  pl: NULL,
  load: load,
  unload: unload,
  identify: identify,
  load_image: load_image,
  destroy: destroy,
  get_names: get_names,
  get_description: get_description,
  get_author: get_author
};

Loader *
loader_create(void)
{
  Loader *l;

  if ((l = (Loader *)calloc(1, sizeof(Loader))) == NULL)
    return NULL;
  memcpy(l, &loader_template, sizeof(Loader));

  if ((l->pl = pluginlist_create()) == NULL) {
    free(l);
    return NULL;
  }

  return l;
}

/* methods */

static char *
load(Loader *l, Plugin *p)
{
  PluginList *pl = l->pl;
  LoaderPlugin *lp;

  lp = plugin_get(p);

  if (!pluginlist_add(pl, p, lp->name)) {
    plugin_unload(p);
    return NULL;
  }

  return lp->name;
}

static int
unload(Loader *l, char *pluginname)
{
  Plugin *p;

  if ((p = pluginlist_get(l->pl, pluginname)) == NULL)
    return 0;
  plugin_unload(p);

  return pluginlist_delete(l->pl, pluginname);
}

static int
identify(Loader *l, Image *ip, Stream *st)
{
  Dlist *dl;
  Dlist_data *dd;

  dl = pluginlist_list(l->pl);
  dlist_iter(dl, dd) {
    Plugin *p;
    LoaderPlugin *lp;
    char *pluginname;

    pluginname = dlist_data(dd);
    if ((p = pluginlist_get(l->pl, pluginname)) == NULL) {
      fprintf(stderr, "BUG: %s loader plugin not found but in list.\n", pluginname);
      exit(-1);
    }
    lp = plugin_get(p);

    stream_rewind(st);
    if (lp->identify(ip, st) == LOAD_OK) {
      ip->format = pluginname;
      dlist_move_to_top(dl, dd);
      return 1;
    }
  }

  return 0;
}

static LoaderStatus
load_image(Loader *l, char *pluginname, Image *ip, Stream *st)
{
  Plugin *p;
  LoaderPlugin *lp;

  if ((p = pluginlist_get(l->pl, pluginname)) == NULL)
    return 0;
  lp = plugin_get(p);

  stream_rewind(st);
  return lp->load(ip, st);
}

static void
destroy(Loader *l)
{
  pluginlist_destroy(l->pl);
  free(l);
}

static Dlist *
get_names(Loader *l)
{
  return pluginlist_get_names(l->pl);
}

static unsigned char *
get_description(Loader *l, char *pluginname)
{
  Plugin *p;
  LoaderPlugin *lp;

  if ((p = pluginlist_get(l->pl, pluginname)) == NULL)
    return NULL;
  lp = plugin_get(p);

  return lp->description;
}

static unsigned char *
get_author(Loader *l, char *pluginname)
{
  Plugin *p;
  LoaderPlugin *lp;

  if ((p = pluginlist_get(l->pl, pluginname)) == NULL)
    return NULL;
  lp = plugin_get(p);

  return lp->author;
}
