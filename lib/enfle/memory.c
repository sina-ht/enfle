/*
 * memory.c -- memory object
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Jun 19 01:33:26 2001.
 * $Id: memory.c,v 1.6 2001/06/19 08:16:19 sian Exp $
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

#include <stdlib.h>

#define REQUIRE_UNISTD_H
#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "memory.h"

static unsigned char *alloc_normal(Memory *, unsigned int);
static unsigned char *alloc_shm(Memory *, unsigned int);

static MemoryType request_type(Memory *, MemoryType);
static unsigned char *allocate(Memory *, unsigned int);
static int set(Memory *, void *, MemoryType, unsigned int, unsigned int);
static int free_both(Memory *);
static Memory *duplicate(Memory *, int);
static void destroy(Memory *);

static Memory template = {
  ptr: NULL,
  size: 0,
  used: 0,
  type: _UNKNOWN,
  shmid: -1,
  request_type: request_type,
  allocate: allocate,
  set: set,
  free_both: free_both,
  duplicate: duplicate,
  destroy: destroy
};

Memory *
memory_create(void)
{
  Memory *mem;

  if ((mem = calloc(1, sizeof(Memory))) == NULL)
    return NULL;
  memcpy(mem, &template, sizeof(Memory));
  request_type(mem, _NORMAL);

  return mem;
}

/* for internal use */

static unsigned char *
alloc_normal(Memory *mem, unsigned int s)
{
  unsigned char *tmp;

  if ((tmp = realloc(mem->ptr, s)) == NULL)
    return NULL;

  mem->ptr = tmp;
  mem->size = mem->used = s;
  mem->type = _NORMAL;

  return mem->ptr;
}

static unsigned char *
alloc_shm(Memory *mem, unsigned int s)
{
#ifdef USE_SHM
  /* debug_message("%p mem->size %d <=> %d = s\n", mem, mem->size,s); */

  free_both(mem);

  if ((mem->shmid = shmget(IPC_PRIVATE, s, (IPC_CREAT | 0600))) < 0) {
    show_message(__FUNCTION__ ": shmget failed.\n");
    return NULL;
  }

  debug_message("MEMORY: " __FUNCTION__ ": shmget: id = %d\n", mem->shmid);

  if ((mem->ptr = shmat(mem->shmid, 0, 0)) == (void *)-1) {
    show_message(__FUNCTION__ ": shmat failed.\n");
    mem->ptr = NULL;
    return NULL;
  }

  debug_message("MEMORY: " __FUNCTION__ ": shmat: addr = %p\n", mem->ptr);

  shmctl(mem->shmid, IPC_RMID, NULL);

  mem->size = mem->used = s;
  mem->type = _SHM;

  return mem->ptr;
#else
  show_message("No SHM support.\n");
  return NULL;
#endif
}

/* methods */

static MemoryType
request_type(Memory *mem, MemoryType type)
{
  switch (type) {
  case _NORMAL:
  case _SHM:
#ifdef USE_SHM
    mem->type = type;
#else
    mem->type = _NORMAL;
#endif
    break;
  default:
    mem->type = _UNKNOWN;
    break;
  }
  return mem->type;
}

static unsigned char *
allocate(Memory *mem, unsigned int size)
{
  unsigned int aligned_size;
  unsigned int alignment;

  if (mem->size >= size) {
    mem->used = size;
    return mem->ptr;
  }

  alignment = getpagesize();
  aligned_size = (size % alignment) ? ((size / alignment) + 1) * alignment : size;

  //debug_message("MEMORY: " __FUNCTION__ ": requested %d bytes (aligned %d bytes) as type %s\n", size, aligned_size, mem->type == _NORMAL ? "NORMAL" : (mem->type == _SHM ? "SHM" : "UNKNOWN"));

  switch (mem->type) {
  case _NORMAL:
    return alloc_normal(mem, aligned_size);
  case _SHM:
    return alloc_shm(mem, aligned_size);
  default:
    break;
  }
  return NULL;
}

static int
set(Memory *mem, void *ptr, MemoryType type, unsigned int size, unsigned int used)
{
  mem->ptr = ptr;
  mem->type = type;
  mem->size = size;
  mem->used = used;

  return 1;
}

static int
free_both(Memory *mem)
{
  /* debug_message(__FUNCTION__ ": free %p\n", mem); */

  switch (mem->type) {
  case _NORMAL:
    if (mem->ptr)
      free(mem->ptr);
    break;
#ifdef USE_SHM
  case _SHM:
    if (mem->ptr)
      shmdt(mem->ptr);
    break;
#endif
  case _UNKNOWN:
    break;
  default:
    return 0;
  }

  mem->ptr = NULL;
  mem->size = mem->used = 0;
  mem->type = _UNKNOWN;

  return 1;
}

static Memory *
duplicate(Memory *mem, int as_shm)
{
  Memory *new;

  if ((new = memory_create()) == NULL)
    return NULL;

  switch (mem->type) {
  case _NORMAL:
    if (alloc_normal(new, mem->used) == NULL) {
      free(new);
      return NULL;
    }
    memcpy(new->ptr, mem->ptr, mem->used);
    break;
  case _SHM:
    if (as_shm) {
      if (alloc_shm(new, mem->used) == NULL) {
	free(new);
	return NULL;
      }
    } else {
      if (alloc_normal(new, mem->used) == NULL) {
	free(new);
	return NULL;
      }
    }
    memcpy(new->ptr, mem->ptr, mem->used);
    break;
  case _UNKNOWN:
    break;
  default:
    free(new);
    return NULL;
  }

  return new;
}

static void
destroy(Memory *mem)
{
  free_both(mem);
  free(mem);
}
