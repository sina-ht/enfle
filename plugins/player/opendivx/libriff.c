/*
 * libriff.c -- RIFF file read library
 * (C)Copyright 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Mar  6 12:14:08 2004.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include "libriff.h"

static int rc_destroy(RIFF_Chunk *);

static RIFF_Chunk rc_template = {
  .destroy = rc_destroy
};

static int set_func_input(RIFF_File *, RIFF_Input_func);
static int set_func_seek(RIFF_File *, RIFF_Seek_func);
static int set_func_arg(RIFF_File *, void *);
static int open_file(RIFF_File *);
static int iter_set(RIFF_File *, RIFF_Chunk *);
static int iter_next_chunk(RIFF_File *, RIFF_Chunk **);
static int iter_push(RIFF_File *);
static int iter_pop(RIFF_File *);
static int read_data(RIFF_File *, RIFF_Chunk *);
static const unsigned char *get_errmsg(RIFF_File *);
static int rf_destroy(RIFF_File *);

static RIFF_File rf_template = {
  .set_func_input = set_func_input,
  .set_func_seek = set_func_seek,
  .set_func_arg = set_func_arg,
  .open_file = open_file,
  .iter_set = iter_set,
  .iter_next_chunk = iter_next_chunk,
  .read_data = read_data,
  .iter_push = iter_push,
  .iter_pop = iter_pop,
  .get_errmsg = get_errmsg,
  .destroy = rf_destroy
};

RIFF_Chunk *
riff_chunk_create(void)
{
  RIFF_Chunk *rc;

  if ((rc = calloc(1, sizeof(RIFF_Chunk))) == NULL)
    return NULL;
  memcpy(rc, &rc_template, sizeof(RIFF_Chunk));

  return rc;
}

static int
rc_destroy(RIFF_Chunk *rc)
{
  if (rc->data) {
    free(rc->data);
    rc->data = NULL;
  }

  /* don't free rc ! rc will be freed in rf_destroy(). */

  return 1;
}

RIFF_File *
riff_file_create(void)
{
  RIFF_File *rf;

  if ((rf = calloc(1, sizeof(RIFF_File))) == NULL)
    return NULL;
  memcpy(rf, &rf_template, sizeof(RIFF_File));

  return rf;
}

static int
set_func_input(RIFF_File *rf, RIFF_Input_func rfi)
{
  rf->input_func = rfi;

  return 1;
}

static int
set_func_seek(RIFF_File *rf, RIFF_Seek_func rfs)
{
  rf->seek_func = rfs;

  return 1;
}

static int
set_func_arg(RIFF_File *rf, void *arg)
{
  rf->func_arg = arg;

  return 1;
}

/* internal */
static int
read_chunk(RIFF_File *rf, RIFF_Chunk *rc,
	   unsigned int base, unsigned int *chunk_read_return, unsigned int *chunk_size_return)
{
  unsigned char buffer[8];
  unsigned int chunk_read;
  unsigned int seek_len;

  chunk_read = 0;
  if (rf->input_func(rf->func_arg, buffer, 8) != 8) {
    rf->err = _RIFF_ERR_PREMATURE_CHUNK;
    return 0;
  }
  chunk_read += 8;
  memcpy(rc->name, buffer, 4);
  rc->name[4] = '\0';
  rc->size = (((((buffer[7] << 8) | buffer[6]) << 8) | buffer[5]) << 8) | buffer[4];
  seek_len = (rc->size + 1) & ~1;
  rc->pos = base + chunk_read;

  /* fprintf(stderr, "got chunk %s %d at %d\n", rc->name, rc->size, rc->pos); */

  if (memcmp(rc->name, "LIST", 4)) {
    rc->is_list = 0;
    if (!rf->seek_func(rf->func_arg, seek_len, RIFF_SEEK_CUR)) {
      fprintf(stderr, __FUNCTION__ ": %d\n", rc->size);
      perror("read_chunks: seek");
      rf->err = _RIFF_ERR_TRUNCATED_CHUNK;
      return 0;
    }
    chunk_read += seek_len;
  } else {
    rc->is_list = 1;
    if (rf->input_func(rf->func_arg, buffer, 4) != 4) {
      rf->err = _RIFF_ERR_PREMATURE_CHUNK;
      return 0;
    }
    chunk_read += 4;
    memcpy(rc->list_name, buffer, 4);
    rc->list_name[4] = '\0';
  }

  *chunk_read_return = chunk_read;
  *chunk_size_return = seek_len;

  return 1;
}

static int
read_chunks(RIFF_File *rf, RIFF_Chunk *rc,
	    unsigned int size, unsigned int base, unsigned int *chunks_read_return)
{
  unsigned int chunks_read, chunk_read, chunk_size, list_read;

  chunks_read = 0;
  while (chunks_read < size) {
    if (!read_chunk(rf, rc, base + chunks_read, &chunk_read, &chunk_size))
      return 0;
    chunks_read += chunk_read;
    if (rc->is_list) {
      if ((rc->list = riff_chunk_create()) == NULL) {
	rf->err = _RIFF_ERR_NO_ENOUGH_MEMORY;
	return 0;
      }
      if (!read_chunks(rf, rc->list, chunk_size - 4, base + chunks_read, &list_read))
	return 0;
      chunks_read += list_read;
    } else {
      rc->list = NULL;
    }
    if ((rc->next = riff_chunk_create()) == NULL) {
      rf->err = _RIFF_ERR_NO_ENOUGH_MEMORY;
      return 0;
    }
    rc->next->prev = rc;
    rc = rc->next;
  }

  rc->prev->next = NULL;
  /* this is correct. not to use _destroy() */
  free(rc);

  *chunks_read_return = chunks_read;

  return 1;
}

static int
open_file(RIFF_File *rf)
{
  unsigned char buffer[12];
  unsigned int chunks_read;

  if (rf->input_func(rf->func_arg, buffer, 12) != 12) {
    rf->err = _RIFF_ERR_TOO_SMALL;
    return 0;
  }

  if (memcmp(buffer, "RIFF", 4)) {
    rf->err = _RIFF_ERR_NOT_RIFF;
    return 0;
  }
  rf->size = (((((buffer[7] << 8) | buffer[6]) << 8) | buffer[5]) << 8) | buffer[4];
  memcpy(rf->form, buffer + 8, 4);
  rf->form[4] = '\0';

  if ((rf->chunks = riff_chunk_create()) == NULL) {
    rf->err = _RIFF_ERR_NO_ENOUGH_MEMORY;
    return 0;
  }

  if (!read_chunks(rf, rf->chunks, rf->size - 4, 12, &chunks_read))
    return 0;

  return 1;
}

static int
iter_set(RIFF_File *rf, RIFF_Chunk *rc)
{
  rf->iter_head = rc;

  return 1;
}

static int
iter_next_chunk(RIFF_File *rf, RIFF_Chunk **chunk_return)
{
  RIFF_Chunk *rc;

  rc = rf->iter_current = rf->iter_head;
  if (rc)
    rf->iter_head = rc->next;

  *chunk_return = rc;

  return 1;
}

static int
iter_push(RIFF_File *rf)
{
  if (!rf->iter_current->list) {
    rf->err = _RIFF_ERR_NO_LIST;
    return 0;
  }

  if (!rf->stack) {
    if ((rf->stack = calloc(1, sizeof(_RIFF_iter_stack))) == NULL) {
      rf->err = _RIFF_ERR_NO_ENOUGH_MEMORY;
      return 0;
    }
    rf->stack->prev = NULL;
  } else {
    if ((rf->stack->next = calloc(1, sizeof(_RIFF_iter_stack))) == NULL) {
      rf->err = _RIFF_ERR_NO_ENOUGH_MEMORY;
      return 0;
    }
    rf->stack->next->prev = rf->stack;
    rf->stack = rf->stack->next;
  }

  rf->stack->rc = rf->iter_head;
  rf->iter_head = rf->iter_current->list;

  return 1;
}

static int
iter_pop(RIFF_File *rf)
{
  if (!rf->stack)
    return 0;
  rf->iter_head = rf->stack->rc;
  if (rf->stack->prev) {
    rf->stack = rf->stack->prev;
    free(rf->stack->next);
  } else {
    free(rf->stack);
    rf->stack = NULL;
  }

  return 1;
}

static int
read_data(RIFF_File *rf, RIFF_Chunk *rc)
{
  if (!rf->seek_func(rf->func_arg, rc->pos, RIFF_SEEK_SET)) {
    rf->err = _RIFF_ERR_SEEK_FAILED;
    return 0;
  }
  if ((rc->data = malloc(rc->size)) == NULL) {
    rf->err = _RIFF_ERR_NO_ENOUGH_MEMORY;
    fprintf(stderr, "[%s] %d bytes\n", rc->name, rc->size);
    return 0;
  }
  if (rf->input_func(rf->func_arg, rc->data, rc->size) != rc->size) {
    rf->err = _RIFF_ERR_TRUNCATED_CHUNK;
    return 0;
  }

  return 1;
}

static const unsigned char *
get_errmsg(RIFF_File *rf)
{
  static const unsigned char *errmsg[] = {
    "Success.",
    "File size too small.",
    "RIFF tag not found.",
    "No enough memory.",
    "Premature chunk.",
    "Truncated chunk.",
    "Seek failed.",
    "No list but pushed"
  };

  if (rf->err < _RIFF_ERR_SUCCESS || rf->err >= _RIFF_ERR_MAXCODE)
    return "Unknown error.";
  return errmsg[rf->err];
}

static int
rf_destroy(RIFF_File *rf)
{
  RIFF_Chunk *rc, *rc_next;

  while (iter_pop(rf)) ;

  for (rc = rf->chunks; rc != NULL;) {
    rc_next = rc->next;
    if (rc->data)
      free(rc->data);
    free(rc);
    rc = rc_next;
  }
  free(rf);

  return 1;
}
