/*
 * cache_image.c -- cache image
 * (C)Copyright 2005 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Dec 30 14:46:42 2006.
 * $Id: cache_image.c,v 1.4 2007/04/27 05:55:27 sian Exp $
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
