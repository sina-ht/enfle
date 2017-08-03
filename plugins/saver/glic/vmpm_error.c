/*
 * vmpm_error.c
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * Last Modified: Tue Jun 19 02:02:09 2001.
 */

#include <stdio.h>
#include <stdlib.h>

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
