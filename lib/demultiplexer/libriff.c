/*
 * libriff.c -- RIFF file read library
 * (C)Copyright 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Sep  3 01:28:39 2001.
 * $Id: libriff.c,v 1.1 2001/09/03 00:31:03 sian Exp $
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
#include "libriff.h"

#include "common.h"

static int rc_destroy(RIFF_Chunk *);

static RIFF_Chunk rc_template = {
  destroy: rc_destroy
};

static int open_file(RIFF_File *);
static int read_chunk_header(RIFF_File *, RIFF_Chunk *);
static int skip_chunk_data(RIFF_File *, RIFF_Chunk *);
static int read_data(RIFF_File *, RIFF_Chunk *);
static int read_data_with_seek(RIFF_File *, RIFF_Chunk *);
static int read_chunk_headers(RIFF_File *, RIFF_Chunk *, unsigned int);
static int read_all_chunk_headers(RIFF_File *);
static int iter_set(RIFF_File *, RIFF_Chunk *);
static int iter_next_chunk(RIFF_File *, RIFF_Chunk **);
static int iter_push(RIFF_File *);
static int iter_pop(RIFF_File *);
static const unsigned char *get_errmsg(RIFF_File *);
static int rf_destroy(RIFF_File *);

static RIFF_File rf_template = {
  open_file: open_file,
  read_chunk_header: read_chunk_header,
  skip_chunk_data: skip_chunk_data,
  read_data: read_data,
  read_data_with_seek: read_data_with_seek,
  read_chunk_headers: read_chunk_headers,
  read_all_chunk_headers: read_all_chunk_headers,
  iter_set: iter_set,
  iter_next_chunk: iter_next_chunk,
  iter_push: iter_push,
  iter_pop: iter_pop,
  get_errmsg: get_errmsg,
  destroy: rf_destroy
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
open_file(RIFF_File *rf)
{
  unsigned char buffer[12];

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
  return 1;
}

static int
read_chunk_header(RIFF_File *rf, RIFF_Chunk *rc)
{
  unsigned char buffer[8];
  unsigned int chunk_read;

  if ((chunk_read = rf->input_func(rf->func_arg, buffer, 8)) != 8) {
    if (chunk_read == 0) {
      /* Reach end of file. */
      rf->err = _RIFF_ERR_SUCCESS;
      return 0;
    }
    rf->err = _RIFF_ERR_PREMATURE_CHUNK;
    return 0;
  }
  memcpy(rc->name, buffer, 4);
  rc->name[4] = '\0';
  rc->pos = rf->tell_func(rf->func_arg);
  rc->size = (((((buffer[7] << 8) | buffer[6]) << 8) | buffer[5]) << 8) | buffer[4];
  rc->_size = (rc->size + 1) & ~1;

  //debug_message(__FUNCTION__ ": got chunk %s %d at %d\n", rc->name, rc->size, rc->pos);

  if (memcmp(rc->name, "LIST", 4)) {
    rc->is_list = 0;
  } else {
    rc->is_list = 1;
    if (rf->input_func(rf->func_arg, buffer, 4) != 4) {
      rf->err = _RIFF_ERR_PREMATURE_CHUNK;
      return 0;
    }
    memcpy(rc->list_name, buffer, 4);
    rc->list_name[4] = '\0';
    rc->list_pos = rf->tell_func(rf->func_arg);
    rc->list_size = rc->size - 4;
    rc->_list_size = rc->_size - 4;
  }

  return 1;
}

static int
skip_chunk_data(RIFF_File *rf, RIFF_Chunk *rc)
{
  if (!rc->is_list) {
    if (!rf->seek_func(rf->func_arg, rc->_size, _SEEK_CUR)) {
      debug_message(__FUNCTION__ ": chunk size %d\n", rc->size);
      perror(__FUNCTION__ ": seek");
      rf->err = _RIFF_ERR_TRUNCATED_CHUNK;
      return 0;
    }
  } else {
    if (!rf->seek_func(rf->func_arg, rc->_list_size, _SEEK_CUR)) {
      debug_message(__FUNCTION__ ": list size %d\n", rc->size);
      perror(__FUNCTION__ ": seek");
      rf->err = _RIFF_ERR_TRUNCATED_CHUNK;
      return 0;
    }
  }

  return 1;
}

static int
read_data(RIFF_File *rf, RIFF_Chunk *rc)
{
  //debug_message(__FUNCTION__ ": [%s] %d bytes (aligned %d bytes)\n", rc->name, rc->size, rc->_size);

  if ((rc->data = malloc(rc->_size)) == NULL) {
    rf->err = _RIFF_ERR_NO_ENOUGH_MEMORY;
    return 0;
  }
  if (rf->input_func(rf->func_arg, rc->data, rc->_size) != rc->_size) {
    rf->err = _RIFF_ERR_TRUNCATED_CHUNK;
    return 0;
  }

  return 1;
}

static int
read_data_with_seek(RIFF_File *rf, RIFF_Chunk *rc)
{
  if (!rf->seek_func(rf->func_arg, rc->pos, _SEEK_SET)) {
    rf->err = _RIFF_ERR_SEEK_FAILED;
    return 0;
  }
  return read_data(rf, rc);
}

static int
read_chunk_headers(RIFF_File *rf, RIFF_Chunk *rc, unsigned int size)
{
  unsigned int size_read;

  size_read = 0;
  while (size_read < size) {
    if (!read_chunk_header(rf, rc))
      break;
    if (rc->is_list) {
      if ((rc->list = riff_chunk_create()) == NULL) {
	rf->err = _RIFF_ERR_NO_ENOUGH_MEMORY;
	return 0;
      }
      if (!read_chunk_headers(rf, rc->list, rc->_list_size))
	return 0;
    } else {
      rc->list = NULL;
      if (!skip_chunk_data(rf, rc))
	return 0;
    }
    size_read += 8 + rc->_size;
    if ((rc->next = riff_chunk_create()) == NULL) {
      rf->err = _RIFF_ERR_NO_ENOUGH_MEMORY;
      return 0;
    }
    rc->next->prev = rc;
    rc = rc->next;
  }

  rc->prev->next = NULL;
  /* This is correct. _destroy() will destroy chunks in rc->chunks. */
  free(rc);

  if (rf->err != _RIFF_ERR_SUCCESS)
    return 0;

  return 1;
}

static int
read_all_chunk_headers(RIFF_File *rf)
{
  if ((rf->chunks = riff_chunk_create()) == NULL) {
    rf->err = _RIFF_ERR_NO_ENOUGH_MEMORY;
    return 0;
  }

  if (!rf->seek_func(rf->func_arg, 12, _SEEK_SET)) {
    rf->err = _RIFF_ERR_SEEK_FAILED;
    return 0;
  }

  return read_chunk_headers(rf, rf->chunks, rf->size - 4);
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

  if (rf->chunks) {
    for (rc = rf->chunks; rc != NULL;) {
      rc_next = rc->next;
      if (rc->data)
	free(rc->data);
      free(rc);
      rc = rc_next;
    }
  }
  free(rf);

  return 1;
}
