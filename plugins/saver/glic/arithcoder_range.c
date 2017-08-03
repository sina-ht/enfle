/*
 * arithcoder_range.c -- Renormalization for range coder
 * (C)Copyright 2001 by Hiroshi Takekawa
 * Last Modified: Wed Mar 28 09:51:08 2001.
 *
 * WARNING:
 *  This renormalization code does not infringe any patent, hopefully.
 *  I will not take any responsibility.
 *
 *  YOU HAVE BEEN WARNED.
 */

#include <stdio.h>
#include <stdlib.h>

#include "arithcoder.h"
#include "arithcoder_range.h"

typedef struct _arithcoder_range {
  ARITHCODER_BASE_VARIABLES;
  FILE *fp;
  unsigned char buffer;
} Arithcoder_range;

#define VALUE_BITS (sizeof(Arithvalue) << 3)
#define TOP_VALUE ((unsigned)(1 << (VALUE_BITS - 1)))
#define BOTTOM_VALUE (TOP_VALUE >> 8)
#define CARRY_CHECK_VALUE (0xff << (VALUE_BITS - 1 - 8))

static int encode_init(Arithcoder *, void *);
static int decode_init(Arithcoder *, void *);
static void encode_renormalize(Arithcoder *);
static void decode_renormalize(Arithcoder *);
static int encode_final(Arithcoder *);
static int decode_final(Arithcoder *);
static void destroy(Arithcoder *);

Arithcoder *
arithcoder_range_create(void)
{
  Arithcoder_range *ac;

  if ((ac = calloc(1, sizeof(Arithcoder_range))) == NULL)
    return NULL;
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
  Arithcoder_range *ac = (Arithcoder_range *)_ac;
  FILE *fp = (void *)d;

  ac->low = 0;
  ac->range = TOP_VALUE;
  ac->ndigits_followed = 0;
  ac->fp = fp;
  ac->buffer = '\0';

  return 1;
}

static int
decode_init(Arithcoder *_ac, void *d)
{
  Arithcoder_range *ac = (Arithcoder_range *)_ac;
  FILE *fp = (void *)d;

  ac->low = 0;
  ac->range = 1;
  ac->ndigits_followed = 0;
  ac->fp = fp;
  ac->buffer = '\0';

  return 1;
}

static inline void
output(Arithcoder_range *ac, int c, unsigned char d)
{
  fputc(ac->buffer + c, ac->fp);
  while (ac->ndigits_followed-- > 0)
    fputc(d, ac->fp);
}

static void
encode_renormalize(Arithcoder *_ac)
{
  Arithcoder_range *ac = (Arithcoder_range *)_ac;

  while (ac->range <= BOTTOM_VALUE) {
    if (ac->low < CARRY_CHECK_VALUE) {
      output(ac, 0, 0xff);
      ac->buffer = (unsigned char)(ac->low >> (VALUE_BITS - 1 - 8));
    } else if (ac->low >= TOP_VALUE) {
      output(ac, 1, 0);
      ac->buffer = (unsigned char)(ac->low >> (VALUE_BITS - 1 - 8));
    } else {
      ac->ndigits_followed++;
    }
    ac->low = (ac->low << 8) & (TOP_VALUE - 1);
    ac->range <<= 8;
  }
}

static void
decode_renormalize(Arithcoder *ac)
{
}

static int
encode_final(Arithcoder *_ac)
{
  return 1;
}

static int
decode_final(Arithcoder *_ac)
{
  return 1;
}

static void
destroy(Arithcoder *_ac)
{
  free(_ac);
}
