/*
 * cache_image.c -- cache image
 * (C)Copyright 2005 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Dec 30 14:46:42 2006.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "cache_image.h"

static void
cached_image_destructor(void *p)
{
  CachedObject *co = p;
  debug_message_fnc("Destroy cached and purged image %*s.\n", co->keylen, (char *)co->key);
  image_destroy(co->p);
  cached_object_destroy(co);
}

int
cache_add_image(Cache *c, Image *p, char *path)
{
  CachedObject *co;

  if (image_image(p) == NULL)
    return 0;
  if ((co = cached_object_create(p, _CACHED_IMAGE, path, strlen(path))) == NULL)
    return 0;
  image_clean(p);
  co->memsize = memory_size(image_image(p));
  debug_message_fnc("Caching image %s, %d bytes\n", path, co->memsize);

  return cache_add(c, co, cached_image_destructor);
}

Image *
cache_get_image(Cache *c, char *path)
{
  CachedObject *co;

  //debug_message_fnc("Getting cached image for %s\n", path);

  if ((co = cache_get(c, path, strlen(path))) == NULL)
    return NULL;
  //debug_message_fnc("Got it\n");
  return co->p;
}
