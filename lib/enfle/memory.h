/*
 * memory.h -- memory object header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Thu Jan  4 06:56:47 2001.
 * $Id: memory.h,v 1.3 2001/01/06 23:53:32 sian Exp $
 *
 * Enfle is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Enfle is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
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
  unsigned char *(*allocate)(Memory *, unsigned int);
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
