/*
 * arithmodel_order_zero.h -- Order zero statistical model
 * (C)Copyright 2001 by Hiroshi Takekawa
 * Last Modified: Tue Aug  7 21:44:15 2001.
 * $Id: arithmodel_order_zero.h,v 1.6 2001/08/09 17:28:48 sian Exp $
 */

#ifndef _ARITHMODEL_ORDER_ZERO_H
#define _ARITHMODEL_ORDER_ZERO_H

typedef unsigned int _Freq;
typedef struct _arithmodel_order_zero Arithmodel_order_zero;

struct _arithmodel_order_zero {
  ARITHMODEL_BASE_VARIABLES;
  int (*my_reset)(Arithmodel *, int, int);
  void (*set_update_escape_freq)(Arithmodel *, int (*)(Arithmodel *, Index));
  int (*update_escape_freq)(Arithmodel *, Index);
  void (*set_update_region)(Arithmodel *, void (*)(Arithmodel *, Index));
  void (*update_region)(Arithmodel *, Index);
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

#define arithmodel_order_zero_uninstall_escape(am) (am)->uninstall_symbol((am), ((Arithmodel_order_zero *)(am))->escape_symbol)
#define arithmodel_order_zero_reset(am, eof, esc) ((Arithmodel_order_zero *)am)->my_reset((am), (eof), (esc))
#define arithmodel_order_zero_set_update_escape_freq(am, f) ((Arithmodel_order_zero *)am)->set_update_escape_freq((am), (f))
#define arithmodel_order_zero_set_update_region(am, f) ((Arithmodel_order_zero *)am)->set_update_region((am), (f))

Arithmodel *arithmodel_order_zero_create(void);

#endif
