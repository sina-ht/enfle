/*
 * vmpm.h -- Implementation of a variation of MPM header
 * (C)Copyright 2001 by Hiroshi Takekawa
 * Last Modified: Sun Sep  9 11:24:01 2001.
 * $Id: vmpm.h,v 1.7 2001/09/10 00:04:51 sian Exp $
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
  int nlowbits;
  int blocksize;
  int bitwise;
  int is_stat;
  unsigned int alphabetsize;
  int bits_per_symbol;
  char *infilepath;
  char *outfilepath;
  FILE *infile;
  FILE *outfile;
  FILE *statfile;
  unsigned char *buffer;
  unsigned char *buffer_high;
  unsigned char *buffer_low;
  unsigned int buffersize;
  unsigned int bufferused;
  unsigned int buffer_low_size;
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

#define stat_message(v, format, args...) if ((v)->is_stat) fprintf((v)->statfile, format, ## args)

#endif
