/*
 * arithmodel.h -- Statistical model layer
 * (C)Copyright 2001 by Hiroshi Takekawa
 * Last Modified: Tue Aug  7 16:55:50 2001.
 * $Id: arithmodel.h,v 1.2 2001/08/07 09:28:20 sian Exp $
 */

#ifndef _ARITHMODEL_H
#define _ARITHMODEL_H

typedef struct _arithmodel Arithmodel;

#define ARITHMODEL_BASE_VARIABLES \
  Arithcoder *ac; \
  Index nsymbols; \
  int (*install_symbol)(Arithmodel *, unsigned int); \
  int (*uninstall_symbol)(Arithmodel *, Index); \
  int (*reset)(Arithmodel *); \
  int (*encode_init)(Arithmodel *, Arithcoder *); \
  int (*encode)(Arithmodel *, Index); \
  int (*encode_final)(Arithmodel *); \
  int (*decode_init)(Arithmodel *, Arithcoder *); \
  int (*decode)(Arithmodel *, Index *); \
  int (*decode_final)(Arithmodel *); \
  void (*destroy)(Arithmodel *)

#define ARITHMODEL_METHOD_DECLARE \
 static int install_symbol(Arithmodel *, unsigned int); \
 static int uninstall_symbol(Arithmodel *, Index); \
 static int reset(Arithmodel *); \
 static int encode_init(Arithmodel *, Arithcoder *); \
 static int encode(Arithmodel *, Index); \
 static int encode_final(Arithmodel *); \
 static int decode_init(Arithmodel *, Arithcoder *); \
 static int decode(Arithmodel *, Index *); \
 static int decode_final(Arithmodel *); \
 static void destroy(Arithmodel *)

struct _arithmodel {
  ARITHMODEL_BASE_VARIABLES;
};

#define arithmodel_install_symbol(am, f) (am)->install_symbol((am), (f))
#define arithmodel_uninstall_symbol(am, i) (am)->uninstall_symbol((am), (i))
#define arithmodel_reset(am) (am)->reset((am))
#define arithmodel_encode_init(am, ac) (am)->encode_init((am), (ac))
#define arithmodel_encode(am, i) (am)->encode((am), (i))
#define arithmodel_encode_final(am) (am)->encode_final((am))
#define arithmodel_decode_init(am, ac) (am)->decode_init((am), (ac))
#define arithmodel_decode(am, ip) (am)->decode((am), (ip))
#define arithmodel_decode_final(am) (am)->decode_final((am))
#define arithmodel_destroy(am) (am)->destroy((am))

#endif
