/*
 * misc.c -- miscellaneous routines
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Thu Apr 26 17:45:30 2001.
 * $Id: misc.c,v 1.4 2001/04/27 01:00:10 sian Exp $
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

#include "misc.h"

char *
misc_basename(char *path)
{
  char *ptr = path + strlen(path);

  while (path < ptr && *--ptr != '/') ;
  if (*ptr == '/')
    ptr++;

  return ptr;
}

char *
misc_trim_ext(const char *path, const char *ext)
{
  char *p;
  int len;

  if ((p = strrchr(path, '.')) == NULL)
    return strdup(path);
  if (ext && strcasecmp(p + 1, ext))
    return strdup(path);
  len = p - path;
  if ((p = (char *)malloc(len + 1)) == NULL)
    return NULL;
  memcpy(p, path, len);
  p[len] = '\0';

  return p;
}

/* replace or add extension like 'png' */
char *
misc_replace_ext(char *filename, char *ext)
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
