/*
 * expand.c
 * (C)Copyright 2001 by Hiroshi Takekawa
 * Last Modified: Tue Aug 28 16:07:48 2001.
 * $Id: expand.c,v 1.1 2001/08/29 08:37:25 sian Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "expand.h"
#include "vmpm_error.h"

void
expand(VMPM *vmpm)
{
  unsigned char *tmp;
  unsigned int i;
  int b;

  if ((tmp = malloc(vmpm->buffersize)) == NULL)
    memory_error(NULL, MEMORY_ERROR);
  memcpy(tmp, vmpm->buffer, vmpm->buffersize);
  free(vmpm->buffer);
  if ((vmpm->buffer = malloc(vmpm->buffersize << 3)) == NULL)
    memory_error(NULL, MEMORY_ERROR);
  /* Expand into bits. */
  for (i = 0; i < vmpm->buffersize; i++) {
    unsigned char c = tmp[i];
    for (b = 0; b < 8; b++) {
      vmpm->buffer[i * 8 + b] = (c & 0x80) ? 1 : 0;
      c <<= 1;
    }
  }
  free(tmp);
  vmpm->alphabetsize = 2;
  vmpm->bits_per_symbol = 1;
  vmpm->buffersize <<= 3;
}
