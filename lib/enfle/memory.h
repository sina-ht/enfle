/*
 * memory.h -- memory object header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Dec 26 00:07:12 2003.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _MEMORY_H
#define _MEMORY_H

#ifdef USE_SHM
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#endif

typedef enum _memorytype {
  _UNKNOWN,
  _NORMAL,
  _SHM
} MemoryType;

typedef struct _memory Memory;
struct _memory {
  void *ptr;
  unsigned int size;
  unsigned int used;
  MemoryType type;
  int shmid;

  MemoryType (*request_type)(Memory *, MemoryType);
  void *(*allocate)(Memory *, unsigned int);
  int (*set)(Memory *, void *, MemoryType, unsigned int, unsigned int);
  int (*free_both)(Memory *);
  Memory *(*duplicate)(Memory *, int);
  void (*destroy)(Memory *);
};

#define memory_request_type(p, t) (p)->request_type((p), (t))
#define memory_alloc(p, s) (p)->allocate((p), (s))
#define memory_set(p, pt, t, s, u) (p)->set((p), (pt), (t), (s), (u))
#define memory_free(p) (p)->free_both((p))
#define memory_dup(p) (p)->duplicate((p), 0)
#define memory_dup_as_shm(p) (p)->duplicate((p), 1)
#define memory_destroy(p) (p)->destroy((p))

#define memory_ptr(p) (p)->ptr
#define memory_size(p) (p)->size
#define memory_used(p) (p)->used
#define memory_type(p) (p)->type
#define memory_shmid(p) (((p)->type == _SHM) ? (p)->shmid : 0)

Memory *memory_create(void);

#endif
