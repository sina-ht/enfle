/*
 * vmpm_decompose_esc.c -- ESC estimatation method A decomposer
 * (C)Copyright 2001 by Hiroshi Takekawa
 * Last Modified: Mon Aug  6 22:21:01 2001.
 * $Id: vmpm_decompose_esc.c,v 1.5 2001/08/06 18:51:42 sian Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#define REQUIRE_STRING_H
#include "compat.h"
#define REQUIRE_FATAL
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
  name: "ESC-A",
  description: "ESC estimation method A MPM decomposer",
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
	if (level > 0)
	  result = decompose(vmpm, offset + i * token_length, level - 1, token_length);
      }
      vmpm->token_index[level]++;
    }
  } else {
    for (i = 0; i < ntokens; i++) {
      vmpm->token[level][vmpm->token_index[level]] = (Token *)((int)vmpm->buffer[offset + i]);
      vmpm->token_index[level]++;
    }
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
  Arithmodel *char_am;
  Arithmodel *am;
  Arithmodel *bin_am;
  unsigned int *symbol_to_index;
  int i, match_found;
  unsigned int nsymbols;
  unsigned int j;

  //debug_message(__FUNCTION__ "()\n");

  if (!vmpm->outfile) {
    debug_message(__FUNCTION__ ": outfile is NULL.\n");
    return;
  }

  ac = arithcoder_arith_create();
  arithcoder_encode_init(ac, vmpm->outfile);

  char_am = arithmodel_order_zero_create(0, 1);
  arithmodel_encode_init(char_am, ac);
  arithmodel_order_zero_set_update_escape_freq(char_am, update_escape_freq);

  am = arithmodel_order_zero_create(0, 0);
  arithmodel_encode_init(am, ac);

  bin_am = arithmodel_order_zero_create(0, 0);
  arithmodel_encode_init(bin_am, ac);

  match_found = 0;
  for (i = vmpm->I; i >= 1; i--) {
    nsymbols = 0;
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
      stat_message(vmpm, "Level %d (%d tokens, %d distinct): ", i, vmpm->token_index[i], vmpm->newtoken[i] - 1);
      /* Encode escape symbols at once. */
      arithmodel_order_zero_reset(bin_am, 0, 0);
      arithmodel_install_symbol(bin_am, 1);
      arithmodel_install_symbol(bin_am, 1);
      if (vmpm->token[i][0]->value != 1)
	generic_error((char *)"Invalid token value.\n", INVALID_TOKEN_VALUE_ERROR);
      /* Hence, we don't need to encode it. */
      stat_message(vmpm, "e ");
      nsymbols = 1;
#if 1
      arithmodel_order_zero_reset(am, 0, 0);
      arithmodel_install_symbol(am, 1);
      for (j = 1; j < vmpm->token_index[i]; j++) {
	Token_value tv = vmpm->token[i][j]->value - 1;

	if (nsymbols == tv) {
	  stat_message(vmpm, "e ");
	  nsymbols++;
	  arithmodel_encode(bin_am, 1);
	  arithmodel_install_symbol(am, 1);
	} else {
	  stat_message(vmpm, "%d ", tv);
	  arithmodel_encode(bin_am, 0);
	  arithmodel_encode(am, tv);
	}
      }
#else
      for (j = 1; j < vmpm->token_index[i]; j++) {
	Token_value tv = vmpm->token[i][j]->value - 1;

	if (nsymbols == tv) {
	  stat_message(vmpm, "e ");
	  nsymbols++;
	  arithmodel_encode(bin_am, 1);
	} else {
	  stat_message(vmpm, "%d ", tv);
	  arithmodel_encode(bin_am, 0);
	}
      }
      /* Then encode non-escape symbols */
      arithmodel_order_zero_reset(am, 0, 0);
      arithmodel_install_symbol(am, 1);
      nsymbols = 1;
      for (j = 1; j < vmpm->token_index[i]; j++) {
        Token *t = vmpm->token[i][j];
        Token_value tv = t->value - 1;

        if (nsymbols == tv) {
          nsymbols++;
	  arithmodel_install_symbol(am, 1);
        } else {
	  arithmodel_encode(am, tv);
        }
      }
#endif
      stat_message(vmpm, "\n");
    }
  }

  if ((symbol_to_index = malloc(vmpm->alphabetsize * sizeof(unsigned int))) == NULL)
    memory_error(NULL, MEMORY_ERROR);
  memset(symbol_to_index, 255, vmpm->alphabetsize * sizeof(unsigned int));

  nsymbols = 0;
  arithmodel_order_zero_reset(bin_am, 0, 0);
  arithmodel_install_symbol(bin_am, 1);
  arithmodel_install_symbol(bin_am, 1);
  arithmodel_order_zero_reset(char_am, 1, vmpm->alphabetsize - 1);
  stat_message(vmpm, "Level 0 (%d tokens): ", vmpm->token_index[0]);
  for (j = 0; j < vmpm->token_index[0]; j++) {
    if (symbol_to_index[(int)vmpm->token[0][j]] == (unsigned int)-1) {
      stat_message(vmpm, "e ");
      arithmodel_encode(char_am, nsymbols);
      symbol_to_index[(int)vmpm->token[0][j]] = nsymbols++;
      arithmodel_encode_bits(bin_am, (int)vmpm->token[0][j], vmpm->bits_per_symbol, 0, 1);
    } else {
      stat_message(vmpm, "%d ", symbol_to_index[(int)vmpm->token[0][j]]);
      arithmodel_encode(char_am, symbol_to_index[(int)vmpm->token[0][j]]);
    }
  }
  stat_message(vmpm, "\n");
  free(symbol_to_index);

  arithmodel_encode_final(bin_am);
  arithmodel_encode_final(char_am);
  arithcoder_encode_final(ac);

  arithmodel_destroy(bin_am);
  arithmodel_destroy(char_am);
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
  fatal(255, "DECODING IS INVALID. NEED REIMPLEMENTATION.\n");

  ac = arithcoder_arith_create();
  am = arithmodel_order_zero_create(1, 1);

  arithcoder_decode_init(ac, vmpm->infile);
  arithmodel_decode_init(am, ac);

  bin_am = arithmodel_order_zero_create(0, 0);
  arithmodel_decode_init(bin_am, ac);
  arithmodel_install_symbol(bin_am, 1);
  arithmodel_install_symbol(bin_am, 1);

  vmpm->token_index[vmpm->I] = vmpm->blocksize / ipow(vmpm->r, vmpm->I);
  for (i = vmpm->I; i >= 1; i--) {
    stat_message(vmpm, "Level %d: %d\n", i, vmpm->token_index[i]);
    vmpm->token_index[i - 1] = 0;
    if ((vmpm->tokens[i] = calloc(vmpm->token_index[i], sizeof(Token))) == NULL)
      memory_error(NULL, MEMORY_ERROR);
    /* newtoken[] will not be known by decoder without sending. */
    //arithmodel_order_zero_reset(am, 1, vmpm->newtoken[i]);
    arithmodel_order_zero_reset(am, 1, vmpm->token_index[i] >> 2);
    for (j = 0; j < vmpm->token_index[i]; j++) {
      Index index;

      arithmodel_decode(am, &index);
      vmpm->tokens[i][j].value++;
      if (vmpm->tokens[i][j].value > vmpm->token_index[i - 1]) {
	if (vmpm->tokens[i][j].value == vmpm->token_index[i - 1] + 1)
	  vmpm->token_index[i - 1]++;
	else
	  generic_error((char *)"Invalid token value.\n", INVALID_TOKEN_VALUE_ERROR);
      }
    }
    vmpm->token_index[i - 1] *= vmpm->r;
  }

  if ((index_to_symbol = calloc(vmpm->alphabetsize, sizeof(unsigned int))) == NULL)
    memory_error(NULL, MEMORY_ERROR);

  n = 0;
  stat_message(vmpm, "Level 0: %d\n", vmpm->token_index[0]);
  for (j = 0; j < vmpm->token_index[0]; j++) {
    Index v;

    if (arithmodel_decode(am, &v) == 2) {
      arithmodel_decode_bits(bin_am, &vmpm->tokens[0][j].value, vmpm->bits_per_symbol, 0, 1);
      index_to_symbol[n++] = vmpm->tokens[0][j].value;
    } else {
      vmpm->tokens[0][j].value = index_to_symbol[v];
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
