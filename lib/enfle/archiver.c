/*
 * archiver.c -- archiver plugin interface
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Thu Sep 28 17:32:10 2000.
 * $Id: archiver.c,v 1.1 2000/09/30 17:36:36 sian Exp $
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
#include "archiver.h"
#include "archiver-plugin.h"

static char *load(Archiver *, Plugin *);
static int unload(Archiver *, char *);
static int identify(Archiver *, Archive *, Stream *);
static ArchiverStatus open(Archiver *, Archive *, char *,Stream *);
static void destroy(Archiver *);
static Dlist *get_names(Archiver *);
static unsigned char *get_description(Archiver *, char *);
static unsigned char *get_author(Archiver *, char *);

static Archiver archiver_template = {
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

Archiver *
archiver_create(void)
{
  Archiver *ar;

  if ((ar = (Archiver *)calloc(1, sizeof(Archiver))) == NULL)
    return NULL;
  memcpy(ar, &archiver_template, sizeof(Archiver));

  if ((ar->pl = pluginlist_create()) == NULL) {
    free(ar);
    return NULL;
  }

  return ar;
}

static char *
load(Archiver *ar, Plugin *p)
{
  PluginList *pl = ar->pl;
  ArchiverPlugin *arp;

  arp = plugin_get(p);

  if (!pluginlist_add(pl, p, arp->name)) {
    plugin_unload(p);
    return NULL;
  }

  return arp->name;
}

static int
unload(Archiver *ar, char *pluginname)
{
  Plugin *p;

  if ((p = pluginlist_get(ar->pl, pluginname)) == NULL)
    return 0;
  plugin_unload(p);

  return pluginlist_delete(ar->pl, pluginname);
}

static int
identify(Archiver *ar, Archive *a, Stream *st)
{
  Dlist *dl;
  Dlist_data *dd;

  dl = pluginlist_list(ar->pl);
  dlist_iter(dl, dd) {
    Plugin *p;
    ArchiverPlugin *arp;
    char *pluginname;

    pluginname = dlist_data(dd);
    if ((p = pluginlist_get(ar->pl, pluginname)) == NULL) {
      fprintf(stderr, "BUG: %s archiver plugin not found but in list.\n", pluginname);
      exit(-1);
    }
    arp = plugin_get(p);

    stream_rewind(st);
    if (arp->identify(a, st) == OPEN_OK) {
      a->format = pluginname;
      dlist_move_to_top(dl, dd);
      return 1;
    }
  }

  return 0;
}

static ArchiverStatus
open(Archiver *ar, Archive *a, char *pluginname, Stream *st)
{
  Plugin *p;
  ArchiverPlugin *arp;

  if ((p = pluginlist_get(ar->pl, pluginname)) == NULL)
    return 0;
  arp = plugin_get(p);

  stream_rewind(st);
  return arp->open(a, st);
}

static void
destroy(Archiver *ar)
{
  pluginlist_destroy(ar->pl);
  free(ar);
}

static Dlist *
get_names(Archiver *ar)
{
  return pluginlist_get_names(ar->pl);
}

static unsigned char *
get_description(Archiver *ar, char *pluginname)
{
  Plugin *p;
  ArchiverPlugin *arp;

  if ((p = pluginlist_get(ar->pl, pluginname)) == NULL)
    return NULL;
  arp = plugin_get(p);

  return arp->description;
}

static unsigned char *
get_author(Archiver *ar, char *pluginname)
{
  Plugin *p;
  ArchiverPlugin *arp;

  if ((p = pluginlist_get(ar->pl, pluginname)) == NULL)
    return NULL;
  arp = plugin_get(p);

  return arp->author;
}
