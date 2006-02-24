/*
 * archive.h -- archive interface header
 * (C)Copyright 2000-2006 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Feb 25 02:24:06 2006.
 * $Id: archive.h,v 1.16 2006/02/24 17:55:48 sian Exp $
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
  Archive *parent;
  int nfiles;
  Hash *filehash;
  Stream *st;
  char *format;
  char *path;

  char *pattern;
  Archive_fnmatch fnmatch;

  int iteration_index;
  int direction;
  Dlist_data *current;

  void *data;

  /* Overridden by an archive plugin */
  int (*open)(Archive *, Stream *, char *);
  void (*destroy)(Archive *);
};

#define ARCHIVE_FILEHASH_SIZE 65537

#define archive_open(a, st, n) (a)->open((a), (st), (n))
#define archive_destroy(a) (a)->destroy((a))

#define archive_direction(a) ((a)->direction)
#define archive_nfiles(a) ((a)->nfiles)
#define archive_iteration_index(a) ((a)->iteration_index)

#define ARCHIVE_ROOT NULL

Archive *archive_create(Archive *);
int archive_key_compare(const void *, const void *);
int archive_read_directory(Archive *arc, char *path, int depth);
void archive_set_fnmatch(Archive *arc, char *pattern, Archive_fnmatch fnm);
void archive_add(Archive *arc, char *path, void *reminder);
void *archive_get(Archive *arc, char *path);
char *archive_delete(Archive *arc, int dir);
char *archive_getpathname(Archive *arc, char *path);
char *archive_iteration_start(Archive *arc);
char *archive_iteration_first(Archive *arc);
char *archive_iteration_last(Archive *arc);
char *archive_iteration_next(Archive *arc);
char *archive_iteration_prev(Archive *arc);
char *archive_iteration(Archive *arc);
char *archive_iteration_delete(Archive *arc);

#endif
