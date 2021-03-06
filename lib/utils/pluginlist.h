/*
 * pluginlist.h -- Plugin List library header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Jul  3 13:42:07 2005.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _PLUGINLIST_H
#define _PLUGINLIST_H

#include "hash.h"
#include "plugin.h"

typedef struct _pluginlist PluginList;
struct _pluginlist {
  Hash *hash;
};

/* Must be prime */
#define PLUGINLIST_HASH_SIZE 1031

PluginList *pluginlist_create(void);
int pluginlist_add(PluginList *, Plugin *, const char *);
Plugin *pluginlist_get(PluginList *, char *);
int pluginlist_delete(PluginList *, char *);
Dlist *pluginlist_get_names(PluginList *);
void pluginlist_destroy(PluginList *);

#define pluginlist_iter(pl, k, kl, p) hash_iter((pl)->hash, k, kl, p)
#define pluginlist_move_to_top dlist_move_to_top(hash_iter_dl, hash_iter_dd)
#define pluginlist_iter_end hash_iter_end

#endif
