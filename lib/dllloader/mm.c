/*
 * mm.c -- Memory manager
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Wed Apr 14 22:03:00 2004.
 * $Id: mm.c,v 1.4 2004/04/18 06:24:22 sian Exp $
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

#include "compat.h"
#include "common.h"
#include "mm.h"

#ifdef USE_MEMORY_PROTECTION

static MM_Chunk *top_chunk = NULL, *head_chunk = NULL;

static MM_Chunk *
mc_find(void *p)
{
  MM_Chunk *mc;

  for (mc = top_chunk; mc; mc = mc->next) {
    if (mc->p == p)
      return mc;
  }

  return NULL;
}

void *
w32api_mem_alloc(int size)
{
  MM_Chunk *mc;
  void *p;

  if ((p = calloc(1, size)) == NULL)
    return NULL;

  if ((mc = calloc(1, sizeof(MM_Chunk))) == NULL) {
    free(p);
    return NULL;
  }
  mc->p = p;

  if (top_chunk == NULL) {
    head_chunk = top_chunk = mc;
  } else {
    head_chunk->next = mc;
    mc->prev = head_chunk;
    head_chunk = mc;
  }

  return p;
}

void *
w32api_mem_realloc(void *p, int size)
{
  MM_Chunk *mc;
  void *new_p;

  if (p == NULL)
    return w32api_mem_alloc(size);

  if ((mc = mc_find(p)) == NULL) {
    show_message("%s: No such chunk %p\n", __FUNCTION__, p);
    return NULL;
  }

  if ((new_p = realloc(p, size)) == NULL)
    return NULL;
  mc->p = new_p;

  return new_p;
}

void
w32api_mem_free(void *p)
{
  MM_Chunk *mc;

  if (p == NULL)
    return;

  if ((mc = mc_find(p)) == NULL) {
    show_message("%s: No such chunk %p\n", __FUNCTION__, p);
    return;
  }

  if (mc->prev)
    mc->prev->next = mc->next;
  if (mc->next)
    mc->next->prev = mc->prev;
  if (mc == top_chunk)
    top_chunk = mc->next;
  if (mc == head_chunk)
    head_chunk = mc->prev;
  free(mc);

  free(p);
}

#endif
