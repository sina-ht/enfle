/*
 * mm.h
 */

/* pretty memory manager :-) */

#ifndef _MM_H
#define _MM_H

#include <stdlib.h>

#ifdef W32API_REQUEST_MEM_ALLOC
static inline
void *w32api_mem_alloc(int size)
{
  return (void *)malloc(size);
}
#endif

#ifdef W32API_REQUEST_MEM_REALLOC
static inline
void *w32api_mem_realloc(void *p, int size)
{
  return (void *)realloc(p, size);
}
#endif

#ifdef W32API_REQUEST_MEM_FREE
static inline
void w32api_mem_free(void *p)
{
  if (p)
    free(p);
}
#endif

#endif
