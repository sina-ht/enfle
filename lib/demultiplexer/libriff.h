/*
 * libriff.h -- RIFF file read library header
 * (C)Copyright 2001-2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Jan 18 14:01:53 2004.
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

#ifndef _LIBRIFF_H
#define _LIBRIFF_H

typedef enum _riff_seekwhence RIFF_SeekWhence;
typedef enum _riff_errcode RIFF_Errcode;
typedef struct _riff_file RIFF_File;
typedef struct _riff_chunk RIFF_Chunk;

enum _riff_seekwhence {
  _SEEK_SET = 0,
  _SEEK_CUR,
  _SEEK_END
};

enum _riff_errcode {
  _RIFF_ERR_SUCCESS = 0,      /* Success */
  _RIFF_ERR_EOF = 0,          /* EOF reached */
  _RIFF_ERR_TOO_SMALL,        /* File size too small */
  _RIFF_ERR_NOT_RIFF,         /* RIFF tag not found */
  _RIFF_ERR_NO_ENOUGH_MEMORY, /* No enough memory */
  _RIFF_ERR_PREMATURE_CHUNK,  /* Premature chunk */
  _RIFF_ERR_TRUNCATED_CHUNK,  /* Truncated chunk */
  _RIFF_ERR_SEEK_FAILED,      /* Seek failed */
  _RIFF_ERR_MAXCODE
};

struct _riff_chunk {
  unsigned int fourcc;
  char name[5];
  unsigned int list_fourcc;
  char list_name[5];
  int is_list;
  unsigned int size;
  unsigned int list_size;
  unsigned int pos;
  unsigned int list_pos;
  /* protected, _size and _list_size have aligned size */
  unsigned int _size;
  unsigned int _list_size;
  void *data;
};

typedef int (*RIFF_Input_func)(void *, void *, int);
typedef int (*RIFF_Seek_func)(void *, unsigned int, RIFF_SeekWhence);
typedef int (*RIFF_Tell_func)(void *);

#define riff_chunk_get_name(rc) (rc)->name
#define riff_chunk_is_list(rc) (rc)->is_list
#define riff_chunk_get_list_name(rc) (rc)->list_name
#define riff_chunk_get_size(rc) (rc)->size
#define riff_chunk_get_pos(rc) (rc)->pos
#define riff_chunk_get_data(rc) (rc)->data

RIFF_Chunk *riff_chunk_create(void);
int riff_chunk_destroy(RIFF_Chunk *);
int riff_chunk_free_data(RIFF_Chunk *);

struct _riff_file {
  RIFF_Input_func input_func;
  RIFF_Seek_func seek_func;
  RIFF_Tell_func tell_func;
  void *func_arg;
  unsigned char form[5];
  unsigned int size;
  RIFF_Errcode err;
};

#define riff_file_is_opened(rf) ((rf)->form[0] != '\0')
#define riff_file_get_form(rf) (rf)->form
#define riff_file_get_size(rf) (rf)->size
#define riff_file_get_err(rf) (rf)->err
#define riff_file_is_eof(rf) riff_file_get_err(rf) == _RIFF_ERR_EOF
#define riff_file_set_func_input(rf, rfi) (rf)->input_func = (rfi)
#define riff_file_set_func_seek(rf, rfs) (rf)->seek_func = (rfs)
#define riff_file_set_func_tell(rf, rft) (rf)->tell_func = (rft)
#define riff_file_set_func_arg(rf, rfa) (rf)->func_arg = (rfa)

RIFF_File *riff_file_create(void);
int riff_file_open(RIFF_File *);
int riff_file_read_chunk_header(RIFF_File *, RIFF_Chunk *);
int riff_file_skip_chunk_data(RIFF_File *, RIFF_Chunk *);
int riff_file_read_data(RIFF_File *, RIFF_Chunk *);
const unsigned char *riff_file_get_errmsg(RIFF_File *);
int riff_file_destroy(RIFF_File *);

#endif
