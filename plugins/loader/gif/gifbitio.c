/*
 * gifbitio.c -- bit based I/O package for GIF library
 * (C)Copyright 1998, 99, 2002 by Hiroshi Takekawa
 *
 * This is for GIF decode library, so output routines are not implemented.
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
