/*
 * vmpm_decompose_null.c -- Normal decomposer
 * (C)Copyright 2001 by Hiroshi Takekawa
 * Last Modified: Tue Aug  7 22:02:33 2001.
 * $Id: vmpm_decompose_null.c,v 1.3 2001/08/09 17:32:08 sian Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#define DEBUG 1

#include "vmpm.h"
#include "vmpm_hash.h"
#include "vmpm_error.h"
#include "ipow.h"

#include "arithcoder.h"
#include "arithcoder_arith.h"
#include "arithmodel.h"
#include "arithmodel_order_zero.h"
#include "arithmodel_utils.h"

static void init(VMPM *);
static int decompose(VMPM *, int, int, int);
static void encode(VMPM *);
static void decode(VMPM *);
static int reconstruct(VMPM *);
static void final(VMPM *);
static void destroy(VMPM *);

static VMPMDecomposer plugin = {
  name: "Null",
  description: "Null MPM decomposer, that is, order zero arithmetic coder",
  init: init,
  decompose: decompose,
  encode: encode,
  decode: decode,
  reconstruct: reconstruct,
  final: final,
  destroy: destroy,
  next: NULL
};

VMPMDecomposer *
decomposer_init(VMPM *vmpm)
{
  if (vmpm->alphabetsize == 0) {
    vmpm->alphabetsize = 256;
    vmpm->bits_per_symbol = 8;
  }

  return &plugin;
}

static void
init(VMPM *vmpm)
{
}

static int
decompose(VMPM *vmpm, int offset, int level, int blocksize)
{
  return blocksize;
}

static void
encode(VMPM *vmpm)
{
  Arithcoder *ac;
  Arithmodel *am;
  Arithmodel *bin_am;
  unsigned int *symbol_to_index;
  int n;
  unsigned int j;

  //debug_message(__FUNCTION__ "()\n");

  ac = arithcoder_arith_create();
  arithcoder_encode_init(ac, vmpm->outfile);

  am = arithmodel_order_zero_create();
  arithmodel_encode_init(am, ac);
  arithmodel_order_zero_reset(am, 0, 1);

  bin_am = arithmodel_order_zero_create();
  arithmodel_encode_init(bin_am, ac);
  arithmodel_order_zero_reset(bin_am, 0, 0);
  arithmodel_install_symbol(bin_am, 1);
  arithmodel_install_symbol(bin_am, 1);

  if ((symbol_to_index = malloc(vmpm->alphabetsize * sizeof(unsigned int))) == NULL)
    memory_error(NULL, MEMORY_ERROR);
  memset(symbol_to_index, 255, vmpm->alphabetsize * sizeof(unsigned int));

  n = 0;
  for (j = 0; j < vmpm->bufferused; j++) {
    if (symbol_to_index[(int)vmpm->buffer[j]] == (unsigned int)-1) {
      arithmodel_encode(am, n);
      symbol_to_index[(int)vmpm->buffer[j]] = n++;
      arithmodel_encode_bits(bin_am, (int)vmpm->buffer[j], vmpm->bits_per_symbol, 0, 1);
    } else
      arithmodel_encode(am, symbol_to_index[(int)vmpm->buffer[j]]);
  }
  free(symbol_to_index);

  arithmodel_encode_final(bin_am);
  arithmodel_encode_final(am);
  arithcoder_encode_final(ac);

  arithmodel_destroy(bin_am);
  arithmodel_destroy(am);
  arithcoder_destroy(ac);
}

static void
decode(VMPM *vmpm)
{
  Arithcoder *ac;
  Arithmodel *am;
  Arithmodel *bin_am;
  unsigned int *index_to_symbol;
  int i, n;
  unsigned int j;

  //debug_message(__FUNCTION__ "()\n");

  ac = arithcoder_arith_create();
  arithcoder_decode_init(ac, vmpm->infile);

  am = arithmodel_order_zero_create();
  arithmodel_order_zero_reset(am, 0, 1);
  arithmodel_decode_init(am, ac);

  bin_am = arithmodel_order_zero_create();
  arithmodel_decode_init(bin_am, ac);
  arithmodel_order_zero_reset(bin_am, 0, 0);
  arithmodel_install_symbol(bin_am, 1);
  arithmodel_install_symbol(bin_am, 1);

  if ((index_to_symbol = calloc(vmpm->alphabetsize, sizeof(unsigned int))) == NULL)
    memory_error(NULL, MEMORY_ERROR);

  n = 0;
  for (j = 0; j < vmpm->bufferused; j++) {
    Index v;

    if (arithmodel_decode(am, &v) == 2) {
      int v;

      arithmodel_decode_bits(bin_am, &v, vmpm->bits_per_symbol, 0, 1);
      index_to_symbol[n++] = vmpm->buffer[j] = v;
    } else {
      vmpm->buffer[j] = index_to_symbol[v];
    }
  }
  free(index_to_symbol);

  arithcoder_decode_final(ac);

  arithmodel_destroy(am);
  arithcoder_destroy(ac);
}

static int
reconstruct(VMPM *vmpm)
{
  return 1;
}

static void
final(VMPM *vmpm)
{
}

static void
destroy(VMPM *vmpm)
{
}
