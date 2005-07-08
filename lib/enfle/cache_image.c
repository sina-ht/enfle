/*
 * cache_image.c -- cache image
 * (C)Copyright 2005 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Thu Jun 30 23:56:51 2005.
 * $Id: cache_image.c,v 1.1 2005/07/08 18:15:26 sian Exp $
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

  debug_message_fnc("Caching image %s\n", path);

  if ((co = cached_object_create(p, _CACHED_IMAGE, path, strlen(path))) == NULL)
    return 0;
  image_clean(p);
  return cache_add(c, co, cached_image_destructor);
}

Image *
cache_get_image(Cache *c, char *path)
{
  CachedObject *co;

  debug_message_fnc("Getting cached image for %s\n", path);

  if ((co = cache_get(c, path, strlen(path))) == NULL)
    return NULL;
  debug_message_fnc("Got it\n");
  return co->p;
}
