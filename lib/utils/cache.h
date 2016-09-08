/*
 * cache.h -- cache object header
 * (C)Copyright 2005 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Thu Sep  7 22:47:17 2006.
 * $Id: cache.h,v 1.3 2006/09/09 12:55:55 sian Exp $
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#if !defined(_CACHE_H)
#define _CACHE_H

#include "dlist.h"
#include "hash.h"

typedef enum {
  _CACHED_IMAGE
} CachableObjectType;

typedef struct _cached_object {
  void *key;
  unsigned int keylen;
  CachableObjectType cot;
  int memsize;
  void *p;
} CachedObject;

typedef struct _cache {
  int size;
  int memsize;
  int memsize_max;
  Dlist *dl;
  Hash *hash;
} Cache;

typedef void (*CachedObjectDestructor)(void *obj);

Cache *cache_create(int size, int memsize_max);
void cache_destroy(Cache *c);
CachedObject *cached_object_create(void *p, CachableObjectType cot, void *key, unsigned int keylen);
void cached_object_destroy(CachedObject *co);
int cache_add(Cache *c, CachedObject *co, CachedObjectDestructor cod);
CachedObject *cache_get(Cache *c, void *key, unsigned int keylen);

#endif
