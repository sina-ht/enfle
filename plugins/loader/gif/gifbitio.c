/*
 * gifbitio.c -- bit based I/O package for GIF library
 * (C)Copyright 1998, 99, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Jun 21 21:17:32 2004.
 * $Id: gifbitio.c,v 1.2 2004/06/21 12:22:23 sian Exp $
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

#include "compat.h"
#include "common.h"

#include "giflib.h"
#include "gifbitio.h"
#include "enfle/stream-utils.h"

/* global variable */
static int left , buf, blocksize;
static Stream *st;

void
gif_bitio_init(Stream *s)
{
  left = 0;
  blocksize = 0;
  st = s;
}

int
getbit(void)
{
  int d;

  if (left == 0) {
    if ((blocksize == 0) && ((blocksize = stream_getc(st)) == 0))
      return -1;
    if ((buf = stream_getc(st)) < 0)
      return -1;
    blocksize--;
    left = 8;
  }
  d = buf & 1;
  buf >>= 1;
  left--;

  return d;
}

int
getbits(int bits)
{
  int d = 0, done = 0;
  static int mask[9] = {0, 1, 3, 7, 15, 31, 63, 127, 255};

  while (bits > 0) {
    if (left == 0) {
      if ((blocksize == 0) && ((blocksize = stream_getc(st)) == 0)) {
	stream_seek(st, -1, SEEK_CUR); /* ungetc(0, fp); */
	return -1;
      }
      if ((buf = stream_getc(st)) < 0)
	return -1;
      blocksize--;
      left = 8;
    }
    if (left >= bits) {
      d |= (buf & mask[bits]) << done;
      buf >>= bits;
      left -= bits;
      bits = 0;
    } else {
      d |= (buf & mask[left]) << done;
      bits -= left;
      done += left;
      left = 0;
    }
  }

  return d;
}
