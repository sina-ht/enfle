/*
 * libriff.h -- RIFF file read library header
 * (C)Copyright 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Sep  2 23:26:51 2001.
 * $Id: libriff.h,v 1.1 2001/09/03 00:31:02 sian Exp $
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
  _RIFF_ERR_TOO_SMALL,        /* File size too small */
  _RIFF_ERR_NOT_RIFF,         /* RIFF tag not found */
  _RIFF_ERR_NO_ENOUGH_MEMORY, /* No enough memory */
  _RIFF_ERR_PREMATURE_CHUNK,  /* Premature chunk */
  _RIFF_ERR_TRUNCATED_CHUNK,  /* Truncated chunk */
  _RIFF_ERR_SEEK_FAILED,      /* Seek failed */
  _RIFF_ERR_NO_LIST,          /* No list but pushed */
  _RIFF_ERR_MAXCODE
};

struct _riff_chunk {
  unsigned char name[5];
  unsigned char list_name[5];
  int is_list;
  unsigned int size;
  unsigned int list_size;
  unsigned int pos;
  unsigned int list_pos;
  /* protected, _size and _list_size have aligned size */
  unsigned int _size;
  unsigned int _list_size;
  void *data;
  RIFF_Chunk *list;
  RIFF_Chunk *prev;
  RIFF_Chunk *next;

  int (*destroy)(RIFF_Chunk *);
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
#define riff_chunk_destroy(rc) (rc)->destroy((rc))

RIFF_Chunk *riff_chunk_create(void);

typedef struct _iter_stack _RIFF_iter_stack;
struct _iter_stack {
  RIFF_Chunk *rc;
  _RIFF_iter_stack *next;
  _RIFF_iter_stack *prev;
};

struct _riff_file {
  RIFF_Input_func input_func;
  RIFF_Seek_func seek_func;
  RIFF_Tell_func tell_func;
  void *func_arg;
  unsigned char form[5];
  unsigned int size;
  RIFF_Errcode err;
  RIFF_Chunk *chunks;
  RIFF_Chunk *iter_current;
  RIFF_Chunk *iter_head;
  /* protected */
  _RIFF_iter_stack *stack;

  int (*open_file)(RIFF_File *);
  int (*read_chunk_header)(RIFF_File *, RIFF_Chunk *);
  int (*skip_chunk_data)(RIFF_File *, RIFF_Chunk *);
  int (*read_data)(RIFF_File *, RIFF_Chunk *);
  int (*read_data_with_seek)(RIFF_File *, RIFF_Chunk *);
  int (*read_chunk_headers)(RIFF_File *, RIFF_Chunk *, unsigned int);
  int (*read_all_chunk_headers)(RIFF_File *);
  int (*iter_set)(RIFF_File *, RIFF_Chunk *);
  int (*iter_next_chunk)(RIFF_File *, RIFF_Chunk **);
  int (*iter_push)(RIFF_File *);
  int (*iter_pop)(RIFF_File *);
  const unsigned char *(*get_errmsg)(RIFF_File *);
  int (*destroy)(RIFF_File *);
};

#define riff_file_is_opened(rf) ((rf)->form[0] != '\0')
#define riff_file_get_form(rf) (rf)->form
#define riff_file_get_size(rf) (rf)->size
#define riff_file_get_err(rf) (rf)->err
#define riff_file_set_func_input(rf, rfi) (rf)->input_func = (rfi)
#define riff_file_set_func_seek(rf, rfs) (rf)->seek_func = (rfs)
#define riff_file_set_func_tell(rf, rft) (rf)->tell_func = (rft)
#define riff_file_set_func_arg(rf, rfa) (rf)->func_arg = (rfa)

#define riff_file_open(rf) (rf)->open_file((rf))
#define riff_file_read_chunk_header(rf, rc) (rf)->read_chunk_header((rf), (rc))
#define riff_file_skip_chunk_data(rf, rc) (rf)->skip_chunk_data((rf), (rc))
#define riff_file_read_data(rf, rc) (rf)->read_data((rf), (rc))
#define riff_file_read_data_with_seek(rf, rc) (rf)->read_data_with_seek((rf), (rc))
#define riff_file_read_chunk_headers(rf, rc, s) (rf)->read_chunk_headers((rf), (rc), (s))
#define riff_file_read_all_chunk_headers(rf, rc) (rf)->read_all_chunk_header((rf), (rc))
#define riff_file_iter_set(rf, rc) (rf)->iter_set((rf), (rc))
#define riff_file_iter_reset(rf) if ((rf)->chunks) riff_file_iter_set((rf), ((rf)->chunks))
#define riff_file_iter_next_chunk(rf, rcp) (rf)->iter_next_chunk((rf), (rcp))
#define riff_file_iter_push(rf) (rf)->iter_push((rf))
#define riff_file_iter_pop(rf) (rf)->iter_pop((rf))
#define riff_file_get_errmsg(rf) (rf)->get_errmsg((rf))
#define riff_file_destroy(rf) (rf)->destroy((rf))

RIFF_File *riff_file_create(void);

#endif
