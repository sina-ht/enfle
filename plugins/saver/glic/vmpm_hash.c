/*
 * vmpm_hash.c -- Hash related routines
 * (C)Copyright 2001 by Hiroshi Takekawa
 * Last Modified: Tue Jun 19 02:03:08 2001.
 * $Id: vmpm_hash.c,v 1.3 2001/06/19 08:16:19 sian Exp $
 */

#include <stdio.h>

#define REQUIRE_STRING_H
#include "compat.h"

#include "vmpm.h"
#include "vmpm_hash.h"
#include "vmpm_error.h"

static unsigned int
hash_function(void *key, unsigned int len)
{
  unsigned char c, *k;
  unsigned int h = 0, l;

  k = key;
  l = len;
  while (len--) {
    c = *k++;
    h += c + (c << 17);
    h ^= (h >> 2);
  }
  h += l + (l << 17);
  h ^= (h >> 2);

  return h;
}

static unsigned int
hash_function2(void *key, unsigned int len)
{
  unsigned char c, *k;
  unsigned int h = 0, l;

  k = key;
  l = len;
  while (len--) {
    c = *k++;
    h += c + (c << 13);
    h ^= (h >> 3);
  }
  h += l + (l << 13);
  h ^= (h >> 3);

  return 17 - (h % 17);
}

int
register_to_token_hash(VMPM *vmpm, int offset, unsigned int length, int flag, Token_value nthtoken, Token **t_return)
{
  int i;
  void *k;
  Hash_value hash, skip;

  k = vmpm->buffer + offset;
  hash = hash_function(k, length) % HASH_SIZE;
  skip = hash_function2(k, length);

  i = 0;
  while (vmpm->token_hash[hash].length) {
    unsigned int o = vmpm->token_hash[hash].offset;
    unsigned int l = vmpm->token_hash[hash].length;

    if (l == length && memcmp(vmpm->buffer + o, k, l) == 0) {
      *t_return = &vmpm->token_hash[hash];
      return 1;
    }

    hash = (hash + skip) % HASH_SIZE;
    i++;
    if (i > 100)
      generic_error((char *)"Hash functions are not good\n", HASH_ERROR);
  }

  /* not found, register it */
  vmpm->token_hash[hash].value  = nthtoken;
  vmpm->token_hash[hash].offset = offset;
  vmpm->token_hash[hash].length = length;
  vmpm->token_hash[hash].flag   = flag;

  *t_return = &vmpm->token_hash[hash];
  return 0;
}
