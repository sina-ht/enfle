/*
 * pluginlist.h -- Plugin List library header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Thu Aug  8 00:11:37 2002.
 * $Id: pluginlist.h,v 1.4 2002/08/07 15:33:45 sian Exp $
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

#ifndef _PLUGINLIST_H
#define _PLUGINLIST_H

#include "hash.h"
#include "plugin.h"

typedef struct _pluginlist PluginList;
struct _pluginlist {
  Hash *hash;

  int (*add)(PluginList *, Plugin *, const char *);
  Plugin *(*get)(PluginList *, char *);
  int (*delete)(PluginList *, char *);
  Dlist *(*get_names)(PluginList *);
  void (*destroy)(PluginList *);
};

#define PLUGINLIST_HASH_SIZE 1024

#define pluginlist_hash(pl) (pl)->hash
#define pluginlist_add(pl, p, n) (pl)->add((pl), (p), (n))
#define pluginlist_get(pl, n) (pl)->get((pl), (n))
#define pluginlist_delete(pl, n) (pl)->delete((pl), (n))
#define pluginlist_get_names(pl) (pl)->get_names((pl))
#define pluginlist_destroy(pl) (pl)->destroy((pl))

#define pluginlist_iter(pl, k, kl, p) hash_iter((pl)->hash, k, kl, p)
#define pluginlist_move_to_top dlist_move_to_top(hash_iter_dl, hash_iter_dd)
#define pluginlist_iter_end hash_iter_end

PluginList *pluginlist_create(void);

#endif
