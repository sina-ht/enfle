/*
 * stream-utils.c -- stream utility
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Feb 20 20:10:47 2001.
 * $Id: stream-utils.c,v 1.2 2001/02/20 13:55:41 sian Exp $
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
