/*
 * streamer.c -- streamer plugin interface
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Oct 10 04:56:28 2000.
 * $Id: streamer.c,v 1.2 2000/10/09 20:29:56 sian Exp $
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

#define REQUIRE_STRING_H
#include "compat.h"

#include "common.h"

#include "streamer.h"
#include "streamer-plugin.h"

static char *load(Streamer *, Plugin *);
static int unload(Streamer *, char *);
static int identify(Streamer *, Stream *, char *);
static int open(Streamer *, Stream *, char *, char *);
static void destroy(Streamer *);
static Dlist *get_names(Streamer *);
static unsigned char *get_description(Streamer *, char *);
static unsigned char *get_author(Streamer *, char *);

static Streamer streamer_template = {
  pl: NULL,
  load: load,
  unload: unload,
  identify: identify,
  open: open,
  destroy: destroy,
  get_names: get_names,
  get_description: get_description,
  get_author: get_author
};

Streamer *
streamer_create(void)
{
  Streamer *st;

  if ((st = (Streamer *)calloc(1, sizeof(Streamer))) == NULL)
    return NULL;
  memcpy(st, &streamer_template, sizeof(Streamer));

  if ((st->pl = pluginlist_create()) == NULL) {
    free(st);
    return NULL;
  }

  return st;
}

/* methods */

static char *
load(Streamer *st, Plugin *p)
{
  PluginList *pl = st->pl;
  StreamerPlugin *stp;

  stp = plugin_get(p);

  if (!pluginlist_add(pl, p, stp->name)) {
    plugin_unload(p);
    return NULL;
  }

  return stp->name;
}

static int
unload(Streamer *st, char *pluginname)
{
  Plugin *p;

  if ((p = pluginlist_get(st->pl, pluginname)) == NULL)
    return 0;
  plugin_unload(p);

  return pluginlist_delete(st->pl, pluginname);
}

static int
identify(Streamer *st, Stream *s, char *filepath)
{
  Dlist *dl;
  Dlist_data *dd;

  dl = pluginlist_list(st->pl);
  dlist_iter(dl, dd) {
    Plugin *p;
    StreamerPlugin *stp;
    char *pluginname;

    pluginname = dlist_data(dd);
    if ((p = pluginlist_get(st->pl, pluginname)) == NULL) {
      fprintf(stderr, "BUG: %s streamer plugin not found but in list.\n", pluginname);
      exit(-1);
    }
    stp = plugin_get(p);

    if (stp->identify(s, filepath) == STREAM_OK) {
      s->format = pluginname;
      dlist_move_to_top(dl, dd);
      return 1;
    }
  }

  return 0;
}

static int
open(Streamer *st, Stream *s, char *pluginname, char *filepath)
{
  Plugin *p;
  StreamerPlugin *stp;

  if ((p = pluginlist_get(st->pl, pluginname)) == NULL)
    return 0;
  stp = plugin_get(p);

  s->path = strdup(filepath);
  return stp->open(s, filepath);
}

static void
destroy(Streamer *st)
{
  pluginlist_destroy(st->pl);
  free(st);
}

static Dlist *
get_names(Streamer *st)
{
  return pluginlist_get_names(st->pl);
}

static unsigned char *
get_description(Streamer *st, char *pluginname)
{
  Plugin *p;
  StreamerPlugin *stp;

  if ((p = pluginlist_get(st->pl, pluginname)) == NULL)
    return NULL;
  stp = plugin_get(p);

  return stp->description;
}

static unsigned char *
get_author(Streamer *st, char *pluginname)
{
  Plugin *p;
  StreamerPlugin *stp;

  if ((p = pluginlist_get(st->pl, pluginname)) == NULL)
    return NULL;
  stp = plugin_get(p);

  return stp->author;
}
