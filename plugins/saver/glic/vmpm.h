/*
 * vmpm.h -- Implementation of a variation of MPM header
 * (C)Copyright 2001 by Hiroshi Takekawa
 * Last Modified: Fri Apr 20 18:42:09 2001.
 * $Id: vmpm.h,v 1.2 2001/04/21 07:28:07 sian Exp $
 */

#ifndef _VMPM_H
#define _VMPM_H

typedef struct _vmpm VMPM;

#include "vmpm_decompose.h"

typedef unsigned int Token_value;

typedef struct _list_item {
  Token_value value;
  unsigned int offset;
  unsigned int length;
  unsigned int newindex;
  int flag;
} Token;

struct _vmpm {
  int r, I;
  int blocksize;
  int is_stat;
  unsigned int alphabetsize;
  int bits_per_symbol;
  char *infilepath;
  char *outfilepath;
  FILE *infile;
  FILE *outfile;
  unsigned char *buffer;
  unsigned int buffersize;
  unsigned int bufferused;
  Token *token_hash;
  Token **tokens;
  Token ***token; /* pointers to tokens in token_hash */
  Token_value *newtoken;
  unsigned int *token_index;
  void *method_private;
  VMPMDecomposer *decomposer_top;
  VMPMDecomposer *decomposer_head;
  VMPMDecomposer *decomposer;
};

#define stat_message(v, format, args...) if ((v)->is_stat) fprintf(stderr, format, ## args)

#endif
