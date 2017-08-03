/*
 * arithcoder.h -- Arithmetic coder base class
 * (C)Copyright 2001 by Hiroshi Takekawa
 * Last Modified: Mon Sep 17 23:04:56 2001.
 */

#ifndef _ARITHCODER_H
#define _ARITHCODER_H

typedef unsigned int Arithvalue;
typedef unsigned int Index;

typedef struct _arithcoder Arithcoder;

#define ARITHCODER_BASE_VARIABLES \
  Arithvalue low; \
  Arithvalue range; \
  Arithvalue value; \
  int ndigits_followed; \
  int (*encode_init)(Arithcoder *, void *); \
  int (*decode_init)(Arithcoder *, void *); \
  void (*encode_renormalize)(Arithcoder *); \
  void (*decode_renormalize)(Arithcoder *); \
  int (*encode_final)(Arithcoder *); \
  int (*decode_final)(Arithcoder *); \
  void (*destroy)(Arithcoder *)

struct _arithcoder {
  ARITHCODER_BASE_VARIABLES;
};

#define arithcoder_encode_init(ac, d) (ac)->encode_init((ac), (d))
#define arithcoder_decode_init(ac, d) (ac)->decode_init((ac), (d))
#define arithcoder_encode_renormalize(ac) (ac)->encode_renormalize((ac))
#define arithcoder_decode_renormalize(ac) (ac)->decode_renormalize((ac))
#define arithcoder_encode_final(ac) (ac)->encode_final((ac))
#define arithcoder_decode_final(ac) (ac)->decode_final((ac))
#define arithcoder_destroy(ac) (ac)->destroy((ac))

#endif
