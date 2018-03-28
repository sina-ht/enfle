/*
 * mm.h
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Wed Oct 10 20:52:52 2001.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

/* slightly improved memory manager... */
#define USE_MEMORY_PROTECTION

#ifndef _MM_H
#define _MM_H

#include <stdlib.h>

#ifdef USE_MEMORY_PROTECTION

typedef struct _mm_chunk MM_Chunk;
struct _mm_chunk {
  void *p;
  MM_Chunk *next, *prev;
};

void *w32api_mem_realloc(void *, int);
void *w32api_mem_alloc(int);
void w32api_mem_free(void *);

#else /* USE_MEMORY_PROTECTION */

#ifdef W32API_REQUEST_MEM_ALLOC
static inline void *
w32api_mem_alloc(int size)
{
  return (void *)malloc(size);
}
#endif

#ifdef W32API_REQUEST_MEM_REALLOC
static inline void *
w32api_mem_realloc(void *p, int size)
{
  return (void *)realloc(p, size);
}
#endif

#ifdef W32API_REQUEST_MEM_FREE
static inline void
w32api_mem_free(void *p)
{
  if (p)
    free(p);
}
#endif

#endif /* USE_MEMORY_PROTECTION */

#endif
