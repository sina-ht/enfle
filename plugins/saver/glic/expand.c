/*
 * expand.c
 * (C)Copyright 2001 by Hiroshi Takekawa
 * Last Modified: Sun Sep  9 11:19:27 2001.
 * $Id: expand.c,v 1.3 2001/09/10 00:00:05 sian Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "common.h"

#include "expand.h"
#include "vmpm_error.h"

int
expand(VMPM *vmpm)
{
  unsigned char *tmp;
  unsigned int i;
  int b, expand_factor;

  if ((tmp = malloc(vmpm->bufferused)) == NULL)
    memory_error(NULL, MEMORY_ERROR);
  memcpy(tmp, vmpm->buffer, vmpm->bufferused);
  free(vmpm->buffer);

  if (vmpm->r == 2) {
    switch (vmpm->nlowbits) {
    case 0:
    case 1:
    case 2:
    case 3:
      expand_factor = 8;
      break;
    case 4:
    case 5:
      expand_factor = 4;
      break;
    case 6:
    case 7:
      expand_factor = 2;
      break;
    case 8:
      expand_factor = 1;
      break;
    default:
      show_message("Invalid nlowbits = %d\n", vmpm->nlowbits);
      return 0;
    }
  } else {
    expand_factor = 8;
  }

  if ((vmpm->buffer = malloc(vmpm->bufferused * expand_factor)) == NULL)
    memory_error(NULL, MEMORY_ERROR);

  /* Expand into bits. */
  for (i = 0; i < vmpm->bufferused; i++) {
    unsigned char c = tmp[i];
    for (b = 0; b < expand_factor; b++) {
      vmpm->buffer[i * expand_factor + b] = (c & (1 << (expand_factor - 1))) ? 1 : 0;
      c <<= 1;
    }
  }
  free(tmp);

  vmpm->alphabetsize = 2;
  vmpm->bits_per_symbol = 1;
  vmpm->bufferused *= expand_factor;
  vmpm->buffersize = vmpm->bufferused;

  return 1;
}
