/*
 * arithmodel_order_zero.h -- Order zero statistical model
 * (C)Copyright 2001 by Hiroshi Takekawa
 * Last Modified: Thu Apr  5 17:53:00 2001.
 * $Id: arithmodel_order_zero.h,v 1.1 2001/04/18 05:43:31 sian Exp $
 */

#ifndef _ARITHMODEL_ORDER_ZERO_H
#define _ARITHMODEL_ORDER_ZERO_H

typedef unsigned int _Freq;

typedef struct _arithmodel_order_zero {
  ARITHMODEL_BASE_VARIABLES;
  int (*my_reset)(Arithmodel *, int, int);
  void (*set_update_escape_freq)(Arithmodel *, int (*)(Arithmodel *, Index));
  int (*update_escape_freq)(Arithmodel *, Index);
  int default_initial_eof_freq;
  int initial_eof_freq;
  Index eof_symbol;
  int default_initial_escape_freq;
  int initial_escape_freq;
  Index escape_symbol;
  Index start_symbol;
  _Freq *freq;
} Arithmodel_order_zero;

#define arithmodel_order_zero_reset(am, eof, esc) ((Arithmodel_order_zero *)am)->my_reset((am), (eof), (esc))
#define arithmodel_order_zero_set_update_escape_freq(am, f) ((Arithmodel_order_zero *)am)->set_update_escape_freq((am), (f))

Arithmodel *arithmodel_order_zero_create(int, int);

#endif
