/*
 * getopt-support.c -- getopt support
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Jul 17 03:50:16 2005.
 * $Id: getopt-support.c,v 1.2 2005/07/16 18:52:28 sian Exp $
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

#include <stdio.h>

#define REQUIRE_STRING_H
#include "compat.h"

#include "utils/libstring.h"
#include "getopt-support.h"

char *
gen_optstring(Option opt[], struct option **long_opt_r)
{
  int i;
  String *s;
  char *optstr;
  struct option *long_opt;

  s = string_create();
  i = 0;
  while (opt[i].longopt != NULL) {
    string_cat_ch(s, opt[i].opt);
    switch (opt[i].argreq) {
    case _NO_ARGUMENT:
      break;
    case _REQUIRED_ARGUMENT:
      string_cat_ch(s, ':');
      break;
    case _OPTIONAL_ARGUMENT:
      string_cat(s, "::");
      break;
    }
    i++;
  }
  optstr = strdup(string_get(s));
  string_destroy(s);

#if !defined(HAVE_GETOPT_LONG)
  *long_opt_r = NULL;
#else
  if ((long_opt = *long_opt_r = calloc(i + 1, sizeof(struct option))) == NULL)
    return optstr;

  i = 0;
  while (opt[i].longopt != NULL) {
    long_opt[i].name = opt[i].longopt;
    switch (opt[i].argreq) {
    case _NO_ARGUMENT:
      long_opt[i].has_arg = 0;
      break;
    case _REQUIRED_ARGUMENT:
      long_opt[i].has_arg = 1;
      break;
    case _OPTIONAL_ARGUMENT:
      long_opt[i].has_arg = 2;
      break;
    }
    long_opt[i].flag = NULL;
    long_opt[i].val = opt[i].opt;
    i++;
  }
  long_opt[i].name = NULL;
#endif
  return optstr;
}

void
print_option_usage(Option opt[])
{
  int i;

#if defined(HAVE_GETOPT_LONG)
  printf("Long option supported.\n");
#else
  printf("Long option not supported.\n");
#endif
  i = 0;
  while (opt[i].longopt != NULL) {
    printf(" %c(%s): \t%s\n",
	   opt[i].opt, opt[i].longopt, opt[i].description);
    i++;
  }
}
