/*
 * hash.c -- Hash Table Library
 * (C)Copyright 1999, 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Aug 19 21:34:30 2002.
 * $Id: hash.c,v 1.16 2002/08/19 12:51:23 sian Exp $
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
#include <signal.h>

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "hash.h"

#define HASH_LOOKUP_SEARCH_MATCH 0
#define HASH_LOOKUP_ACCEPT_DELETED 1
#define HASH_DESTROYED_KEY ((Dlist_data *)-1)

static unsigned int default_hash_function(void *, unsigned int);
static unsigned int default_hash_function2(void *, unsigned int);

static Hash hash_template = {
  hash_function: default_hash_function,
  hash_function2: default_hash_function2,
};

static unsigned int
default_hash_function(void *key, unsigned int len)
{
  unsigned char c, *k;
  unsigned int h = 0, l;

  k = key;
  l = len;
  while (len--) {
    c = *k++;
    h += c + (c << 17);
    h ^= (h >> 2);
  }
  h += l + (l << 17);
  h ^= (h >> 2);

  return h;
}

static unsigned int
default_hash_function2(void *key, unsigned int len)
{
  unsigned char c, *k;
  unsigned int h = 0, l;

  k = key;
  l = len;
  while (len--) {
    c = *k++;
    h += c + (c << 13);
    h ^= (h >> 3);
  }
  h += l + (l << 13);
  h ^= (h >> 3);

  return 17 - (h % 17);
}

Hash *
hash_create(int size)
{
  int i;
  Hash *h;
  Hash_data *d;

  if ((h = (Hash *)calloc(1, sizeof(Hash))) == NULL)
    return NULL;
  memcpy(h, &hash_template, sizeof(Hash));

  if ((h->data = (Hash_data **)calloc(size, sizeof(Hash_data *))) == NULL)
    goto error_h;

  if ((d = (Hash_data *)calloc(size, sizeof(Hash_data))) == NULL)
    goto error_data;

  for (i = 0; i < size; i++)
    h->data[i] = d++;

  if ((h->keys = dlist_create()) == NULL)
    goto error_d;

  h->size = size;

  return h;

 error_d:    free(d);
 error_data: free(h->data);
 error_h:    free(h);
  return NULL;
}

static unsigned int
lookup_internal(Hash *h, void *k, unsigned int len, int flag)
{
  int count = 0;
  unsigned int hash = h->hash_function(k, len) % h->size;
  unsigned int skip = h->hash_function2(k, len);
  Hash_key *hk;

  do {
    if (h->data[hash]->key == HASH_DESTROYED_KEY) {
      if (flag == HASH_LOOKUP_ACCEPT_DELETED)
	break;
      hash = (hash + skip) % h->size;
    } else if (!h->data[hash]->key) {
      /* found empty */
      break;
    } else if ((hk = dlist_data(h->data[hash]->key)) &&
	       hk->len == len && (memcmp(hk->key, k, len) == 0)) {
      break;
    } else {
      hash = (hash + skip) % h->size;
    }
    count++;

    bug_on(count > 100000);

  } while (1);

  return hash;
}

static Hash_key *
hash_key_create(void *k, unsigned int len)
{
  Hash_key *hk;

  if ((hk = malloc(sizeof(Hash_key))) == NULL)
    return NULL;
  if ((hk->key = malloc(len)) == NULL) {
    free(hk);
    return NULL;
  }
  memcpy(hk->key, k, len);
  hk->len = len;

  return hk;
}

static void
hash_key_destroy(void *arg)
{
  Hash_key *hk = arg;

  if (hk) {
    if (hk->key)
      free(hk->key);
    free(hk);
  }
}

/* methods */

void
hash_set_function(Hash *h, unsigned int (*function)(void *, unsigned int))
{
  h->hash_function = function;
}

void
hash_set_function2(Hash *h, unsigned int (*function2)(void *, unsigned int))
{
  h->hash_function2 = function2;
}

int
hash_define_object(Hash *h, void *k, unsigned int len, void *d, Hash_data_destructor dest)
{
  unsigned int i = lookup_internal(h, k, len, HASH_LOOKUP_ACCEPT_DELETED);
  Hash_key *hk;

  if (h->data[i]->key != NULL && h->data[i]->key != HASH_DESTROYED_KEY)
    return -1; /* already registered */

  if ((hk = hash_key_create(k, len)) == NULL)
    return 0;
  if ((h->data[i]->key = dlist_add_object(h->keys, hk, hash_key_destroy)) == NULL) {
    hash_key_destroy(hk);
    return 0;
  }

  h->data[i]->datum = d;
  h->data[i]->data_destructor = dest;

  return 1;
}

int
hash_define(Hash *h, void *k, unsigned int len, void *d)
{
  return hash_define_object(h, k, len, d, free);
}

int
hash_define_value(Hash *h, void *k, unsigned int len, void *d)
{
  return hash_define_object(h, k, len, d, NULL);
}

static void
destroy_hash_data(Hash_data *d)
{
  if (d->datum && d->data_destructor)
    d->data_destructor(d->datum);
}

int
hash_set_object(Hash *h, void *k, unsigned int len, void *d, Hash_data_destructor dest)
{
  unsigned int i = lookup_internal(h, k, len, HASH_LOOKUP_ACCEPT_DELETED);
  Hash_key *hk;

  if (h->data[i]->key == NULL || h->data[i]->key == HASH_DESTROYED_KEY) {
    if ((hk = hash_key_create(k, len)) == NULL)
      return 0;
    if ((h->data[i]->key = dlist_add_object(h->keys, hk, hash_key_destroy)) == NULL) {
      hash_key_destroy(hk);
      return 0;
    }
  } else {
    /* Should destroy overwritten data */
    destroy_hash_data(h->data[i]);
  }

  h->data[i]->datum = d;
  h->data[i]->data_destructor = dest;

  return 1;
}

int
hash_set(Hash *h, void *k, unsigned int len, void *d)
{
  return hash_set_object(h, k, len, d, free);
}

int
hash_set_value(Hash *h, void *k, unsigned int len, void *d)
{
  return hash_set_object(h, k, len, d, NULL);
}

int
hash_get_key_size(Hash *h)
{
  Dlist *keys = hash_get_keys(h);
  return dlist_size(keys);
}

void *
hash_lookup(Hash *h, void *k, unsigned int len)
{
  unsigned int i = lookup_internal(h, k, len, HASH_LOOKUP_SEARCH_MATCH);
  return (h->data[i]->key == NULL) ? NULL : h->data[i]->datum;
}

static int
destroy_datum(Hash *h, int i)
{
  Hash_data *d = h->data[i];

  if (d->key == HASH_DESTROYED_KEY)
    return 0;
  if (d->key != NULL) {
    if (!dlist_delete(h->keys, d->key))
      return 0;
    d->key = HASH_DESTROYED_KEY;
  }
  destroy_hash_data(d);

  return 1;
}

int
hash_delete(Hash *h, void *k, unsigned int len)
{
  int i = lookup_internal(h, k, len, HASH_LOOKUP_SEARCH_MATCH);

  if (h->data[i]->key == NULL)
    return 0;

  if (!destroy_datum(h, i))
    return 0;

  return 1;
}

void
hash_destroy(Hash *h)
{
  Dlist_data *t;
  Dlist *keys = hash_get_keys(h);
  Hash_key *hk;

  while (hash_get_key_size(h) > 0) {
    t = dlist_top(keys);
    hk = dlist_data(t);
    hash_delete(h, hk->key, hk->len);
  }

  dlist_destroy(keys);
  free(h->data[0]);
  free(h->data);
  free(h);
}
