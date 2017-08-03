/*
 * arithmodel_utils.c -- Utility functions
 * (C)Copyright 2001 by Hiroshi Takekawa
 * Last Modified: Wed Dec 26 09:47:17 2001.
 */

#include <stdio.h>

#include "arithcoder.h"
#include "arithmodel.h"
#include "arithmodel_utils.h"

int
arithmodel_encode_bits(Arithmodel *m, int b, int n, Index bit0, Index bit1)
{
  int i;
  int mask = 1 << (n - 1);

  for (i = 0; i < n; i++, mask >>= 1)
    if (!arithmodel_encode(m, (b & mask) ? bit1 : bit0))
      return i;

  return n;
}

int
arithmodel_decode_bits(Arithmodel *m, int *b_return, int n, Index bit0, Index bit1)
{
  int i, b = 0;
  Index bit;

  for (i = 0; i < n; i++) {
    if (!arithmodel_decode(m, &bit))
      return 0;
    if (bit == bit0)
      b <<= 1;
    else if (bit == bit1)
      b = (b << 1) | 1;
    else
      return 0;
  }

  *b_return = b;

  return 1;
}

static int
calc_num_of_figures(unsigned int n)
{
  int nbits = 0;

  while ((n >> 8) > 0) {
    n >>= 8;
    nbits += 8;
  }
  while (n > 0) {
    n >>= 1;
    nbits++;
  }

  return nbits;
}

int
arithmodel_encode_gamma(Arithmodel *m, unsigned int n, Index bit0, Index bit1)
{
  int i;
  int nbits = calc_num_of_figures(n);

  for (i = 0; i < nbits - 1; i++)
    arithmodel_encode(m, bit0);
  arithmodel_encode_bits(m, n, nbits, bit0, bit1);

  return nbits * 2 - 1;
}

int
arithmodel_encode_delta(Arithmodel *m, unsigned int n, Index bit0, Index bit1)
{
  int t;
  int nbits = calc_num_of_figures(n);

  t = arithmodel_encode_gamma(m, nbits, bit0, bit1);
  if (nbits > 1)
    arithmodel_encode_bits(m, n, nbits - 1, bit0, bit1);

  return t + nbits - 1;
}

int
arithmodel_decode_gamma(Arithmodel *am, unsigned int *n_return, Index bit0, Index bit1)
{
  unsigned int n = 1;
  int nbits = 0;
  int b;

  for (;;) {
    if (!arithmodel_decode_bit(am, &b, bit0, bit1))
      return 0;
    if (!b)
      nbits++;
    else
      break;
  }

  if (nbits)
    for (; nbits; nbits--) {
      if (!arithmodel_decode_bit(am, &b, bit0, bit1))
	return 0;
      n = (n << 1) | b;
    }

  *n_return = n;

  return 1;
}

int
arithmodel_decode_delta(Arithmodel *am, unsigned int *n_return, Index bit0, Index bit1)
{
  unsigned nbits;
  int n = 1;
  int b;

  if (!arithmodel_decode_gamma(am, &nbits, bit0, bit1))
    return 0;
  while (--nbits > 0) {
    if (!arithmodel_decode_bit(am, &b, bit0, bit1))
      return 0;
    n = (n << 1) | b;
  }

  *n_return = n;

  return 1;
}

int
arithmodel_encode_cbt(Arithmodel *am, unsigned int n, unsigned int nmax, Index bit0, Index bit1)
{
  int k = calc_num_of_figures(nmax + 1);
  unsigned int kn = (1 << k) - (nmax + 1);

  if (n > nmax) {
    fprintf(stderr, "%s: n = %d > %d = nmax\n", __FUNCTION__, n, nmax);
    return -1;
  }

  if (n < kn) {
    arithmodel_encode_bits(am, n, k - 1, bit0, bit1);
    return k - 1;
  }

  arithmodel_encode_bits(am, n + kn, k, bit0, bit1);
  return k;
}

int
arithmodel_decode_cbt(Arithmodel *am, unsigned int *n_return, unsigned int nmax, Index bit0, Index bit1)
{
  int k = calc_num_of_figures(nmax + 1);
  unsigned int kn = (1 << k) - (nmax + 1);
  unsigned int t;
  int b;

  if (!arithmodel_decode_bits(am, &t, k - 1, bit0, bit1))
    return 0;

  if (t < kn) {
    *n_return = t;
    return 1;
  }

  if (!arithmodel_decode_bit(am, &b, bit0, bit1))
    return 0;

  *n_return = ((t << 1) | b) - kn;
  return 1;
}
