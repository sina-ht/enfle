/*
 * arithio.h -- Data I/O with symbolization
 * (C)Copyright 2001 by Hiroshi Takekawa
 * Last Modified: Wed Mar 28 10:29:09 2001.
 * $Id: arithio.h,v 1.1 2001/04/18 05:43:31 sian Exp $
 */

#ifndef _ARITHIO_H
#define _ARITHIO_H

typedef struct _arithio ArithIO;

#define ARITHIO_BASE_VARIABLES \
  Arithmodel *am; \
  int (*input_init)(ArithIO *, Arithmodel *, void *); \
  int (*input)(ArithIO *); \
  int (*input_final)(ArithIO *); \
  int (*output_init)(ArithIO *, Arithmodel *, void *); \
  int (*output)(ArithIO *); \
  int (*output_final)(ArithIO *); \
  void (*destroy)(ArithIO *)

struct _arithio {
  ARITHIO_BASE_VARIABLES;
};

#define arithio_input_init(io, m, p) (io)->input_init((io), (m), (p))
#define arithio_input(io) (io)->input((io))
#define arithio_input_final(io) (io)->input_final((io))
#define arithio_output_init(io, m, p) (io)->output_init((io), (m), (p))
#define arithio_output(io) (io)->output((io))
#define arithio_output_final(io) (io)->output_final((io))
#define arithio_destroy(io) (io)->destroy((io))

#endif
