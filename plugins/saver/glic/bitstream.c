/*
 * bitstream.c -- bitwise I/O stream
 * (C)Copyright 2000 by Hiroshi Takekawa
 * Last Modified: Tue Jun 19 02:05:00 2001.
 * $Id: bitstream.c,v 1.2 2001/06/19 08:16:19 sian Exp $
 */

#include <stdlib.h>

#define REQUIRE_STRING_H
#include "compat.h"

#include "bitstream.h"

static void init(BitStream *, FILE *);
static int getbit(BitStream *);
static unsigned int getbits(BitStream *, unsigned int, int *);
static int putbit(BitStream *, int);
static int putbits(BitStream *, unsigned int, int);
static int flush(BitStream *);
static void destroy(BitStream *);

static BitStream template = {
  fp: NULL,
  buffer: 0,
  buffer_left: 0,
  init: init,
  getbit: getbit,
  getbits: getbits,
  putbit: putbit,
  putbits: putbits,
  flush: flush,
  destroy: destroy
};

BitStream *
bitstream_create(void)
{
  BitStream *bs;

  if ((bs = (BitStream *)calloc(1, sizeof(BitStream))) == NULL)
    return NULL;
  memcpy(bs, &template, sizeof(BitStream));

  return bs;
}

/* for internal use */

static inline int
fill_buffer(BitStream *bs)
{
  if ((bs->buffer_left = fread(&bs->buffer, 1, BUFFER_SIZE, bs->fp)) < BUFFER_SIZE) {
    if (feof(bs->fp)) {
      if (bs->buffer_left == 0) {
	return EOF;
      }
    } else {
      return EOF - 1;
    }
  }
  bs->buffer_left <<= 3;

  return bs->buffer_left;
}

static inline int
flush_buffer(BitStream *bs)
{
  int to_write, written;

  to_write = (bs->buffer_left + 7) >> 3;
  if ((written = fwrite(&bs->buffer, 1, to_write, bs->fp)) != to_write)
    return 0;

  bs->buffer = 0;
  bs->buffer_left = 0;

  return 1;
}

static inline unsigned int
extract_bits(BitStream *bs, int n)
{
  return ((bs->buffer >> (BUFFER_BITS - n)) & ((1 << n) - 1));
}

/* methods */

static void
init(BitStream *bs, FILE *fp)
{
  bs->fp = fp;
  bs->buffer = 0;
  bs->buffer_left = 0;
}

static int
getbit(BitStream *bs)
{
  int ret;

  if (bs->buffer_left == 0) {
    int read;

    if ((read = fill_buffer(bs)) == EOF)
      return EOF;
    else if (read == EOF - 1)
      return EOF - 1;
  }

  ret = (bs->buffer & BUFFER_MSB) ? 1 : 0;
  bs->buffer <<= 1;
  bs->buffer_left--;

  return ret;
}

static unsigned int
getbits(BitStream *bs, unsigned int n, int *err_return)
{
  unsigned int ret = 0;

  *err_return = 0;
  while (n > 0) {
    if (bs->buffer_left == 0) {
      int read;

      if ((read = fill_buffer(bs)) == EOF) {
	*err_return = EOF;
	return ret;
      } else if (read == EOF - 1) {
	*err_return = EOF - 1;
	return ret;
      }
    }

    if (bs->buffer_left >= n) {
      ret = (ret << n) | extract_bits(bs, n);
      bs->buffer <<= n;
      bs->buffer_left -= n;
      return ret;
    } else {
      ret = (ret << bs->buffer_left) | extract_bits(bs, bs->buffer_left);
      n -= bs->buffer_left;
      bs->buffer_left = 0;
    }
  }

  return ret;
}

static int
putbit(BitStream *bs, int b)
{
  bs->buffer = (bs->buffer << 1) | (b & 1);
  bs->buffer_left++;
  if (bs->buffer_left == BUFFER_BITS)
    if (!flush_buffer(bs))
      return 0;

  return 1;
}

static int
putbits(BitStream *bs, unsigned int bits, int n)
{
  int written = n;

  while (n > 0) {
    if (bs->buffer_left + n <= BUFFER_BITS) {
      bs->buffer = (bs->buffer << n)  | (bits & ((1 << n) - 1));
      bs->buffer_left += n;
      n = 0;
      if (bs->buffer_left == BUFFER_BITS)
	if (!flush_buffer(bs))
	  return 0;
    } else {
      bs->buffer = (bs->buffer << (BUFFER_BITS - bs->buffer_left)) |
	((bits >> (n - (BUFFER_BITS - bs->buffer_left))) &
	 ((1 << (BUFFER_BITS - bs->buffer_left)) - 1));
      n -= (BUFFER_BITS - bs->buffer_left);
      bits &= (1 << n) - 1;
      if (!flush_buffer(bs))
	return 0;
    }
  }

  return written;
}

static int
flush(BitStream *bs)
{
  return flush_buffer(bs);
}

static void
destroy(BitStream *bs)
{
  free(bs);
}
