/*
 * vmpm_error.c
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * Last Modified: Fri Apr 20 18:42:40 2001.
 * $Id: vmpm_error.c,v 1.2 2001/04/21 07:28:07 sian Exp $
 */

#include <stdio.h>

#include "vmpm_error.h"

void
generic_error(char *message, int code)
{
  if (message)
    fprintf(stderr, "%s", message);
  else
    fprintf(stderr, "Error occurred.\n");

  exit(code);
}

void
memory_error(char *message, int code)
{
  if (message)
    fprintf(stderr, "%s", message);
  else
    fprintf(stderr, "No enough memory.\n");

  exit(code);
}

void
open_error(char *filename, int code)
{
  fprintf(stderr, "Cannot open %s", filename);
  perror("");
  exit(code);
}
