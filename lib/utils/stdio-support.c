/*
 * stdio-support.c -- stdio support package
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Sep 21 02:26:39 2001.
 *
 * SPDX-License-Identifier: GPL-2.0-only
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
