/*
 * stdio-support.c -- stdio support package
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Sep 19 23:25:21 2000.
 * $Id: stdio-support.c,v 1.1 2000/09/30 17:36:36 sian Exp $
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
#include <stdlib.h>
#include <string.h>

#include "common.h"

#include "stdio-support.h"

char *
stdios_gets(FILE *fp)
{
  char *p, *tmp;
  int size = 80;

  p = NULL;
  for (;;) {
    if ((tmp = realloc(p, size)) == NULL) {
      if (p)
	free(p);
      return NULL;
    }
    p = tmp;

    if (fgets(p, size, fp) == NULL) {
      free(p);
      return NULL;
    }

    if (strchr(p, '\n') != NULL)
      break;

    size += 80;
  }

  p = realloc(p, strlen(p) + 1);

  return p;
}
