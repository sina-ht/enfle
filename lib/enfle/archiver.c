/*
 * archiver.c -- archiver plugin interface
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Oct 17 22:16:53 2000.
 * $Id: archiver.c,v 1.2 2000/10/17 14:04:01 sian Exp $
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

static int identify(EnflePlugins *, Archive *, Stream *);
static ArchiverStatus open(EnflePlugins *, Archive *, char *,Stream *);

static Archiver template = {
  identify: identify,
  open: open
};

Archiver *
archiver_create(void)
{
  Archiver *ar;

  if ((ar = (Archiver *)calloc(1, sizeof(Archiver))) == NULL)
    return NULL;
  memcpy(ar, &template, sizeof(Archiver));

  return ar;
}

/* methods */

static int
identify(EnflePlugins *eps, Archive *a, Stream *st)
{
  Dlist *dl;
  Dlist_data *dd;
  PluginList *pl;

  pl = eps->pls[ENFLE_PLUGIN_ARCHIVER];
  dl = pluginlist_list(pl);
  dlist_iter(dl, dd) {
    Plugin *p;
    ArchiverPlugin *arp;
    char *pluginname;

    pluginname = dlist_data(dd);
    if ((p = pluginlist_get(pl, pluginname)) == NULL) {
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
open(EnflePlugins *eps, Archive *a, char *pluginname, Stream *st)
{
  Plugin *p;
  ArchiverPlugin *arp;

  if ((p = pluginlist_get(eps->pls[ENFLE_PLUGIN_ARCHIVER], pluginname)) == NULL)
    return 0;
  arp = plugin_get(p);

  stream_rewind(st);
  return arp->open(a, st);
}
