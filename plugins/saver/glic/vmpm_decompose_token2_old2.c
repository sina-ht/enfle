/*
 * vmpm_decompose_token -- Token decomposer
 * (C)Copyright 2001 by Hiroshi Takekawa
 * Last Modified: Fri Apr 20 18:44:11 2001.
 * $Id: vmpm_decompose_token2_old2.c,v 1.2 2001/04/21 07:28:07 sian Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "vmpm.h"
#include "vmpm_hash.h"
#include "vmpm_error.h"
#include "ipow.h"

#include "arithcoder.h"
#include "arithcoder_arith.h"
#include "arithmodel.h"
#include "arithmodel_order_zero.h"
#include "arithmodel_utils.h"

#define NEW_TOKEN          (1 << 0)
#define SHIFTED_TOKEN      (1 << 1)
#define USED_SHIFTED_TOKEN (1 << 2)

typedef struct _vmpmdecomposer_token {
  unsigned char **new_token_flag;
} VMPMDecomposer_Token;

static void init(VMPM *);
static int decompose(VMPM *, int, int, int);
static void encode(VMPM *);
static void decode(VMPM *);
static int reconstruct(VMPM *);
static void final(VMPM *);
static void destroy(VMPM *);

static VMPMDecomposer plugin = {
  name: "Token2_old2",
  description: "New token shift by 1 symbol decomposer",
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
  if ((vmpm->method_private = calloc(1, sizeof(VMPMDecomposer_Token))) == NULL)
    return NULL;

  if (vmpm->alphabetsize == 0) {
    vmpm->alphabetsize = 256;
    vmpm->bits_per_symbol = 8;
  }

  return &plugin;
}

static void
init(VMPM *vmpm)
{
  VMPMDecomposer_Token *d = (VMPMDecomposer_Token *)vmpm->method_private;
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

  if ((d->new_token_flag = calloc(vmpm->I + 1, sizeof(unsigned char *))) == NULL)
    memory_error(NULL, MEMORY_ERROR);
}

static int
decompose(VMPM *vmpm, int offset, int level, int blocksize)
{
  VMPMDecomposer_Token *d = (VMPMDecomposer_Token *)vmpm->method_private;
  int token_length = 0;
  int ntokens = 0;
  int i, result;
  Token *t;
  Token **tmp;
  unsigned char *tmp2;

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
  if ((tmp2 = realloc(d->new_token_flag[level],
		      (vmpm->token_index[level] + ntokens) * sizeof(unsigned char))) == NULL)
    memory_error(NULL, MEMORY_ERROR);
  d->new_token_flag[level] = tmp2;

  if (level >= 1) {
    for (i = 0; i < ntokens; i++) {
      if (register_to_token_hash(vmpm, offset + i * token_length, token_length, USED_FLAG, vmpm->newtoken[level], &t)) {
	/* already registered token */
	if (!is_used_token(t)) {
	  set_token_used(t);
	  d->new_token_flag[level][vmpm->token_index[level]] |= (unsigned char)SHIFTED_TOKEN;
	  d->new_token_flag[level][t->newindex] |= USED_SHIFTED_TOKEN;
	}
	vmpm->token[level][vmpm->token_index[level]] = t;
      } else {
	/* newly registered token */
	vmpm->token[level][vmpm->token_index[level]] = t;
	d->new_token_flag[level][vmpm->token_index[level]] |= (unsigned char)NEW_TOKEN;
	vmpm->newtoken[level]++;

	/* Register a shifted string */
	if (i > 0) {
	  if (!register_to_token_hash(vmpm,
				      offset + i * token_length - 1,
				      token_length, EXTRA_FLAG, vmpm->newtoken[level], &t))
	    t->newindex = vmpm->token_index[level];
	}

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

static void
encode(VMPM *vmpm)
{
  VMPMDecomposer_Token *d = (VMPMDecomposer_Token *)vmpm->method_private;
  Arithcoder *ac;
  Arithmodel *am;
  Arithmodel *bin_am;
  unsigned int *symbol_to_index;
  int i, n;
  unsigned int j;

  ac = arithcoder_arith_create();
  am = arithmodel_order_zero_create(1, 1);

  arithcoder_encode_init(ac, vmpm->outfile);
  arithmodel_encode_init(am, ac);

  bin_am = arithmodel_order_zero_create(0, 0);
  arithmodel_encode_init(bin_am, ac);
  arithmodel_install_symbol(bin_am, 1);
  arithmodel_install_symbol(bin_am, 1);

  for (i = vmpm->I; i >= 1; i--) {
    stat_message(vmpm, "level %d: %d\n", i, vmpm->token_index[i]);
    for (j = 0; j < vmpm->token_index[i]; j++) {
      Token *t = vmpm->token[i][j];

      if (d->new_token_flag[i][j] & NEW_TOKEN) {
	arithmodel_encode(am, t->value - 1);
      } else {
	arithmodel_encode(am, t->value - 1);
	arithmodel_encode(bin_am, is_extra_token(t) ? 1 : 0);
      }
    }
    arithmodel_reset(am);
  }

  if ((symbol_to_index = malloc(vmpm->alphabetsize * sizeof(unsigned int))) == NULL)
    memory_error(NULL, MEMORY_ERROR);
  memset(symbol_to_index, 255, vmpm->alphabetsize * sizeof(unsigned int));

  n = 0;
  stat_message(vmpm, "level 0: %d\n", vmpm->token_index[0]);
  for (j = 0; j < vmpm->token_index[0]; j++) {
    if (symbol_to_index[(int)vmpm->token[0][j]] == (unsigned int)-1) {
      arithmodel_encode(am, n);
      symbol_to_index[(int)vmpm->token[0][j]] = n++;
      arithmodel_encode_bits(bin_am, (int)vmpm->token[0][j], vmpm->bits_per_symbol, 0, 1);
    } else
      arithmodel_encode(am, symbol_to_index[(int)vmpm->token[0][j]]);
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
}

static int
reconstruct(VMPM *vmpm)
{
  return 0;
}

static void
final(VMPM *vmpm)
{
  VMPMDecomposer_Token *d = (VMPMDecomposer_Token *)vmpm->method_private;
  int i;

  if (d->new_token_flag)
    free(d->new_token_flag);

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
  if (vmpm->method_private)
    free(vmpm->method_private);
}
