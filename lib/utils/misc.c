/*
 * misc.c -- miscellaneous routines
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Oct 13 01:09:10 2001.
 * $Id: misc.c,v 1.6 2001/10/14 12:32:54 sian Exp $
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
#include <ctype.h>

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
misc_get_ext(const char *path, int if_lower)
{
  char *p, *ret, *r;

  if ((p = strrchr(path, '.')) == NULL)
    return NULL;
  p++;

  ret = r = malloc(strlen(p) + 1);
  if (!if_lower)
    return strcpy(r, p);
  while (*p) {
    *r++ = (char)tolower((int)*p);
    p++;
  }
  *r = '\0';

  return ret;
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

/* Make a directory pathname canonical */
char *
misc_canonical_pathname(char *pathname)
{
  int l = strlen(pathname);
  char *p;

  if (pathname[l - 1] != '/') {
    /* Pathname should be ended with '/'. */
    if ((p = malloc(l + 2)) == NULL)
      return NULL;
    strcpy(p, pathname);
    strcat(p, "/");
  } else if (pathname[l - 2] == '/') {
    /* Pathname should be ended with single '/'. */
    l--;
    while (pathname[l - 2] == '/') {
      pathname[l - 1] = '\0';
      l--;
    }
    if ((p = malloc(l + 1)) == NULL)
      return NULL;
    strncpy(p, pathname, l);
  } else {
    if ((p = strdup(pathname)) == NULL)
      return NULL;
  }

  return p;
}
