/*
 * libstring.c -- String manipulation library
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Aug 18 23:23:13 2002.
 * $Id: libstring.c,v 1.10 2002/08/18 14:26:55 sian Exp $
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
#include <stdarg.h>

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "libstring.h"

String *
string_create(void)
{
  String *s;

  if ((s = calloc(1, sizeof(String))) == NULL)
    return NULL;

  return s;
}

/* internal functions */

static void
buffer_free(String *s)
{
  if (string_buffer(s)) {
    free(string_buffer(s));
    string_buffer(s) = NULL;
    string_buffer_size(s) = 0;
  }
}

static int
buffer_alloc(String *s, unsigned int size)
{
  if (string_buffer_size(s) >= size)
    return 1;

  buffer_free(s);

  if ((string_buffer(s) = calloc(1, size)) == NULL)
    return 0;

  string_buffer_size(s) = size;

  return 1;
}

static int
str_alloc(String *s, unsigned int size)
{
  return buffer_alloc(s, size + 1);
}

static int
buffer_increase(String *s, unsigned int size)
{
  char *tmp;
  unsigned int newsize;

  if (string_buffer_size(s)) {
    newsize = string_buffer_size(s) + size;
    if ((tmp = realloc(string_buffer(s), newsize)) == NULL)
      return 0;
  } else {
    newsize = size + 1;
    if ((tmp = calloc(1, newsize)) == NULL)
      return 0;
  }

  string_buffer(s) = tmp;
  string_buffer_size(s) = newsize;

  return 1;
}

/* methods */

int
string_set(String *s, const char *p)
{
  unsigned int l = strlen(p);

  if (!str_alloc(s, l))
    return 0;
  strcpy(string_buffer(s), p);
  string_length(s) = l;

  return 1;
}

int
string_copy(String *s1, String *s2)
{
  if (!str_alloc(s1, string_length(s2)))
    return 0;
  strcpy(string_buffer(s1), string_buffer(s2));
  string_length(s1) = string_length(s2);

  return 1;
}

int
string_cat_ch(String *s, char c)
{
  if (!buffer_increase(s, 1))
    return 0;
  string_buffer(s)[string_length(s)] = c;
  string_buffer(s)[string_length(s) + 1] = '\0';
  string_length(s)++;

  return 1;
}

int
string_ncat(String *s, const char *p, unsigned int l)
{
  if (l > strlen(p))
    l = strlen(p);
  if (!buffer_increase(s, l))
    return 0;
  strncat(string_buffer(s), p, l);
  string_length(s) += l;

  return 1;
}

int
string_cat(String *s, const char *p)
{
  return string_ncat(s, p, strlen(p));
}

int
string_catf(String *s, const char *format, ...)
{
  va_list args;
  char *p, *tmp;
  int n, result;
  int size = 100;

  if ((p = malloc(size)) == NULL)
    return 0;

  while (1) {
    va_start(args, format);
    n = vsnprintf (p, size, format, args);
    va_end(args);

    if (n > -1 && n < size)
      break;

    size = (n > -1) ? n + 1 : (size << 1);
    if ((tmp = realloc(p, size)) == NULL) {
      free(p);
      return 0;
    }
    p = tmp;
  }

  result = string_cat(s, p);
  free(p);

  return result;
}

int
string_append(String *s1, String *s2)
{
  if (!buffer_increase(s1, string_length(s2)))
    return 0;
  strcat(string_buffer(s1), string_buffer(s2));
  string_length(s1) += string_length(s2);

  return 1;
}

void
string_shrink(String *s, unsigned int l)
{
  if (string_length(s) <= l)
    return;
  string_length(s) = l;
  string_buffer(s)[l] = '\0';
}

String *
string_dup(String *s)
{
  String *new;

  if ((new = string_create()) == NULL)
    return NULL;
  if (!string_set(new, string_buffer(s))) {
    string_destroy(new);
    return NULL;
  }

  return new;
}

void
string_destroy(String *s)
{
  buffer_free(s);
  free(s);
}
