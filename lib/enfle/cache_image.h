/*
 * cache_image.h -- cache image header
 * (C)Copyright 2005 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat May 14 22:05:22 2005.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#if !defined(_CACHE_IMAGE_H)
#define _CACHE_IMAGE_H

#include "utils/cache.h"
#include "image.h"

int cache_add_image(Cache *c, Image *p, char *path);
Image *cache_get_image(Cache *c, char *path);

#endif
