/*
 * vmpm_decompose_recur2.c -- Recursive decomposer
 * (C)Copyright 2001 by Hiroshi Takekawa
 * Last Modified: Sat Sep  8 14:34:48 2001.
 * $Id: vmpm_decompose_recur2.c,v 1.2 2001/09/10 00:03:36 sian Exp $
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
  name: "Recur2",
  description: "Recursive MPM decomposer 2",
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
    for (i = 0; i < ntokens; i++) {
      vmpm->token[level][vmpm->token_index[level]] = (Token *)((int)vmpm->buffer[offset + i]);
      vmpm->token_index[level]++;
    }
  }

  if (blocksize - ntokens * token_length > 0)
    decompose(vmpm, offset + ntokens * token_length, level - 1, blocksize - ntokens * token_length);

  return ntokens * token_length;
}

static inline int
calc_highest_level(VMPM *vmpm, int pos)
{
  int i;
  unsigned int r;

  for (i = 1, r = vmpm->r; i < vmpm->I; i++, r *= vmpm->r)
    if (pos % r)
      return i - 1;
  return i;
}

static int
encode_recursively(VMPM *vmpm, Arithmodel **ams, Arithmodel *bin_am, Arithmodel *level_am,
		   unsigned int *s_to_i, int i, int j, unsigned int pos)
{
  unsigned int len;

  if (i == 0) {
    unsigned int nsymbols;
    /* Reach the lowest level. */

    arithmodel_order_zero_encode_with_range(level_am, i, 0, calc_highest_level(vmpm, pos));
    nsymbols = arithmodel_order_zero_nsymbols(ams[0]);
    if (s_to_i[(int)vmpm->token[0][j]] == (unsigned int)-1) {
      arithmodel_encode(ams[0], nsymbols);
      s_to_i[(int)vmpm->token[0][j]] = nsymbols;
      arithmodel_encode_bits(bin_am, (int)vmpm->token[0][j], vmpm->bits_per_symbol, 0, 1);
    } else {
      arithmodel_encode(ams[0], s_to_i[(int)vmpm->token[0][j]]);
    }
    stat_message(vmpm, " %d", (int)vmpm->token[0][j]);
    len = 1;
  } else {
    Token_value tv;

    tv = vmpm->token[i][j]->value - 1;
    if (arithmodel_order_zero_nsymbols(ams[i]) > tv) {
      /* Found. Encode at this level. */
      stat_message(vmpm, "(%d,%d)", i, tv);
      arithmodel_order_zero_encode_with_range(level_am, i, 0, calc_highest_level(vmpm, pos));
      arithmodel_encode(ams[i], tv);
      len = ipow(vmpm->r, i);
    } else if (arithmodel_order_zero_nsymbols(ams[i]) == tv) {
      int k;

      /* Not found. Encode at lower level. */
      arithmodel_install_symbol(ams[i], 1);
      len = 0;
      for (k = 0; k < vmpm->r; k++)
	len += encode_recursively(vmpm, ams, bin_am, level_am, s_to_i, i - 1, tv * vmpm->r + k, pos + len);
      return len;
    } else {
      fatal(254, "nsymbols = %d < %d = tv\n", arithmodel_order_zero_nsymbols(ams[i]), tv);
    }
  }

  if (pos + len >= ipow(vmpm->r, arithmodel_order_zero_nsymbols(level_am)) &&
      arithmodel_order_zero_nsymbols(level_am) <= vmpm->I + 1) {
    debug_message(__FUNCTION__ ": pos %d len %d sum %d r %d nsymbols %d\n", pos, len, pos + len, vmpm->r, arithmodel_order_zero_nsymbols(level_am));
    arithmodel_install_symbol(level_am, 1);
  }

  return len;
}

static void
encode(VMPM *vmpm)
{
  Arithcoder *ac;
  Arithmodel **ams;
  Arithmodel *bin_am;
  Arithmodel *level_am;
  unsigned int *s_to_i;
  int i, match_found, nsymbols;
  unsigned int j, pos;

  //debug_message("Recur2: " __FUNCTION__ "()\n");

  if (!vmpm->outfile) {
    debug_message(__FUNCTION__ ": outfile is NULL.\n");
    return;
  }

  ac = arithcoder_arith_create();
  arithcoder_encode_init(ac, vmpm->outfile);

  match_found = 0;
  for (i = vmpm->I; i >= 1; i--) {
    nsymbols = 0;
    for (j = 0; j < vmpm->token_index[i]; j++) {
      Token *t = vmpm->token[i][j];
      Token_value tv = t->value - 1;

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
  vmpm->I = i;

  if ((ams = calloc(vmpm->I + 1, sizeof(Arithmodel **))) == NULL)
    memory_error(NULL, MEMORY_ERROR);

  for (i = 0; i <= vmpm->I; i++) {
    ams[i] = arithmodel_order_zero_create();
    arithmodel_encode_init(ams[i], ac);
    arithmodel_order_zero_reset(ams[i], 0, 0);
  }
  arithmodel_order_zero_reset(ams[0], 0, 1);

  bin_am = arithmodel_order_zero_create();
  arithmodel_encode_init(bin_am, ac);
  arithmodel_order_zero_reset(bin_am, 0, 0);
  arithmodel_install_symbol(bin_am, 1);
  arithmodel_install_symbol(bin_am, 1);

  level_am = arithmodel_order_zero_create();
  arithmodel_encode_init(level_am, ac);
  arithmodel_order_zero_reset(level_am, 0, 0);
  arithmodel_install_symbol(level_am, 1);

  if ((s_to_i = malloc(vmpm->alphabetsize * sizeof(unsigned int))) == NULL)
    memory_error(NULL, MEMORY_ERROR);
  memset(s_to_i, 255, vmpm->alphabetsize * sizeof(unsigned int));

  pos = 0;
  for (j = 0; j < vmpm->token_index[vmpm->I]; j++)
    pos += encode_recursively(vmpm, ams, bin_am, level_am, s_to_i, vmpm->I, j, pos);
  for (i = vmpm->I - 1; i >= 0; i--) {
    unsigned int n = (vmpm->newtoken[i + 1] - 1) * vmpm->r;

    for (j = n; j < vmpm->token_index[i]; j++)
      pos += encode_recursively(vmpm, ams, bin_am, level_am, s_to_i, i, j, pos);
  }
  stat_message(vmpm, "\n%d symbols encoded.\n", pos);

  free(s_to_i);

  for (i = 0; i <= vmpm->I; i++)
    arithmodel_encode_final(ams[i]);
  arithmodel_encode_final(level_am);
  arithcoder_encode_final(ac);

  for (i = 0; i <= vmpm->I; i++)
    arithmodel_destroy(ams[i]);
  arithmodel_destroy(level_am);
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
  arithcoder_decode_init(ac, vmpm->infile);

  am = arithmodel_order_zero_create();
  arithmodel_decode_init(am, ac);

  bin_am = arithmodel_order_zero_create();
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
    //arithmodel_order_zero_reset(am, 0, vmpm->newtoken[i]);
    arithmodel_order_zero_reset(am, 0, vmpm->token_index[i] >> 2);
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
