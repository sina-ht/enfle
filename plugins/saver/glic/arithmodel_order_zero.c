/*
 * arithmodel_order_zero.c -- Order zero statistical model
 * (C)Copyright 2001 by Hiroshi Takekawa
 * Last Modified: Wed Aug 15 18:06:57 2001.
 * $Id: arithmodel_order_zero.c,v 1.7 2001/08/26 00:58:18 sian Exp $
 */

#ifdef HAVE_CONFIG_H
# include "enfle-config.h"
#endif
#define CONFIG_H_INCLUDED
#undef DEBUG

#define REQUIRE_FATAL
#include "common.h"

#include <stdlib.h>

#include "arithcoder.h"
#include "arithmodel.h"
#include "arithmodel_order_zero.h"
#include "arithmodel_utils.h"

typedef _Freq Freq;

#define FREQ_MAX (((Freq)~0) >> 2)
#define IS_EOF_INSTALLED(am) ((am)->is_eof_used)
#define IS_ESCAPE_INSTALLED(am) ((am)->is_escape_used)

ARITHMODEL_METHOD_DECLARE;
static int my_reset(Arithmodel *, int, int);

static int
default_update_escape_freq(Arithmodel *_am, Index index)
{
  Arithmodel_order_zero *am = (Arithmodel_order_zero *)_am;
  Index i;

  for (i = index; i < am->nsymbols; i++)
    am->freq[i]++;

  return 1;
}

static void
normalize_freq(Arithmodel *_am)
{
  Arithmodel_order_zero *am = (Arithmodel_order_zero *)_am;
  int i;

  if (am->freq[am->nsymbols - 1] >= FREQ_MAX) {
    Freq cum, freq;

    cum = (am->freq[0] + 1) >> 1;
    for (i = 1; i < am->nsymbols; i++) {
      freq = (am->freq[i] - am->freq[i - 1] + 1) >> 1;
      am->freq[i - 1] = cum;
      cum += freq;
    }
    am->freq[i - 1] = cum;
  }
}

static void
default_update_region(Arithmodel *_am, Index index)
{
  Arithmodel_order_zero *am = (Arithmodel_order_zero *)_am;
  Arithcoder *ac = am->ac;
  unsigned int tmp = ac->range / am->freq[am->nsymbols - 1];

  debug_message(__FUNCTION__ "(%d, tmp %X)\n", index, tmp);

  if (index > 0) {
    ac->low += tmp * am->freq[index - 1];
    ac->range = tmp * (am->freq[index] - am->freq[index - 1]);
  } else {
    ac->range = tmp * am->freq[0];
  }
}

static void
set_update_escape_freq(Arithmodel *_am, int (*func)(Arithmodel *, Index))
{
  Arithmodel_order_zero *am = (Arithmodel_order_zero *)_am;

  am->update_escape_freq = func;
}

static void
set_update_region(Arithmodel *_am, void (*func)(Arithmodel *, Index))
{
  Arithmodel_order_zero *am = (Arithmodel_order_zero *)_am;

  am->update_region = func;
}

Arithmodel *
arithmodel_order_zero_create(void)
{
  Arithmodel_order_zero *am;

  if ((am = calloc(1, sizeof(Arithmodel_order_zero))) == NULL)
    return NULL;
  am->install_symbol = install_symbol;
  am->uninstall_symbol = uninstall_symbol;
  am->reset = reset;
  am->encode_init = encode_init;
  am->encode = encode;
  am->encode_final = encode_final;
  am->decode_init = decode_init;
  am->decode = decode;
  am->decode_final = decode_final;
  am->destroy = destroy;
  am->my_reset = my_reset;
  am->set_update_escape_freq = set_update_escape_freq;
  am->set_update_region = set_update_region;
  set_update_escape_freq((Arithmodel *)am, default_update_escape_freq);
  set_update_region((Arithmodel *)am, default_update_region);

  return (Arithmodel *)am;
}

static int
install_symbol(Arithmodel *_am, Freq freq)
{
  Arithmodel_order_zero *am = (Arithmodel_order_zero *)_am;
  Freq *new_freq;

  debug_message(__FUNCTION__ "(index %d freq %d)\n", am->nsymbols, freq);

  if ((new_freq = realloc(am->freq, (am->nsymbols + 1) * sizeof(Freq))) == NULL)
    return 0;
  am->freq = new_freq;

  if (am->nsymbols > 0)
    am->freq[am->nsymbols] = am->freq[am->nsymbols - 1] + freq;
  else
    am->freq[0] = freq;
  am->nsymbols++;

  return 1;
}

static int
uninstall_symbol(Arithmodel *_am, Index index)
{
  Arithmodel_order_zero *am = (Arithmodel_order_zero *)_am;
  Freq freq;

  debug_message(__FUNCTION__ "(index %d)\n", index);

  if (index < am->start_symbol)
    am->start_symbol--;
  if (index == am->escape_symbol)
    am->escape_symbol = -1;

  if (index > 0)
    freq = am->freq[index] - am->freq[index - 1];
  else
    freq = am->freq[index];
  for (index++; index < am->nsymbols; index++)
    am->freq[index - 1] = am->freq[index] - freq;

  /* We can shrink the memory size, but leave it as is. */
  am->nsymbols--;

  return 1;
}

static int
init(Arithmodel *_am, int eof_freq, int escape_freq)
{
  Arithmodel_order_zero *am = (Arithmodel_order_zero *)_am;

  am->nsymbols = 0;

  if (eof_freq) {
    am->is_eof_used = 1;
    am->eof_symbol = am->nsymbols;
    install_symbol(_am, eof_freq);
  } else {
    am->is_eof_used = 0;
  }
  if (escape_freq) {
    am->is_escape_used = 1;
    am->escape_symbol = am->nsymbols;
    install_symbol(_am, escape_freq);
  } else {
    am->is_escape_used = 0;
  }
  am->start_symbol = am->nsymbols;
  am->escape_run = 0;

  return 1;
}

static int
reset(Arithmodel *_am)
{
  Arithmodel_order_zero *am = (Arithmodel_order_zero *)_am;

  if (am->freq) {
    free(am->freq);
    am->freq = NULL;
  }

  return init(_am, 0, 0);
}

static int
my_reset(Arithmodel *_am, int eof_freq, int escape_freq)
{
  Arithmodel_order_zero *am = (Arithmodel_order_zero *)_am;

  if (am->freq) {
    free(am->freq);
    am->freq = NULL;
  }

  return init(_am, eof_freq, escape_freq);
}

static int
common_init(Arithmodel *_am, Arithcoder *ac)
{
  Arithmodel_order_zero *am = (Arithmodel_order_zero *)_am;

  if (am->freq)
    fatal(1, __FUNCTION__ ": am->freq != NULL\n");

  am->ac = ac;

  return init(_am, 0, 0);
}

static int
encode_init(Arithmodel *_am, Arithcoder *ac)
{
  return common_init(_am, ac);
}

static void
update_freq(Arithmodel *_am, Index index)
{
  Arithmodel_order_zero *am = (Arithmodel_order_zero *)_am;
  unsigned int i;

  debug_message(__FUNCTION__ "(%d)\n", index);

  if (index == am->escape_symbol && am->update_escape_freq) {
    if (!am->update_escape_freq(_am, index))
      return;
  } else {
    for (i = index; i < am->nsymbols; i++)
      am->freq[i]++;
  }

  normalize_freq(_am);
}

static int
encode_bulk(Arithmodel *_am, Index index)
{
  Arithmodel_order_zero *am = (Arithmodel_order_zero *)_am;

  if (am->nsymbols < index)
    fatal(2, __FUNCTION__ ": nsymbols %d < %d index\n", am->nsymbols, index);

  if (am->nsymbols == index) {
    if (IS_ESCAPE_INSTALLED(am)) {
      if (am->escape_encoded_with_rle) {
	am->escape_run++;
      } else {
	encode_bulk(_am, am->escape_symbol);
      }
      install_symbol(_am, 1);
      return 2;
    } else {
      fatal(3, __FUNCTION__ ": nsymbols %d == %d index, but escape disabled\n", am->nsymbols, index);
    }
  }

  if (am->escape_encoded_with_rle && am->escape_run) {
    int run = am->escape_run;

    am->escape_run = 0;
    encode_bulk(_am, am->escape_symbol);
    arithmodel_encode_delta(am->bin_am, run, 0, 1);
  }

  am->update_region(_am, index);
  arithcoder_encode_renormalize(am->ac);
  update_freq(_am, index);

  return 1;
}

static int
encode(Arithmodel *_am, Index index)
{
  Arithmodel_order_zero *am = (Arithmodel_order_zero *)_am;

  return encode_bulk(_am, index + am->start_symbol);
}

static int
encode_final(Arithmodel *_am)
{
  Arithmodel_order_zero *am = (Arithmodel_order_zero *)_am;

  if (IS_EOF_INSTALLED(am)) {
    encode_bulk(_am, am->eof_symbol);
    encode_bulk(_am, am->eof_symbol);
    //encode_bulk(_am, am->eof_symbol);
  }

  if (am->freq) {
    free(am->freq);
    am->freq = NULL;
  }

  return 1;
}

static int
decode_init(Arithmodel *_am, Arithcoder *ac)
{
  return common_init(_am, ac);
}

static int
decode(Arithmodel *_am, Index *index_return)
{
  Arithmodel_order_zero *am = (Arithmodel_order_zero *)_am;
  Arithcoder *ac = am->ac;
  Freq freq;
  Index left, right, index;
  unsigned int tmp;

  tmp = ac->range / am->freq[am->nsymbols - 1];
  freq = (ac->value - ac->low) / tmp;

  debug_message(__FUNCTION__ ": low %X range %X target %X freq %X\n", ac->low, ac->range, ac->value, freq);

#if 1
  if (am->nsymbols == 2) {
    index =  (am->freq[0] > freq) ? 0 : 1;
  } else {
    left = 0;
    right = am->nsymbols - 1;
    while (right > left) {
      if (am->freq[(left + right) >> 1] <= freq)
	left = ((left + right) >> 1) + 1;
      else
	right = (left + right) >> 1;
    }
    index = right;
    if (index == 0) {
      if (am->freq[0] <= freq)
	fatal(6, __FUNCTION__ ": invalid selection of symbol: index = 0, freq = %d, freq[0] = %d\n", freq, am->freq[0]);
    } else {
      if (am->freq[index - 1] > freq || am->freq[index] <= freq)
	fatal(6, __FUNCTION__ ": invalid selection of symbol: index = %d, freq = %d, freq[index - 1] = %d, freq[index] = %d\n", index, freq, am->freq[index - 1], am->freq[index]);
    }
  }
#else
  for (index = 0; am->freq[index] <= freq; index++) ;
#endif

  am->update_region(_am, index);
  arithcoder_decode_renormalize(am->ac);

  if (IS_EOF_INSTALLED(am) && index == am->eof_symbol)
    return 0;

  update_freq(_am, index);

  if (IS_ESCAPE_INSTALLED(am) && index == am->escape_symbol) {
    *index_return = am->nsymbols - am->start_symbol;
    install_symbol(_am, 1);
    return 2;
  }

  *index_return = index - am->start_symbol;

  return 1;
}

static int
decode_final(Arithmodel *_am)
{
  Arithmodel_order_zero *am = (Arithmodel_order_zero *)_am;

  if (am->freq) {
    free(am->freq);
    am->freq = NULL;
  }

  return 1;
}

static void
destroy(Arithmodel *_am)
{
  Arithmodel_order_zero *am = (Arithmodel_order_zero *)_am;

  if (am->freq)
    free(am->freq);
  free(am);
}
