/*
 * misc.c -- miscellaneous routines
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Oct 31 00:37:14 2000.
 * $Id: misc.c,v 1.2 2000/10/30 16:17:44 sian Exp $
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
misc_trim_ext(char *path, char *ext)
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
