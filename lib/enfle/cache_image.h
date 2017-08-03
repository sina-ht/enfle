/*
 * cache_image.h -- cache image header
 * (C)Copyright 2005 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat May 14 22:05:22 2005.
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

#if !defined(_CACHE_IMAGE_H)
#define _CACHE_IMAGE_H

#include "utils/cache.h"
#include "image.h"

int cache_add_image(Cache *c, Image *p, char *path);
Image *cache_get_image(Cache *c, char *path);

#endif
