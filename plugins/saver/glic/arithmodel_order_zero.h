/*
 * arithmodel_order_zero.h -- Order zero statistical model
 * (C)Copyright 2001 by Hiroshi Takekawa
 * Last Modified: Mon Jul 16 17:54:26 2001.
 * $Id: arithmodel_order_zero.h,v 1.2 2001/07/17 12:22:51 sian Exp $
 */

#ifndef _ARITHMODEL_ORDER_ZERO_H
#define _ARITHMODEL_ORDER_ZERO_H

typedef unsigned int _Freq;
typedef struct _arithmodel_order_zero Arithmodel_order_zero;

#undef ESCAPE_RUN

struct _arithmodel_order_zero {
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
#ifdef ESCAPE_RUN
  int escape_run;
  Arithmodel *bin_am;
#endif
  _Freq *freq;
};

#define arithmodel_order_zero_reset(am, eof, esc) ((Arithmodel_order_zero *)am)->my_reset((am), (eof), (esc))
#define arithmodel_order_zero_set_update_escape_freq(am, f) ((Arithmodel_order_zero *)am)->set_update_escape_freq((am), (f))

Arithmodel *arithmodel_order_zero_create(int, int);

#endif
