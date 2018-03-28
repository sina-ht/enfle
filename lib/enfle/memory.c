/*
 * memory.c -- memory object
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Jul  1 01:43:23 2005.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include <stdlib.h>

#define REQUIRE_UNISTD_H
#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "memory.h"

static void *alloc_normal(Memory *, unsigned int);
static void *alloc_shm(Memory *, unsigned int);

static MemoryType request_type(Memory *, MemoryType);
static void *allocate(Memory *, unsigned int);
static int set(Memory *, void *, MemoryType, unsigned int, unsigned int);
static int free_both(Memory *);
static Memory *duplicate(Memory *, int);
static void destroy(Memory *);

static Memory template = {
  .ptr = NULL,
  .type = _UNKNOWN,
  .shmid = -1,

  .request_type = request_type,
  .allocate = allocate,
  .set = set,
  .free_both = free_both,
  .duplicate = duplicate,
  .destroy = destroy
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

static void *
alloc_normal(Memory *mem, unsigned int s)
{
  void *tmp;

  free_both(mem);

  if ((tmp = memalign(ATTRIBUTE_ALIGNED_MAX, s)) == NULL)
    return NULL;

  mem->ptr = tmp;
  mem->size = mem->used = s;
  mem->type = _NORMAL;

  return mem->ptr;
}

static void *
alloc_shm(Memory *mem, unsigned int s)
{
#ifdef USE_SHM
  /* debug_message("%p mem->size %d <=> %d = s\n", mem, mem->size,s); */

  free_both(mem);

  if ((mem->shmid = shmget(IPC_PRIVATE, s, (IPC_CREAT | 0600))) < 0) {
    show_message_fnc("shmget failed.\n");
    return NULL;
  }

  debug_message_fnc("shmget: id = %d\n", mem->shmid);

  if ((mem->ptr = shmat(mem->shmid, 0, 0)) == (void *)-1) {
    show_message_fnc("shmat failed.\n");
    mem->ptr = NULL;
    return NULL;
  }

  debug_message_fnc("shmat: addr = %p\n", mem->ptr);

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

static void *
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

  //debug_message_fnc("requested %d bytes (aligned %d bytes) as type %s\n", size, aligned_size, mem->type == _NORMAL ? "NORMAL" : (mem->type == _SHM ? "SHM" : "UNKNOWN"));

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
  /* debug_message_fnc("mem = %p\n", mem); */

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
