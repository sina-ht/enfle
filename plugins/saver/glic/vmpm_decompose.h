/*
 * vmpm_decompose.h -- Decomposition header
 * (C)Copyright 2001 by Hiroshi Takekawa
 * Last Modified: Fri Apr 20 18:42:21 2001.
 * $Id: vmpm_decompose.h,v 1.2 2001/04/21 07:28:07 sian Exp $
 */

#ifndef _VMPM_DECOMPOSE_H
#define _VMPM_DECOMPOSE_H

typedef struct _vmpmdecomposer VMPMDecomposer;
typedef VMPMDecomposer *(*VMPMDecomposer_init)(VMPM *);

struct _vmpmdecomposer {
  const char *name;
  const unsigned char *description;
  void (*init)(VMPM *);
  int (*decompose)(VMPM *, int, int, int);
  void (*encode)(VMPM *);
  void (*decode)(VMPM *);
  int (*reconstruct)(VMPM *);
  void (*final)(VMPM *);
  void (*destroy)(VMPM *);
  VMPMDecomposer *next;
};

int decomposer_scan_and_load(VMPM *, char *);
VMPMDecomposer *decomposer_search(VMPM *, char *);
void decomposer_list(VMPM *, FILE *);
void decomposer_list_detail(VMPM *, FILE *);

#endif
