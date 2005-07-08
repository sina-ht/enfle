/*
 * cache.c -- cache object
 * (C)Copyright 2005 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue May 31 23:57:33 2005.
 * $Id: cache.c,v 1.1 2005/07/08 18:15:26 sian Exp $
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

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "cache.h"

Cache *
cache_create(int size, int hashsize)
{
  Cache *c;

  if (size > hashsize)
    return NULL;

  c = calloc(1, sizeof(*c));
  if (c == NULL)
    return NULL;

  c->dl = dlist_create();
  if (c->dl == NULL)
    goto out;

  c->hash = hash_create(hashsize);
  if (c->hash == NULL)
    goto out2;

  c->size = size;

  return c;
 out2:
  dlist_destroy(c->dl);
 out:
  free(c);
  return NULL;
}

void
cache_destroy(Cache *c)
{
  Dlist_data *dd;

  if (c) {
    /* Free all the cached objects */
    dlist_iter(c->dl, dd) {
      CachedObject *co = dlist_data(dd);
      debug_message_fnc("Removing cached object %*s\n", co->keylen, (char *)co->key);
      hash_delete(c->hash, co->key, co->keylen);
    }
    debug_message_fnc("Destroying hash\n");
    hash_destroy(c->hash);
    debug_message_fnc("Destroying dlist\n");
    dlist_destroy(c->dl);
    debug_message_fnc("Freeing Cache object\n");
    free(c);
  }
}

CachedObject *
cached_object_create(void *p, CachableObjectType cot, void *key, unsigned int keylen)
{
  CachedObject *co;

  if ((co = calloc(1, sizeof(*co))) == NULL)
    return NULL;
  co->p = p;
  co->cot = cot;
  co->key = malloc(keylen);
  memcpy(co->key, key, keylen);
  co->keylen = keylen;

  return co;
}

void
cached_object_destroy(CachedObject *co)
{
  if (co) {
    if (co->key)
      free(co->key);
    free(co);
  }
}

/*
 * CachedObject will be purged on the LRU(least recently used) basis.
 * When we add an object to the list, make it on the top.
 * When we use an object, make it on the top.
 * Thus, the object on the head is the least recently used object.
 */

static void
purge_cache(Cache *c)
{
  CachedObject *co;
  Dlist_data *dd;

  if ((dd = dlist_head(c->dl)) == NULL)
    fatal("dlist_head(c->dl) is NULL!\n");
  co = dlist_data(dd);

  debug_message_fnc("Purging cache, removing cached object %*s\n", co->keylen, (char *)co->key);

  hash_delete(c->hash, co->key, co->keylen);
  dlist_delete(c->dl, dd);
}

int
cache_add(Cache *c, CachedObject *co, CachedObjectDestructor cod)
{
  Dlist_data *dd;

  if (dlist_size(c->dl) >= c->size)
    purge_cache(c);

  if ((dd = dlist_add_object(c->dl, co, cod)) == NULL) {
    warning("dlist_add_object() failed\n");
    return 0;
  }
  if (!hash_define_value(c->hash, co->key, co->keylen, dd)) {
    warning("hash_define_value() failed\n");
    return 0;
  }
  dlist_move_to_top(c->dl, dd);

  debug_message_fnc("Cache: %d objects\n", dlist_size(c->dl));

  return 1;
}

CachedObject *
cache_get(Cache *c, void *key, unsigned int keylen)
{
  Dlist_data *dd;

  if ((dd = hash_lookup(c->hash, key, keylen)) == NULL)
    return NULL;
  dlist_move_to_top(c->dl, dd);

  return dlist_data(dd);
}
