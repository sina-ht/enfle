/*
 * arithcoder.h -- Arithmetic coder base class
 * (C)Copyright 2001 by Hiroshi Takekawa
 * Last Modified: Thu Mar 29 13:13:46 2001.
 * $Id: arithcoder.h,v 1.1 2001/04/18 05:43:31 sian Exp $
 */

#ifndef _ARITHCODER_H
#define _ARITHCODER_H

typedef unsigned int Arithvalue;
typedef unsigned short int Index;

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
