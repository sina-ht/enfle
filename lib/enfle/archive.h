/*
 * archive.h -- archive interface header
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Thu Apr 19 15:59:39 2001.
 * $Id: archive.h,v 1.4 2001/04/20 07:24:58 sian Exp $
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

#ifndef _ARCHIVE_H
#define _ARCHIVE_H

#include "utils/hash.h"
#include "stream.h"

typedef enum _archive_fnmatch {
  _ARCHIVE_FNMATCH_ALL = 0,
  _ARCHIVE_FNMATCH_INCLUDE,
  _ARCHIVE_FNMATCH_EXCLUDE
} Archive_fnmatch;

typedef struct _archive Archive;
struct _archive {
  Hash *filehash;
  Dlist_data *current;
  Stream *st;
  char *format;
  char *path;
  char *pattern;
  Archive_fnmatch fnmatch;
  int direction;
  int nfiles;
  void *data;

  int (*read_directory)(Archive *, char *, int);
  void (*set_fnmatch)(Archive *, char *, Archive_fnmatch);
  void (*add)(Archive *, char *, void *);
  void *(*get)(Archive *, char *);
  char *(*iteration_start)(Archive *);
  char *(*iteration_next)(Archive *);
  char *(*iteration_prev)(Archive *);
  char *(*iteration)(Archive *);
  void (*iteration_delete)(Archive *);
  int (*open)(Archive *, Stream *, char *);

  void (*destroy)(Archive *);
};

#define ARCHIVE_FILEHASH_SIZE 4096

#define archive_read_directory(a, p, d) (a)->read_directory((a), (p), (d))
#define archive_set_fnmatch(a, p, f) (a)->set_fnmatch((a), (p), (f))
#define archive_add(a, p, rem) (a)->add((a), (p), (rem))
#define archive_get(a, p) (a)->get((a), (p))
#define archive_iteration_start(a) (a)->iteration_start((a))
#define archive_iteration_next(a) (a)->iteration_next((a))
#define archive_iteration_prev(a) (a)->iteration_prev((a))
#define archive_iteration(a) (a)->iteration((a))
#define archive_iteration_delete(a) (a)->iteration_delete((a))
#define archive_open(a, st, n) (a)->open((a), (st), (n))
#define archive_destroy(a) (a)->destroy((a))

#define ARCHIVE_ROOT NULL

Archive *archive_create(Archive *);

#endif
