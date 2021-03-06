/*
 * cache.h -- cache object header
 * (C)Copyright 2005 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Thu Sep  7 22:47:17 2006.
 *
 * SPDX-License-Identifier: GPL-2.0-only
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
