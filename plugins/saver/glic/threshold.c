/*
 * threshold.c
 * (C)Copyright 2001 by Hiroshi Takekawa
 * Last Modified: Mon Sep 17 22:43:59 2001.
 */

#include <stdlib.h>

#include "common.h"

#include "threshold.h"

#define LOW_MASK(n) ((1U << n) - 1)

int
threshold(VMPM *vmpm)
{
  unsigned int i;

  if (vmpm->buffer_high)
    free(vmpm->buffer_high);
  if ((vmpm->buffer_high = malloc(vmpm->bufferused)) == NULL)
    return 0;

  if (vmpm->buffer_low)
    free(vmpm->buffer_low);
  if ((vmpm->buffer_low = malloc(vmpm->bufferused)) == NULL)
    return 0;

  vmpm->bits_per_symbol = 8 - vmpm->nlowbits;
  vmpm->alphabetsize = 1 << vmpm->bits_per_symbol;
  vmpm->buffer_low_size = vmpm->bufferused;

  for (i = 0; i < vmpm->bufferused; i++) {
    vmpm->buffer_low[i] = vmpm->buffer[i] & LOW_MASK(vmpm->nlowbits);
    vmpm->buffer[i] >>= vmpm->nlowbits;
    vmpm->buffer_high[i] = vmpm->buffer[i];
  }

  return 1;
}
