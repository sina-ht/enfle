/*
 * arithcoder_arith.c -- Renormalization for arithmetic coder
 * (C)Copyright 2001 by Hiroshi Takekawa
 * Last Modified: Thu Apr 12 14:31:06 2001.
 * $Id: arithcoder_arith.c,v 1.1 2001/04/18 05:43:31 sian Exp $
 *
 * WARNING:
 *  This renormalization code probably implements the exact patented arithmetic coder.
 *  Unless applicable law permits to use this code safely,
 *  you are prohibited to use this, even if for research.
 *
 *  YOU HAVE BEEN WARNED.
 */

#include <stdlib.h>

#ifdef HAVE_CONFIG_H
# include "enfle-config.h"
#endif
#define CONFIG_H_INCLUDED
#undef DEBUG

#include "common.h"

#include "arithcoder.h"
#include "arithcoder_arith.h"
#include "bitstream.h"

#define VALUE_BITS (sizeof(Arithvalue) << 3)
#define HALF_VALUE (1U << (VALUE_BITS - 1))
#define QUARTER_VALUE (HALF_VALUE >> 1)

typedef struct _arithcoder_arith {
  ARITHCODER_BASE_VARIABLES;
  BitStream *bs;
} Arithcoder_arith;

static int encode_init(Arithcoder *, void *);
static int decode_init(Arithcoder *, void *);
static void encode_renormalize(Arithcoder *);
static void decode_renormalize(Arithcoder *);
static int encode_final(Arithcoder *);
static int decode_final(Arithcoder *);
static void destroy(Arithcoder *);

Arithcoder *
arithcoder_arith_create(void)
{
  Arithcoder_arith *ac;

  if ((ac = calloc(1, sizeof(Arithcoder_arith))) == NULL)
    return NULL;
  if ((ac->bs = bitstream_create()) == NULL) {
    free(ac);
    return NULL;
  }
  ac->encode_init = encode_init;
  ac->decode_init = decode_init;
  ac->encode_renormalize = encode_renormalize;
  ac->decode_renormalize = decode_renormalize;
  ac->encode_final = encode_final;
  ac->decode_final = decode_final;
  ac->destroy = destroy;

  return (Arithcoder *)ac;
}

static int
encode_init(Arithcoder *_ac, void *d)
{
  Arithcoder_arith *ac = (Arithcoder_arith *)_ac;
  FILE *fp = (FILE *)d;

  ac->low = 0;
  ac->range = HALF_VALUE;
  ac->ndigits_followed = 0;
  bitstream_init(ac->bs, fp);

  return 1;
}

static int
decode_init(Arithcoder *_ac, void *d)
{
  Arithcoder_arith *ac = (Arithcoder_arith *)_ac;
  FILE *fp = (FILE *)d;

  ac->low = 0;
  bitstream_init(ac->bs, fp);
#if 0
  ac->value = 0;
  ac->range = 1;
#else
  {
    int err;

    ac->range = HALF_VALUE;
    ac->value = bitstream_getbits(ac->bs, VALUE_BITS, &err);
  }
#endif

  return 1;
}

static inline void
output(Arithcoder_arith *ac, int b)
{
  bitstream_putbit(ac->bs, b);
  if (ac->ndigits_followed < 8) {
    if (b)
      bitstream_putbits(ac->bs, 0, ac->ndigits_followed);
    else
      bitstream_putbits(ac->bs, (1U << ac->ndigits_followed) - 1, ac->ndigits_followed);
    ac->ndigits_followed = 0;
  } else {
    b ^= 1;
    while (ac->ndigits_followed > 0) {
      bitstream_putbit(ac->bs, b);
      ac->ndigits_followed--;
    }
  }
}

static void
encode_renormalize(Arithcoder *_ac)
{
  Arithcoder_arith *ac = (Arithcoder_arith *)_ac;

  debug_message(__FUNCTION__ "(low %X range %X)\n", ac->low, ac->range);

  while (ac->range <= QUARTER_VALUE) {
    if (ac->low >= HALF_VALUE) {
      output(ac, 1);
      ac->low -= HALF_VALUE;
    } else if (ac->low + ac->range <= HALF_VALUE) {
      output(ac, 0);
    } else {
      ac->ndigits_followed++;
      ac->low -= QUARTER_VALUE;
    }
    ac->low <<= 1;
    ac->range <<= 1;
  }

  debug_message(__FUNCTION__ " done (low %X range %X)\n", ac->low, ac->range);
}

static void
decode_renormalize(Arithcoder *_ac)
{
  Arithcoder_arith *ac = (Arithcoder_arith *)_ac;
  int b;

  debug_message(__FUNCTION__ "(low %X range %X value %X)\n", ac->low, ac->range, ac->value);

  while (ac->range <= QUARTER_VALUE) {
    if (ac->low >= HALF_VALUE) {
      ac->low -= HALF_VALUE;
      ac->value -= HALF_VALUE;
    } else if (ac->low + ac->range <= HALF_VALUE) {
      /* Do nothing */
    } else {
      ac->low -= QUARTER_VALUE;
      ac->value -= QUARTER_VALUE;
    }
    ac->low <<= 1;
    ac->range <<= 1;
    if ((b = bitstream_getbit(ac->bs)) < 0) {
      debug_message(__FUNCTION__ ": b == %d\n", b);
      b = 1;
    }
    ac->value = (ac->value << 1) | b;
  }

  debug_message(__FUNCTION__ " done (low %X range %X value %X)\n", ac->low, ac->range, ac->value);
}

static int
encode_final(Arithcoder *_ac)
{
  Arithcoder_arith *ac = (Arithcoder_arith *)_ac;

  for (;;) {
    if (ac->low + (ac->range >> 1) >= HALF_VALUE) {
      output(ac, 1);
      if (ac->low < HALF_VALUE) {
	ac->range -= HALF_VALUE - ac->low;
	ac->low = 0;
      } else {
	ac->low -= HALF_VALUE;
      }
    } else {
      output(ac, 0);
      if (ac->low + ac->range > HALF_VALUE)
	ac->range = HALF_VALUE - ac->low;
    }
    if (ac->range == HALF_VALUE)
      break;
    ac->low <<= 1;
    ac->range <<= 1;
  }

  return bitstream_flush(ac->bs);
}

static int
decode_final(Arithcoder *_ac)
{
  return 1;
}

static void
destroy(Arithcoder *_ac)
{
  Arithcoder_arith *ac = (Arithcoder_arith *)_ac;

  if (ac->bs)
    bitstream_destroy(ac->bs);
  free(ac);
}
