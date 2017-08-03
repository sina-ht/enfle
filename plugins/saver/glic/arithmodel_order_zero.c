/*
 * arithmodel_order_zero.c -- Order zero statistical model
 * (C)Copyright 2001 by Hiroshi Takekawa
 * Last Modified: Fri Feb 22 17:51:48 2002.
 */

#ifdef HAVE_CONFIG_H
# include "enfle-config.h"
#endif
#define CONFIG_H_INCLUDED
#undef DEBUG

#define REQUIRE_STRING_H
#include "compat.h"
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

#define AMOZ(am) ((Arithmodel_order_zero *)(am))
#define AMOZ_START_SYMBOL(am) (((Arithmodel_order_zero *)(am))->start_symbol)

ARITHMODEL_ORDER_ZERO_METHOD_DECLARE;

static Arithmodel_order_zero template = {
  install_symbol: install_symbol,
  uninstall_symbol: uninstall_symbol,
  reset: reset,
  encode_init: encode_init,
  encode: encode,
  encode_with_range: encode_with_range,
  encode_final: encode_final,
  decode_init: decode_init,
  decode: decode,
  decode_with_range: decode_with_range,
  decode_final: decode_final,
  destroy: destroy,
  my_reset: my_reset,
  get_freq: get_freq,
  set_update_escape_freq: set_update_escape_freq,
  private_data: NULL,
  is_eof_used: 0,
  is_escape_used: 0,
  escape_encoded_with_rle: 0,
  escape_run: 0,
  bin_am: NULL,
  freq: NULL
};

static int
default_update_escape_freq(Arithmodel *_am, Index idx)
{
  Arithmodel_order_zero *am = AMOZ(_am);
  Index i;

  for (i = idx; i < am->nsymbols; i++)
    am->freq[i]++;

  return 1;
}

static void
normalize_freq(Arithmodel *_am)
{
  Arithmodel_order_zero *am = AMOZ(_am);
  unsigned int i;

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
update_region_with_range(Arithmodel *_am, Index idx, Index low, Index high)
{
  Arithmodel_order_zero *am = AMOZ(_am);
  Arithcoder *ac = am->ac;
  unsigned int lowest, highest, tmp;

  lowest = (low > 0) ? am->freq[low - 1] : 0;
  highest = am->freq[high] - lowest;
  tmp = ac->range / highest;

  debug_message_fn("(%d, tmp %X, lowest %X, highest %X)\n", idx, tmp, lowest, highest);

  if (idx > low) {
    ac->low += tmp * (am->freq[idx - 1] - lowest);
    ac->range = tmp * (am->freq[idx] - am->freq[idx - 1]);
  } else if (idx == low) {
    ac->range = tmp * (am->freq[low] - lowest);
  } else {
    show_message_fnc("idx == %d < %d == low.\n", idx, low);
  }
}

static void
set_update_escape_freq(Arithmodel *_am, int (*func)(Arithmodel *, Index))
{
  Arithmodel_order_zero *am = AMOZ(_am);

  am->update_escape_freq = func;
}

Arithmodel *
arithmodel_order_zero_create(void)
{
  Arithmodel_order_zero *am;

  if ((am = calloc(1, sizeof(Arithmodel_order_zero))) == NULL)
    return NULL;
  memcpy(am, &template, sizeof(Arithmodel_order_zero));

  set_update_escape_freq((Arithmodel *)am, default_update_escape_freq);

  return (Arithmodel *)am;
}

static int
install_symbol(Arithmodel *_am, Freq freq)
{
  Arithmodel_order_zero *am = AMOZ(_am);
  Freq *new_freq;

  debug_message_fn("(index %d freq %d)\n", am->nsymbols, freq);

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
uninstall_symbol(Arithmodel *_am, Index idx)
{
  Arithmodel_order_zero *am = AMOZ(_am);
  Freq freq;

  debug_message_fn("(idx %d)\n", idx);

  if (idx < am->start_symbol)
    am->start_symbol--;
  if (idx == am->escape_symbol)
    am->escape_symbol = -1;

  if (idx > 0)
    freq = am->freq[idx] - am->freq[idx - 1];
  else
    freq = am->freq[idx];
  for (idx++; idx < am->nsymbols; idx++)
    am->freq[idx - 1] = am->freq[idx] - freq;

  /* We can shrink the memory size, but leave it as is. */
  am->nsymbols--;

  return 1;
}

static int
init(Arithmodel *_am, int eof_freq, int escape_freq)
{
  Arithmodel_order_zero *am = AMOZ(_am);

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
  Arithmodel_order_zero *am = AMOZ(_am);

  if (am->freq) {
    free(am->freq);
    am->freq = NULL;
  }

  return init(_am, 0, 0);
}

static int
my_reset(Arithmodel *_am, int eof_freq, int escape_freq)
{
  Arithmodel_order_zero *am = AMOZ(_am);

  if (am->freq) {
    free(am->freq);
    am->freq = NULL;
  }

  return init(_am, eof_freq, escape_freq);
}

static unsigned int
get_freq(Arithmodel *_am, unsigned int n)
{
  Arithmodel_order_zero *am = AMOZ(_am);
  
  if (n > 0)
    return am->freq[n] - am->freq[n - 1];
  return am->freq[0];
}

static int
common_init(Arithmodel *_am, Arithcoder *ac)
{
  Arithmodel_order_zero *am = AMOZ(_am);

  if (am->freq)
    fatal("%s: am->freq != NULL\n", __FUNCTION__);

  am->ac = ac;

  return init(_am, 0, 0);
}

static int
encode_init(Arithmodel *_am, Arithcoder *ac)
{
  return common_init(_am, ac);
}

static void
update_freq(Arithmodel *_am, Index idx)
{
  Arithmodel_order_zero *am = AMOZ(_am);
  unsigned int i;

  debug_message_fn("(%d)\n", idx);

  if (idx == am->escape_symbol && am->update_escape_freq) {
    if (!am->update_escape_freq(_am, idx))
      return;
  } else {
    for (i = idx; i < am->nsymbols; i++)
      am->freq[i]++;
  }

  normalize_freq(_am);
}

/* Call with 0 <= low < high <= nsymbols - 1 && low <= idx <= high */
static int
encode_bulk(Arithmodel *_am, Index idx, Index low, Index high, int use_range)
{
  Arithmodel_order_zero *am = AMOZ(_am);

  if (am->nsymbols < idx)
    fatal("%s: nsymbols %d < %d idx, start = %d, low = %d, high = %d\n", __FUNCTION__, am->nsymbols, idx, am->start_symbol, low, high);

  if (am->nsymbols == idx) {
    if (use_range)
      fatal("%s: nsymbols %d == %d idx, when use_range is true.\n", __FUNCTION__, am->nsymbols, idx);
    if (IS_ESCAPE_INSTALLED(am)) {
      if (am->escape_encoded_with_rle) {
	am->escape_run++;
      } else {
	encode_bulk(_am, am->escape_symbol, low, high, use_range);
      }
      install_symbol(_am, 1);
      return 2;
    } else {
      fatal("%s: nsymbols %d == %d idx, but escape disabled\n", __FUNCTION__, am->nsymbols, idx);
    }
  }

  if (am->escape_encoded_with_rle && am->escape_run) {
    int run = am->escape_run;

    am->escape_run = 0;
    encode_bulk(_am, am->escape_symbol, low, high, use_range);
    arithmodel_encode_delta(am->bin_am, run, 0, 1);
  }

  update_region_with_range(_am, idx, low, high);
  arithcoder_encode_renormalize(am->ac);
  update_freq(_am, idx);

  return 1;
}

static int
encode(Arithmodel *_am, Index idx)
{
  return encode_bulk(_am, idx + AMOZ_START_SYMBOL(_am), 0, _am->nsymbols - 1, 0);
}

static int
encode_with_range(Arithmodel *_am, Index idx, Index low, Index high)
{
  /* Ensure the parameters are OK. */
  if (high >= arithmodel_order_zero_nsymbols(_am))
    return 0;
  if (idx < low && high < idx)
    return 0;

  /* No need to encode. */
  if (low == high)
    return 1;

  return encode_bulk(_am, idx + AMOZ_START_SYMBOL(_am), low + AMOZ_START_SYMBOL(_am), high + AMOZ_START_SYMBOL(_am), 1);
}

static int
encode_final(Arithmodel *_am)
{
  Arithmodel_order_zero *am = AMOZ(_am);

  if (IS_EOF_INSTALLED(am)) {
    encode_bulk(_am, am->eof_symbol, 0, _am->nsymbols - 1, 0);
    encode_bulk(_am, am->eof_symbol, 0, _am->nsymbols - 1, 0);
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

/* Call with 0 <= low < high <= nsymbols - 1 */
static int
decode_bulk(Arithmodel *_am, Index *index_return, Index low, Index high)
{
  Arithmodel_order_zero *am = AMOZ(_am);
  Arithcoder *ac = am->ac;
  Freq freq;
  Index left, right, idx;
  unsigned int tmp;

  tmp = ac->range / am->freq[high];
  freq = (ac->value - ac->low) / tmp;

  debug_message_fnc("low %X range %X target %X freq %X\n", ac->low, ac->range, ac->value, freq);

  if (am->nsymbols == 2) {
    idx =  (am->freq[0] > freq) ? 0 : 1;
  } else {
    left = low;
    right = high;
    while (right > left) {
      if (am->freq[(left + right) >> 1] <= freq)
	left = ((left + right) >> 1) + 1;
      else
	right = (left + right) >> 1;
    }
    idx = right;
    if (idx == low) {
      if (am->freq[low] <= freq)
	fatal("%s: Invalid selection of symbol: idx = %d, freq = %d, freq[%d] = %d\n", __FUNCTION__,
	      low, freq, low, am->freq[low]);
    } else {
      if (am->freq[idx - 1] > freq || am->freq[idx] <= freq)
	fatal("%s: Invalid selection of symbol: idx = %d, freq = %d, freq[idx - 1] = %d, freq[idx] = %d\n", __FUNCTION__, idx, freq, am->freq[idx - 1], am->freq[idx]);
    }
  }

  update_region_with_range(_am, idx, low, high);
  arithcoder_decode_renormalize(am->ac);

  if (IS_EOF_INSTALLED(am) && idx == am->eof_symbol)
    return 0;

  update_freq(_am, idx);

  if (IS_ESCAPE_INSTALLED(am) && idx == am->escape_symbol) {
    *index_return = am->nsymbols;
    install_symbol(_am, 1);
    return 2;
  }

  *index_return = idx;

  return 1;
}

static int
decode(Arithmodel *_am, Index *index_return)
{
  Index bulk;
  int result;

  if ((result = decode_bulk(_am, &bulk, 0, _am->nsymbols - 1)))
    *index_return = bulk - AMOZ_START_SYMBOL(_am);

  return result;
}

static int
decode_with_range(Arithmodel *_am, Index *index_return, Index low, Index high)
{
  Index bulk;
  int result;

  if (low > high)
    return 0;
  if (low == high) {
    *index_return = low;
    return 1;
  }

  if ((result = decode_bulk(_am, &bulk, low + AMOZ_START_SYMBOL(_am), high + AMOZ_START_SYMBOL(_am))))
    *index_return = bulk - AMOZ_START_SYMBOL(_am);

  return result;
}

static int
decode_final(Arithmodel *_am)
{
  Arithmodel_order_zero *am = AMOZ(_am);

  if (am->freq) {
    free(am->freq);
    am->freq = NULL;
  }

  return 1;
}

static void
destroy(Arithmodel *_am)
{
  Arithmodel_order_zero *am = AMOZ(_am);

  if (am->freq)
    free(am->freq);
  free(am);
}
