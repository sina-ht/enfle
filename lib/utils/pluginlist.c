/*
 * pluginlist.c -- Plugin List library
 * (C)Copyright 2000, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Aug 18 23:39:38 2002.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include <stdlib.h>

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "pluginlist.h"

PluginList *
pluginlist_create(void)
{
  PluginList *pl;

  if ((pl = (PluginList *)calloc(1, sizeof(PluginList))) == NULL)
    return NULL;

  if ((pl->hash = hash_create(PLUGINLIST_HASH_SIZE)) == NULL) {
    free(pl);
    return NULL;
  }

  return pl;
}

int
pluginlist_add(PluginList *pl, Plugin *p, const char *name)
{
  return hash_set_str_value(pl->hash, name, p);
}

Plugin *
pluginlist_get(PluginList *pl, char *name)
{
  return hash_lookup_str(pl->hash, name);
}

int
pluginlist_delete(PluginList *pl, char *name)
{
  return hash_delete_str(pl->hash, name);
}

Dlist *
pluginlist_get_names(PluginList *pl)
{
  if (!pl)
    return NULL;
  return hash_get_keys(pl->hash);
}

void
pluginlist_destroy(PluginList *pl)
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
