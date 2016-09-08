/*
 * libriff.c -- RIFF file read library
 * (C)Copyright 2001-2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue May  3 09:42:38 2005.
 * $Id: libriff.c,v 1.2 2005/05/03 01:08:30 sian Exp $
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

#include <stdlib.h>
#include "libriff.h"

#include "common.h"
#define REQUIRE_STRING_H
#include "compat.h"

RIFF_Chunk *
riff_chunk_create(void)
{
  RIFF_Chunk *rc;

  if ((rc = calloc(1, sizeof(RIFF_Chunk))) == NULL)
    return NULL;

  return rc;
}

int
riff_chunk_destroy(RIFF_Chunk *rc)
{
  if (rc)
    free(rc);

  return 1;
}

int
riff_chunk_free_data(RIFF_Chunk *rc)
{
  if (rc->data) {
    free(rc->data);
    rc->data = NULL;
  }

  return 1;
}

RIFF_File *
riff_file_create(void)
{
  RIFF_File *rf;

  if ((rf = calloc(1, sizeof(RIFF_File))) == NULL)
    return NULL;

  return rf;
}

int
riff_file_open(RIFF_File *rf)
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

int
riff_file_read_chunk_header(RIFF_File *rf, RIFF_Chunk *rc)
{
  unsigned char buffer[8];
  unsigned int chunk_read;
  int byte_read;

  if ((chunk_read = rf->input_func(rf->func_arg, buffer, 8)) != 8) {
    if (chunk_read == 0) {
      /* Reach end of file. */
      rf->err = _RIFF_ERR_EOF;
      return 0;
    }
    debug_message_fnc("need 8bytes but got only %d bytes.\n", chunk_read);
    rf->err = _RIFF_ERR_PREMATURE_CHUNK;
    return 0;
  }
  rc->pos = rf->tell_func(rf->func_arg);
  if (rc->pos > rf->size) {
    rf->err = _RIFF_ERR_EOF;
    return 0;
  }
  memcpy(rc->name, buffer, 4);
  rc->name[4] = '\0';
  rc->fourcc = (((((rc->name[3] << 8) | rc->name[2]) << 8) | rc->name[1]) << 8) | rc->name[0];
  rc->size = (((((buffer[7] << 8) | buffer[6]) << 8) | buffer[5]) << 8) | buffer[4];
  rc->_size = (rc->size + 1) & ~1;

  //debug_message_fnc("got chunk %s %d at %d\n", rc->name, rc->size, rc->pos);

  if (memcmp(rc->name, "LIST", 4)) {
    rc->is_list = 0;
  } else {
    rc->is_list = 1;
    if ((byte_read = rf->input_func(rf->func_arg, buffer, 4)) != 4) {
      debug_message_fnc("need 4bytes but got only %d bytes.\n", byte_read);
      rf->err = _RIFF_ERR_PREMATURE_CHUNK;
      return 0;
    }
    memcpy(rc->list_name, buffer, 4);
    rc->list_name[4] = '\0';
    rc->list_fourcc = (((((rc->list_name[3] << 8) | rc->list_name[2]) << 8) | rc->list_name[1]) << 8) | rc->list_name[0];
    rc->list_pos = rf->tell_func(rf->func_arg);
    rc->list_size = rc->size - 4;
    rc->_list_size = rc->_size - 4;
  }

  return 1;
}

int
riff_file_skip_chunk_data(RIFF_File *rf, RIFF_Chunk *rc)
{
  if (!rc->is_list) {
    if (!rf->seek_func(rf->func_arg, rc->_size, _SEEK_CUR)) {
      debug_message_fnc("chunk size %d\n", rc->size);
      perror(__FUNCTION__);
      rf->err = _RIFF_ERR_TRUNCATED_CHUNK;
      return 0;
    }
  } else {
    if (!rf->seek_func(rf->func_arg, rc->_list_size, _SEEK_CUR)) {
      debug_message_fnc("list size %d\n", rc->size);
      perror(__FUNCTION__);
      rf->err = _RIFF_ERR_TRUNCATED_CHUNK;
      return 0;
    }
  }

  return 1;
}

int
riff_file_read_data(RIFF_File *rf, RIFF_Chunk *rc)
{
  int byte_read;

  if (rc->_size > 0) {
    if ((rc->data = malloc(rc->_size)) == NULL) {
      rf->err = _RIFF_ERR_NO_ENOUGH_MEMORY;
      return 0;
    }
    if ((byte_read = rf->input_func(rf->func_arg, rc->data, rc->_size)) != (int)rc->_size) {
      debug_message_fnc("requested %d bytes, but got %d bytes\n", rc->_size, byte_read);
      rf->err = _RIFF_ERR_TRUNCATED_CHUNK;
      return 0;
    }
  }

  return 1;
}

const char *
riff_file_get_errmsg(RIFF_File *rf)
{
  static const char *errmsg[] = {
    "Success.",
    "File size too small.",
    "RIFF tag not found.",
    "No enough memory.",
    "Premature chunk.",
    "Truncated chunk.",
    "Seek failed."
  };

  if (rf->err >= _RIFF_ERR_MAXCODE)
    return "Unknown error.";
  return errmsg[rf->err];
}

int
riff_file_destroy(RIFF_File *rf)
{
  if (rf)
    free(rf);

  return 1;
}
