/*
 * arithmodel_order_zero.h -- Order zero statistical model
 * (C)Copyright 2001 by Hiroshi Takekawa
 * Last Modified: Fri Sep  7 13:27:51 2001.
 * $Id: arithmodel_order_zero.h,v 1.8 2001/09/07 05:03:00 sian Exp $
 */

#ifndef _ARITHMODEL_ORDER_ZERO_H
#define _ARITHMODEL_ORDER_ZERO_H

typedef unsigned int _Freq;
typedef struct _arithmodel_order_zero Arithmodel_order_zero;

#define ARITHMODEL_ORDER_ZERO_METHOD_DECLARE \
 ARITHMODEL_METHOD_DECLARE; \
 static int my_reset(Arithmodel *, int, int); \
 static int encode_with_range(Arithmodel *, Index, Index, Index); \
 static int decode_with_range(Arithmodel *, Index *, Index, Index); \
 static void set_update_escape_freq(Arithmodel *, int (*)(Arithmodel *, Index)); \

struct _arithmodel_order_zero {
  ARITHMODEL_BASE_VARIABLES;
  int (*my_reset)(Arithmodel *, int, int);
  int (*encode_with_range)(Arithmodel *, Index, Index, Index);
  int (*decode_with_range)(Arithmodel *, Index *, Index, Index);
  void (*set_update_escape_freq)(Arithmodel *, int (*)(Arithmodel *, Index));
  int (*update_escape_freq)(Arithmodel *, Index);
  void *private_data;
  int is_eof_used;
  Index eof_symbol;
  int is_escape_used;
  Index escape_symbol;
  Index start_symbol;
  int escape_encoded_with_rle;
  int escape_run;
  Arithmodel *bin_am;
  _Freq *freq;
};

#define arithmodel_order_zero_nsymbols(am) (((Arithmodel_order_zero *)(am))->nsymbols - ((Arithmodel_order_zero *)(am))->start_symbol)

#define arithmodel_order_zero_uninstall_escape(am) (am)->uninstall_symbol((am), ((Arithmodel_order_zero *)(am))->escape_symbol)
#define arithmodel_order_zero_reset(am, eof, esc) ((Arithmodel_order_zero *)am)->my_reset((am), (eof), (esc))
#define arithmodel_order_zero_set_update_escape_freq(am, f) ((Arithmodel_order_zero *)am)->set_update_escape_freq((am), (f))
#define arithmodel_order_zero_encode_with_range(am, i, l, h) ((Arithmodel_order_zero *)am)->encode_with_range((am), (i), (l), (h))
#define arithmodel_order_zero_decode_with_range(am, ip, l, h) ((Arithmodel_order_zero *)am)->decode_with_range((am), (ip), (l), (h))

Arithmodel *arithmodel_order_zero_create(void);

#endif
