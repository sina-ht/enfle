/*
 * stream-utils.c -- stream utility
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Aug 14 16:34:48 2001.
 * $Id: stream-utils.c,v 1.5 2001/08/15 06:37:23 sian Exp $
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

#include "stream-utils.h"
#include "utils.h"

int
stream_read_little_uint32(Stream *st, unsigned int *val)
{
  unsigned char buf[4];

  if (stream_read(st, buf, 4) != 4)
    return 0;

  *val = utils_get_little_uint32(buf);

  return 1;
}

int
stream_read_big_uint32(Stream *st, unsigned int *val)
{
  unsigned char buf[4];

  if (stream_read(st, buf, 4) != 4)
    return 0;

  *val = utils_get_big_uint32(buf);

  return 1;
}

int
stream_read_little_uint16(Stream *st, unsigned short int *val)
{
  unsigned char buf[2];

  if (stream_read(st, buf, 2) != 2)
    return 0;

  *val = utils_get_little_uint16(buf);

  return 1;
}

int
stream_read_big_uint16(Stream *st, unsigned short int *val)
{
  unsigned char buf[2];

  if (stream_read(st, buf, 2) != 2)
    return 0;

  *val = utils_get_big_uint16(buf);

  return 1;
}

#define ROOM_PER_ALLOC 80

char *
stream_gets(Stream *st)
{
  char *p = NULL, *tmp;
  int size = 0, ptr = 0, read_size;

  for (;;) {
    /* Be sure the buffer has an additional byte for '\0'. */
    if (ptr >= size - 1) {
      if ((tmp = realloc(p, size + ROOM_PER_ALLOC)) == NULL) {
	if (p)
	  free(p);
	return NULL;
      }
      p = tmp;
      size += ROOM_PER_ALLOC;
    }

    if ((read_size = stream_read(st, p + ptr, 1)) < 0) {
      free(p);
      return NULL;
    } else if (read_size == 0) {
      break;
    } else if (p[ptr] == '\n')
      break;
    ptr++;
  }

  p[ptr] = '\0';

  if ((tmp = realloc(p, strlen(p) + 1)) == NULL) {
    free(p);
    return NULL;
  }

  return tmp;
}
