/*
 * pluginlist.c -- Plugin List library
 * (C)Copyright 2000, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Thu Aug 15 22:44:22 2002.
 * $Id: pluginlist.c,v 1.8 2002/08/17 02:19:36 sian Exp $
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

#include <stdlib.h>

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "pluginlist.h"

static int add(PluginList *, Plugin *, const char *);
static Plugin *get(PluginList *, char *);
static int delete(PluginList *, char *);
static Dlist *get_names(PluginList *);
static void destroy(PluginList *);

static PluginList pluginlist_template = {
  add: add,
  get: get,
  delete: delete,
  get_names: get_names,
  destroy: destroy
};

PluginList *
pluginlist_create(void)
{
  PluginList *pl;

  if ((pl = (PluginList *)calloc(1, sizeof(PluginList))) == NULL)
    return NULL;
  memcpy(pl, &pluginlist_template, sizeof(PluginList));

  if ((pl->hash = hash_create(PLUGINLIST_HASH_SIZE)) == NULL) {
    free(pl);
    return NULL;
  }

  return pl;
}

static int
add(PluginList *pl, Plugin *p, const char *name)
{
  return hash_set_str_value(pl->hash, name, p);
}

static Plugin *
get(PluginList *pl, char *name)
{
  return hash_lookup_str(pl->hash, name);
}

static int
delete(PluginList *pl, char *name)
{
  return hash_delete_str(pl->hash, name);
}

static Dlist *
get_names(PluginList *pl)
{
  if (!pl)
    return NULL;
  return hash_get_keys(pl->hash);
}

static void
destroy(PluginList *pl)
{
  Plugin *p;
  void *k;
  unsigned int kl;

  pluginlist_iter(pl, k, kl, p)
    plugin_destroy(p);
  pluginlist_iter_end;
  hash_destroy(pl->hash);

  free(pl);
}
