/*
 * expand.c
 * (C)Copyright 2001 by Hiroshi Takekawa
 * Last Modified: Mon Sep 24 02:32:44 2001.
 * $Id: expand.c,v 1.5 2001/09/23 17:36:57 sian Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#define REQUIRE_STRING_H
#include "compat.h"

#include "expand.h"
#include "vmpm_error.h"

int
expand(VMPM *vmpm)
{
  unsigned int i;
  int b, expand_factor;

  if (vmpm->r == 2 || vmpm->r == 4) {
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
      expand_factor = 2;
      break;
    case 7:
      expand_factor = 1;
      break;
    case 8:
      expand_factor = 0;
      break;
    default:
      show_message("Invalid nlowbits = %d\n", vmpm->nlowbits);
      return 0;
    }
  } else {
    if (vmpm->nlowbits >= 0 && vmpm->nlowbits <= 7)
      expand_factor = 8;
    else if (vmpm->nlowbits == 8)
      expand_factor = 0;
    else {
      show_message("Invalid nlowbits = %d\n", vmpm->nlowbits);
      return 0;
    }
  }

  if (expand_factor > 0) {
    unsigned char *tmp;

    if ((tmp = malloc(vmpm->bufferused)) == NULL)
      memory_error(NULL, MEMORY_ERROR);
    memcpy(tmp, vmpm->buffer, vmpm->bufferused);
    free(vmpm->buffer);

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
  } else {
    vmpm->bufferused = 0;
  }

  return 1;
}
