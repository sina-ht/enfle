/*
 * stream-utils.c -- stream utility
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Sep 18 07:17:44 2000.
 * $Id: stream-utils.c,v 1.1 2000/09/30 17:36:36 sian Exp $
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
