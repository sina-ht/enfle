/*
 * threshold.c
 * (C)Copyright 2001 by Hiroshi Takekawa
 * Last Modified: Thu Sep  6 12:31:42 2001.
 * $Id: threshold.c,v 1.1 2001/09/07 04:42:39 sian Exp $
 */

#include <stdlib.h>

#include "common.h"

#include "threshold.h"

#define LOW_MASK(n) ((1 << n) - 1)

int
threshold(VMPM *vmpm)
{
  int i;

  debug_message(__FUNCTION__ ": nlowbits = %d\n", vmpm->nlowbits);

  if (vmpm->buffer_low)
    free(vmpm->buffer_low);
  if ((vmpm->buffer_low = malloc(vmpm->bufferused)) == NULL)
    return 0;

  vmpm->bits_per_symbol = (8 - vmpm->nlowbits);
  vmpm->alphabetsize = 1 << vmpm->bits_per_symbol;

  for (i = 0; i < vmpm->bufferused; i++) {
    unsigned char c;

    c = vmpm->buffer[i];
    vmpm->buffer_low[i] = c & LOW_MASK(vmpm->nlowbits);
    vmpm->buffer[i] = c >> vmpm->nlowbits;
  }

  return 1;
}
