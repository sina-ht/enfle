/*
 * hash.h -- Hash Table Library Header
 * (C)Copyright 1999, 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Wed Sep 20 00:21:10 2000.
 * $Id: hash.h,v 1.1 2000/09/30 17:36:36 sian Exp $
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

#ifndef _HASH_H
#define _HASH_H

#include "dlist.h"

typedef struct {
  Dlist_data *key;
  void *datum;
} Hash_data;

typedef struct _hash Hash;
struct _hash {
  int size;
  Hash_data **data;
  Dlist *keys;
  int (*hash_function)(unsigned char *, int, int);
  int (*define)(Hash *, unsigned char *, void *);
  int (*set)(Hash *, unsigned char *, void *);
  void (*set_function)(Hash *, int (*)(unsigned char *, int, int));
  Dlist *(*get_keys)(Hash *);
  int (*get_key_size)(Hash *);
  void *(*lookup)(Hash *, unsigned char *);
  int (*delete)(Hash *, unsigned char *, int);
  void (*destroy)(Hash *, int);
};

#define hash_define(h, k, d) (h)->define((h), (k), (d))
#define hash_set(h, k, d) (h)->set((h), (k), (d))
#define hash_set_function(h, func) (h)->set_function((h), (func))
#define hash_get_keys(h) (h)->get_keys((h))
#define hash_get_key_size(h) (h)->get_key_size((h))
#define hash_lookup(h, k) (h)->lookup((h), (k))
#define hash_delete(h, k, f) (h)->delete((h), (k), (f))
#define hash_destroy(h, f) (h)->destroy((h), (f))

#define hash_iter(h, dl, dd, k, d) \
 dl = hash_get_keys(h); \
 for (dd = dlist_get_top((dl)), k = dlist_data((dd)), d = hash_lookup((h), (k)); \
      dd != NULL && (k = dlist_data((dd))) && (d = hash_lookup((h), (k))); \
      dd = dlist_next((dd)))

Hash *hash_create(int);

#endif
