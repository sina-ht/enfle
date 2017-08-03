/*
 * stdio-support.c -- stdio support package
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Sep 21 02:26:39 2001.
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

#include <stdlib.h>

#define REQUIRE_STRING_H
#include "compat.h"

#include "common.h"

#include "stdio-support.h"
#include "libstring.h"

#define TMP_SIZE 80

char *
stdios_gets(FILE *fp)
{
  String *s;
  char t[TMP_SIZE], *p;

  if ((s = string_create()) == NULL)
    return NULL;

  for (;;) {
    if (fgets(t, TMP_SIZE, fp) == NULL) {
      string_destroy(s);
      return NULL;
    }

    string_cat(s, t);
    if (strchr(t, '\n') != NULL)
      break;
  }

  p = strdup(string_get(s));
  string_destroy(s);

  return p;
}
