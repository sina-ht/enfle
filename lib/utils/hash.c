/*
 * hash.c -- Hash Table Library
 * (C)Copyright 1999, 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Sep 18 07:14:12 2000.
 * $Id: hash.c,v 1.1 2000/09/30 17:36:36 sian Exp $
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
#include <string.h>

#include "common.h"

#include "hash.h"

static int define(Hash *, unsigned char *, void *);
static int set(Hash *, unsigned char *, void *);
static void set_function(Hash *, int (*)(unsigned char *, int, int));
static Dlist *get_keys(Hash *);
static int get_key_size(Hash *);
static void *lookup(Hash *, unsigned char *);
static int delete(Hash *, unsigned char *, int);
static void destroy(Hash *, int);

static int default_hash_function(unsigned char *, int, int);

static Hash hash_template = {
  hash_function: default_hash_function,
  define: define,
  set: set,
  set_function: set_function,
  get_keys: get_keys,
  get_key_size: get_key_size,
  lookup: lookup,
  delete: delete,
  destroy: destroy
};

static int
default_hash_function(unsigned char *k, int size, int n)
{
  int i, index;

  index = n * n;
  for (i = 0; i < strlen(k); i++) {
    index ^= k[i] << (i % 24);
  }
  index %= size;

  return index;
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

static int
lookup_internal(Hash *h, unsigned char *k)
{
  int n = 1, index;

  do {
    do {
      index = h->hash_function(k, h->size, n++);
    } while (h->data[index]->key == (Dlist_data *)-1);
  } while (h->data[index]->key && dlist_data(h->data[index]->key) &&
	   (strcmp(dlist_data(h->data[index]->key), k) != 0));

  return index;
}

/* methods */

static void
set_function(Hash *h, int (*function)(unsigned char *, int, int))
{
  h->hash_function = function;
}

static int
define(Hash *h, unsigned char *k, void *d)
{
  int index = lookup_internal(h, k);

  if (h->data[index]->key != NULL)
    return -1; /* already registered */

  if ((h->data[index]->key = h->keys->add_str(h->keys, k)) == NULL)
    return 0;

  h->data[index]->datum = d;

  return 1;
}

static int
set(Hash *h, unsigned char *k, void *d)
{
  int index = lookup_internal(h, k);

  if (h->data[index]->key == NULL)
    if ((h->data[index]->key = h->keys->add_str(h->keys, k)) == NULL)
      return 0;

  h->data[index]->datum = d;

  return 1;
}

static Dlist *
get_keys(Hash *h)
{
  return h->keys;
}

static int
get_key_size(Hash *h)
{
  Dlist *keys;

  keys = get_keys(h);
  return keys->get_datasize(keys);
}

static void *
lookup(Hash *h, unsigned char *k)
{
  int index = lookup_internal(h, k);

  return (h->data[index]->key == NULL) ? NULL : h->data[index]->datum;
}

static void
destroy_datum(Hash *h, int index, int f)
{
  Hash_data *d = h->data[index];

  if (d->key == (Dlist_data *)-1)
    return;
  if (d->key != NULL) {
    h->keys->delete(h->keys, d->key);
    d->key = (Dlist_data *)-1;
  }
  if (f && d->datum != NULL)
    free(d->datum);
}

static int
delete(Hash *h, unsigned char *k, int f)
{
  int index = lookup_internal(h, k);

  if (h->data[index]->key == NULL)
    return 0;

  destroy_datum(h, index, f);

  return 1;
}

static void
destroy(Hash *h, int f)
{
  Dlist_data *t;
  Dlist *keys;

  keys = get_keys(h);
  while (get_key_size(h) > 0) {
    t = keys->get_top(keys);
    delete(h, dlist_data(t), f);
  }

  keys->destroy(keys);
  free(h->data[0]);
  free(h->data);
  free(h);
}
