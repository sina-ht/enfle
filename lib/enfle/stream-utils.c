/*
 * stream-utils.c -- stream utility
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat May  4 23:21:30 2002.
 * $Id: stream-utils.c,v 1.8 2002/05/04 14:36:00 sian Exp $
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

#include "stream-utils.h"
#define UTILS_NEED_GET_LITTLE_UINT32
#define UTILS_NEED_GET_LITTLE_UINT16
#define UTILS_NEED_GET_BIG_UINT32
#define UTILS_NEED_GET_BIG_UINT16
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

    if ((read_size = stream_read(st, (unsigned char *)p + ptr, 1)) < 0) {
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

char *
stream_ngets(Stream *st, char *buf, int size)
{
  char *p = buf;
  int len = 0;

  /* XXX: hmm... */
  while (len < size - 1) {
    if (stream_read(st, (unsigned char *)p, 1) != 1) {
      if (len)
	break;
      return NULL;
    }
    if (*p++ == '\n')
      break;
    len++;
  }
  *p = '\0';

  return buf;
}

int
stream_getc(Stream *st)
{
  unsigned char c;

  if (stream_read(st, &c, 1) != 1)
    return -1;
  return (int)c;
}
