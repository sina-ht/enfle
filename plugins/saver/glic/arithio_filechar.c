/*
 * arithio_filechar.c -- Data I/O with character-based symbolization
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * Last Modified: Tue Apr  3 18:56:44 2001.
 * $Id: arithio_filechar.c,v 1.1 2001/04/18 05:43:31 sian Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#define REQUIRE_FATAL
#include "common.h"

#include "arithcoder.h"
#include "arithmodel.h"
#include "arithmodel_order_zero.h"
#include "arithmodel_utils.h"
#include "arithio.h"
#include "arithio_filechar.h"

static int input_init(ArithIO *, Arithmodel *, void *);
static int input(ArithIO *);
static int input_final(ArithIO *);
static int output_init(ArithIO *, Arithmodel *, void *);
static int output(ArithIO *);
static int output_final(ArithIO *);
static void destroy(ArithIO *);

typedef struct _arithio_filechar {
  ARITHIO_BASE_VARIABLES;
  Arithmodel *bin_am;
  FILE *fp;
  Index nsymbols;
  int symbol_to_index[256];
  int index_to_symbol[256];
} ArithIO_filechar;

ArithIO *
arithio_filechar_create(void)
{
  ArithIO_filechar *io;

  if ((io = calloc(1, sizeof(ArithIO_filechar))) == NULL)
    return NULL;
  io->input_init = input_init;
  io->input = input;
  io->input_final = input_final;
  io->output_init = output_init;
  io->output = output;
  io->output_final = output_final;
  io->destroy = destroy;
  io->bin_am = arithmodel_order_zero_create(0, 0);

  return (ArithIO *)io;
}

static int
input_init(ArithIO *_io, Arithmodel *am, void *p)
{
  ArithIO_filechar *io = (ArithIO_filechar *)_io;

  io->am = am;
  io->nsymbols = 0;
  memset(io->symbol_to_index, 255, sizeof(io->symbol_to_index));
  memset(io->index_to_symbol, 255, sizeof(io->index_to_symbol));
  io->fp = (FILE *)p;
  arithmodel_encode_init(io->bin_am, am->ac);
  arithmodel_install_symbol(io->bin_am, 1);
  arithmodel_install_symbol(io->bin_am, 1);

  return 1;
}

static int
input(ArithIO *_io)
{
  ArithIO_filechar *io = (ArithIO_filechar *)_io;
  int c, i;

  while ((c = fgetc(io->fp)) != EOF) {
    if (io->symbol_to_index[c] == -1) {
      io->symbol_to_index[c] = io->nsymbols++;
      if (arithmodel_encode(io->am, io->symbol_to_index[c]) != 2)
	fatal(4, __FUNCTION__ ": escape expected, but arithmodel_encode() didn't tell so.\n");
      debug_message("Escape data: %X\n", c);
      arithmodel_encode_bits(io->bin_am, c, 8, 0, 1);
    } else {
      arithmodel_encode(io->am, io->symbol_to_index[c]);
    }
    debug_message(__FUNCTION__ ": --- %c %X --- \n", c, c);
  }

  return 1;
}

static int
input_final(ArithIO *_io)
{
  ArithIO_filechar *io = (ArithIO_filechar *)_io;

  arithmodel_encode_final(io->bin_am);
  return 1;
}

static int
output_init(ArithIO *_io, Arithmodel *am, void *p)
{
  ArithIO_filechar *io = (ArithIO_filechar *)_io;

  io->am = am;
  io->nsymbols = 0;
  memset(io->symbol_to_index, 255, sizeof(io->symbol_to_index));
  memset(io->index_to_symbol, 255, sizeof(io->index_to_symbol));
  io->fp = (FILE *)p;
  arithmodel_decode_init(io->bin_am, am->ac);
  arithmodel_install_symbol(io->bin_am, 1);
  arithmodel_install_symbol(io->bin_am, 1);

  return 1;
}

static int
output(ArithIO *_io)
{
  ArithIO_filechar *io = (ArithIO_filechar *)_io;
  Index index, b;
  int c, i, ret;

  for (;;) {
    if ((ret = arithmodel_decode(io->am, &index)) < 0)
      break;
    else if (ret == 0) {
      debug_message(__FUNCTION__ ": ret == 0\n");
      return 0;
    }
    else if (ret == 2) {
      if (index == 256)
	fatal(7, "index overflow.\n");
      arithmodel_decode_bits(io->bin_am, &c, 8, 0, 1);
      debug_message("Escape data: %d: %X\n", index, c);

      io->index_to_symbol[index] = c;
    } else {
      c = io->index_to_symbol[index];
      if (c == -1) {
	show_message(__FUNCTION__ ": c == -1, BUG or CORRUPTED. index %d\n", index);
	return 0;
      }
    }
    fputc(c, io->fp);
    debug_message(__FUNCTION__ ": --- %c %X --- \n", c, c);
  }

  return 1;
}

static int
output_final(ArithIO *_io)
{
  ArithIO_filechar *io = (ArithIO_filechar *)_io;

  arithmodel_decode_final(io->bin_am);
  return 1;
}

static void
destroy(ArithIO *_io)
{
  ArithIO_filechar *io = (ArithIO_filechar *)_io;

  if (io->bin_am)
    arithmodel_destroy(io->bin_am);
  free(io);
}
