/*
 * expand.c
 * (C)Copyright 2001 by Hiroshi Takekawa
 * Last Modified: Thu Sep  6 12:00:38 2001.
 * $Id: expand.c,v 1.2 2001/09/07 04:43:26 sian Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "common.h"

#include "expand.h"
#include "vmpm_error.h"

void
expand(VMPM *vmpm)
{
  unsigned char *tmp;
  unsigned int i;
  int b;

  debug_message(__FUNCTION__ "() called\n");

  if ((tmp = malloc(vmpm->bufferused)) == NULL)
    memory_error(NULL, MEMORY_ERROR);
  memcpy(tmp, vmpm->buffer, vmpm->bufferused);
  free(vmpm->buffer);
  if ((vmpm->buffer = malloc(vmpm->bufferused << 3)) == NULL)
    memory_error(NULL, MEMORY_ERROR);
  /* Expand into bits. */
  for (i = 0; i < vmpm->bufferused; i++) {
    unsigned char c = tmp[i];
    for (b = 0; b < 8; b++) {
      vmpm->buffer[i * 8 + b] = (c & 0x80) ? 1 : 0;
      c <<= 1;
    }
  }
  free(tmp);

  vmpm->alphabetsize = 2;
  vmpm->bits_per_symbol = 1;
  vmpm->bufferused <<= 3;
  vmpm->buffersize = vmpm->bufferused;
}
