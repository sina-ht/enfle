/*
 * vmpm_decompose_normal.c -- Original decomposer
 * (C)Copyright 2001 by Hiroshi Takekawa
 * Last Modified: Mon Aug  6 18:47:14 2001.
 * $Id: vmpm_decompose_normal_rle.c,v 1.2 2001/08/06 18:51:42 sian Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

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
  name: "OriginalRLE",
  description: "Refined original MPM decomposer (RLE)",
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
  int i;

  if ((vmpm->token_hash = malloc(HASH_SIZE * sizeof(Token))) == NULL)
    memory_error(NULL, MEMORY_ERROR);
  if ((vmpm->token = calloc(vmpm->I + 1, sizeof(Token **))) == NULL)
    memory_error(NULL, MEMORY_ERROR);
  if ((vmpm->token_index = calloc(vmpm->I + 1, sizeof(unsigned int))) == NULL)
    memory_error(NULL, MEMORY_ERROR);
  if ((vmpm->newtoken = malloc((vmpm->I + 1) * sizeof(Token_value))) == NULL)
    memory_error(NULL, MEMORY_ERROR);
  for (i = 1; i <= vmpm->I; i++)
    vmpm->newtoken[i] = 1;
}

static int
decompose(VMPM *vmpm, int offset, int level, int blocksize)
{
  int token_length = 0;
  int ntokens = 0;
  int i, result;
  Token *t;
  Token **tmp;

  for (; level >= 0; level--) {
    //debug_message(__FUNCTION__ "(%d, %d, %d)\n", offset, level, blocksize);

    token_length = ipow(vmpm->r, level);
    ntokens = blocksize / token_length;
    if (ntokens > 0)
      break;
  }
  if (level == -1)
    return -1;

  if ((tmp = realloc(vmpm->token[level],
		     (vmpm->token_index[level] + ntokens) * sizeof(Token *))) == NULL)
    memory_error(NULL, MEMORY_ERROR);
  vmpm->token[level] = tmp;

  if (level >= 1) {
    for (i = 0; i < ntokens; i++) {
      if (register_to_token_hash(vmpm, offset + i * token_length, token_length, USED_FLAG, vmpm->newtoken[level], &t)) {
	/* already registered token */
	vmpm->token[level][vmpm->token_index[level]] = t;
      } else {
	/* newly registered token */
	vmpm->token[level][vmpm->token_index[level]] = t;
	vmpm->newtoken[level]++;
	result = decompose(vmpm, offset + i * token_length, level - 1, token_length);
      }
      vmpm->token_index[level]++;
    }
  } else {
    for (i = 0; i < ntokens; i++)
      vmpm->token[level][vmpm->token_index[level]++] = (Token *)((int)vmpm->buffer[offset + i]);
  }

  if (blocksize - ntokens * token_length > 0)
    decompose(vmpm, offset + ntokens * token_length, level - 1, blocksize - ntokens * token_length);

  return ntokens * token_length;
}

static int
update_escape_freq(Arithmodel *_am, Index index)
{
  /* No increment */
  return 0;
}

static void
encode(VMPM *vmpm)
{
  Arithcoder *ac;
  Arithmodel *am;
  Arithmodel *bin_am;
  unsigned int *symbol_to_index;
  int i, n, match_found;
  unsigned int j;

  //debug_message(__FUNCTION__ "()\n");

  if (!vmpm->outfile) {
    debug_message(__FUNCTION__ ": outfile is NULL.\n");
    return;
  }

  ac = arithcoder_arith_create();
  arithcoder_encode_init(ac, vmpm->outfile);

  /* EOF won't be used. ESC will be used. */
  am = arithmodel_order_zero_create(0, 1);
  arithmodel_encode_init(am, ac);
  arithmodel_order_zero_set_update_escape_freq(am, update_escape_freq);

  bin_am = arithmodel_order_zero_create(0, 0);
  arithmodel_encode_init(bin_am, ac);
  {
    Arithmodel_order_zero *am_oz = (Arithmodel_order_zero *)am;
    am_oz->bin_am = bin_am;
    am_oz->escape_encoded_with_rle = 1;
  }

  match_found = 0;
  for (i = vmpm->I; i >= 1; i--) {
    int nsymbols = 0;

    for (j = 0; j < vmpm->token_index[i]; j++) {
      Token_value tv = vmpm->token[i][j]->value - 1;

      if (nsymbols == tv) {
	nsymbols++;
      } else {
	match_found++;
	break;
      }
    }
    if (match_found) {
      stat_message(vmpm, "Match found at Level %d\n", i);
      break;
    }
  }
      
  fputc(i, vmpm->outfile);
  if (match_found) {
    for (; i >= 1; i--) {
      int nsymbols = 0;

      stat_message(vmpm, "Level %d (%d tokens, %d distinct): ", i, vmpm->token_index[i], vmpm->newtoken[i] - 1);
      arithmodel_order_zero_reset(bin_am, 0, 0);
      arithmodel_install_symbol(bin_am, 1);
      arithmodel_install_symbol(bin_am, 1);
      /* Send the number of distinct symbols. */
      arithmodel_encode_cbt(bin_am, vmpm->newtoken[i] - 1, vmpm->token_index[i], 0, 1);
      arithmodel_order_zero_reset(am, 1, vmpm->newtoken[i]);

      /* The first token of each level must be t_0. */
      if (vmpm->token[i][0]->value != 1)
	generic_error((char *)"Invalid token value.\n", INVALID_TOKEN_VALUE_ERROR);
      /* Hence, we don't need to encode it. */
      stat_message(vmpm, "e ");
      arithmodel_install_symbol(am, 1);
      nsymbols++;
      for (j = 1; j < vmpm->token_index[i]; j++) {
	Token_value tv = vmpm->token[i][j]->value - 1;

	if (nsymbols == tv) {
	  stat_message(vmpm, "e ");
	  nsymbols++;
	} else {
	  stat_message(vmpm, "%d ", tv);
	}

	arithmodel_encode(am, tv);
      }
      stat_message(vmpm, "\n");
    }
  }

  if ((symbol_to_index = malloc(vmpm->alphabetsize * sizeof(unsigned int))) == NULL)
    memory_error(NULL, MEMORY_ERROR);
  memset(symbol_to_index, 255, vmpm->alphabetsize * sizeof(unsigned int));

  n = 0;
  arithmodel_order_zero_reset(am, 1, vmpm->alphabetsize - 1);
  arithmodel_order_zero_reset(bin_am, 0, 0);
  arithmodel_install_symbol(bin_am, 1);
  arithmodel_install_symbol(bin_am, 1);
  stat_message(vmpm, "Level 0 (%d tokens): ", vmpm->token_index[0]);
  for (j = 0; j < vmpm->token_index[0]; j++) {
    if (symbol_to_index[(int)vmpm->token[0][j]] == (unsigned int)-1) {
      stat_message(vmpm, "e ");
      arithmodel_encode(am, n);
      symbol_to_index[(int)vmpm->token[0][j]] = n++;
      arithmodel_encode_bits(bin_am, (int)vmpm->token[0][j], vmpm->bits_per_symbol, 0, 1);
    } else {
      stat_message(vmpm, "%d ", symbol_to_index[(int)vmpm->token[0][j]]);
      arithmodel_encode(am, symbol_to_index[(int)vmpm->token[0][j]]);
    }
  }
  stat_message(vmpm, "\n");
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

  am = arithmodel_order_zero_create(0, 1);
  arithmodel_decode_init(am, ac);

  bin_am = arithmodel_order_zero_create(0, 0);
  arithmodel_decode_init(bin_am, ac);

  i = fgetc(vmpm->outfile);
  vmpm->token_index[i] = vmpm->blocksize / ipow(vmpm->r, i);
  for (; i >= 1; i--) {
    unsigned int tmp;

    arithmodel_order_zero_reset(bin_am, 0, 0);
    arithmodel_install_symbol(bin_am, 1);
    arithmodel_install_symbol(bin_am, 1);
    arithmodel_decode_cbt(bin_am, &tmp, vmpm->token_index[i], 0, 1);
    vmpm->newtoken[i] = tmp + 1;
    stat_message(vmpm, "D:Level %d (%d tokens, %d distinct): ", i, vmpm->token_index[i], vmpm->newtoken[i] - 1);
    vmpm->token_index[i - 1] = 0;
    if ((vmpm->tokens[i] = calloc(vmpm->token_index[i], sizeof(Token))) == NULL)
      memory_error(NULL, MEMORY_ERROR);
    arithmodel_order_zero_reset(am, 1, vmpm->newtoken[i]);
    vmpm->tokens[i][0].value = 1;
    for (j = 1; j < vmpm->token_index[i]; j++) {
      Index index;

      arithmodel_decode(am, &index);
      vmpm->tokens[i][j].value = index + 1;
      if (vmpm->tokens[i][j].value > vmpm->token_index[i - 1]) {
	if (vmpm->tokens[i][j].value == vmpm->token_index[i - 1] + 1) {
	  stat_message(vmpm, "e ");
	  vmpm->token_index[i - 1]++;
	} else
	  generic_error((char *)"Invalid token value.\n", INVALID_TOKEN_VALUE_ERROR);
      } else {
	stat_message(vmpm, "%d ", index);
      }
    }
    vmpm->token_index[i - 1] *= vmpm->r;
    stat_message(vmpm, "\n");
  }

  if ((index_to_symbol = calloc(vmpm->alphabetsize, sizeof(unsigned int))) == NULL)
    memory_error(NULL, MEMORY_ERROR);

  n = 0;
  arithmodel_order_zero_reset(am, 1, vmpm->alphabetsize - 1);
  arithmodel_order_zero_reset(bin_am, 0, 0);
  arithmodel_install_symbol(bin_am, 1);
  arithmodel_install_symbol(bin_am, 1);
  stat_message(vmpm, "D:Level 0 (%d tokens): ", vmpm->token_index[0]);
  for (j = 0; j < vmpm->token_index[0]; j++) {
    Index v;

    if (arithmodel_decode(am, &v) == 2) {
      stat_message(vmpm, "e ");
      arithmodel_decode_bits(bin_am, &vmpm->tokens[0][j].value, vmpm->bits_per_symbol, 0, 1);
      index_to_symbol[n++] = vmpm->tokens[0][j].value;
    } else {
      vmpm->tokens[0][j].value = index_to_symbol[v];
      stat_message(vmpm, "%d ", v);
    }
  }
  stat_message(vmpm, "\n");
  free(index_to_symbol);

  arithcoder_decode_final(ac);

  arithmodel_destroy(bin_am);
  arithmodel_destroy(am);
  arithcoder_destroy(ac);
}

static int
reconstruct(VMPM *vmpm)
{
  fprintf(stderr, __FUNCTION__ "() not yet implemented.\n");
  return 0;
}

static void
final(VMPM *vmpm)
{
  int i;

  for (i = 0; i <= vmpm->I; i++)
    if (vmpm->token[i])
      free(vmpm->token[i]);
  free(vmpm->newtoken);
  free(vmpm->token_index);
  free(vmpm->token);
  free(vmpm->token_hash);
}

static void
destroy(VMPM *vmpm)
{
}