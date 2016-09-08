/*
 * rgbparse.c -- RGB database parser
 * (C)Copyright 1999, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Aug  6 16:45:04 2006.
 * $Id: rgbparse.c,v 1.6 2006/08/06 07:50:56 sian Exp $
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
#include <ctype.h>

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "rgbparse.h"

/* Must be prime */
#define COLOR_MAX 2017

static char *
parse_number(char *str, int *number_return)
{
  char *p, *s;
  char t;

  s = str;
  while (isspace(*s))
    s++;
  p = s;
  while (isdigit(*p))
    p++;
  if (!isspace(*p)) {
    *number_return = -1;
    return p;
  }

  t = *p;
  *p = '\0';
  *number_return = atoi(s);
  *p++ = t;

  return p;
}

static char *
parse_string(char *p)
{
  char *s;

  while (isspace(*p))
    p++;

  s = strdup(p);
  if (s == NULL)
    return NULL;

  return s;
}

static void
parse_error(char *s)
{
  err_message("rgbparse: %s\n", s);
}

static int
rgbparse_line(Hash *h, char *s)
{
  char *k;
  unsigned int i;
  NColor *c;

  if ((c = malloc(sizeof(NColor))) == NULL)
    return 0;

  for (i = 0; i < 3; i++) {
    if ((s = parse_number(s, &c->c[i])) == NULL) {
      free(c);
      return 0;
    }
    if (c->c[i] == -1) {
      parse_error(s);
      free(c);
      return 0;
    }
  }

  if ((k = parse_string(s)) == NULL) {
    parse_error(s);
    free(c);
    return 0;
  }

  /* color names are case insensitive */
  for (i = 0; i < strlen(k); i++)
    if (isupper(k[i]))
      k[i] = tolower(k[i]);

  hash_define_str(h, k, c);
  free(k);

  return 1;
}

static void
chomp(char *s)
{
  int l = strlen(s);

  if (l == 0)
    return;
  s += l - 1;
  if (*s == '\n')
    if (l > 1 && *(s - 1) == '\r')
      *(s - 1) = '\0';
    else
      *s = '\0';
  else if (*s == '\r')
    *s = '\0';
}

Hash *
rgbparse(char *path)
{
  FILE *fp;
  Hash *h;
  char s[256];

  if ((fp = fopen(path, "r")) == NULL) {
    debug_message_fnc("Cannot open %s\n", path);
    return NULL;
  }

  if ((h = hash_create(COLOR_MAX)) == NULL) {
    fclose(fp);
    return NULL;
  }

  while (fgets(s, 256, fp) != NULL) {
    if (s[0] == '!')
      continue;
    chomp(s);
    if (!rgbparse_line(h, s)) {
      hash_destroy(h);
      fclose(fp);
      return NULL;
    }
  }

  fclose(fp);

  return h;
}
