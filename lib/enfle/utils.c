/*
 * utils.c -- utility functions
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Apr 13 21:13:11 2001.
 * $Id: utils.c,v 1.4 2001/04/18 05:38:20 sian Exp $
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

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"
#include "utils.h"

/* replace or add extension like 'png' */
char *
replace_ext(char *filename, char *ext)
{
  char *ptr = strrchr(filename, '.');
  char *new;
  int base_len;

  if (ptr == NULL) {
    base_len = strlen(filename);
  } else {
    base_len = (int)(ptr - filename);
  }
  if ((new = malloc(base_len + 1 + strlen(ext) + 1)) == NULL)
    return NULL;
  if (ptr != filename)
    memcpy(new, filename, base_len);
  new[base_len] = '.';
  strcpy(new + base_len + 1, ext);

  return new;
}
