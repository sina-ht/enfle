/*
 * archive.c -- archive interface
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Jan 14 10:57:01 2001.
 * $Id: archive.c,v 1.7 2001/01/14 15:23:18 sian Exp $
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

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define REQUIRE_UNISTD_H
#define REQUIRE_STRING_H
#define REQUIRE_DIRENT_H
#include "compat.h"

#define REQUIRE_FATAL
#define REQUIRE_FATAL_PERROR
#include "common.h"

#include "archive.h"

static int read_directory(Archive *, char *, int);
static void add(Archive *, char *, void *);
static void *get(Archive *, char *);
static void delete(Archive *, char *, int);
static char *iteration_start(Archive *);
static char *iteration_next(Archive *);
static char *iteration_prev(Archive *);
static void iteration_delete(Archive *);
static int open(Archive *, Stream *, char *);
static void destroy(Archive *);

static Archive archive_template = {
  filehash: NULL,
  st: NULL,
  format: NULL,
  nfiles: 0,
  data: NULL,
  read_directory: read_directory,
  add: add,
  get: get,
  iteration_start: iteration_start,
  iteration_next: iteration_next,
  iteration_prev: iteration_prev,
  iteration_delete: iteration_delete,
  open: open,
  destroy: destroy
};

Archive *
archive_create(void)
{
  Archive *arc;

  if ((arc = calloc(1, sizeof(Archive))) == NULL)
    return NULL;
  memcpy(arc, &archive_template, sizeof(Archive));
  if ((arc->filehash = hash_create(ARCHIVE_FILEHASH_SIZE)) == NULL) {
    free(arc);
    return NULL;
  }
  arc->format = (char *)"NORMAL";

  return arc;
}

/* for internal use */

static int
read_directory_recursively(Dlist *dl, char *path, int depth)
{
  char *filepath;
  DIR *dir;
  struct dirent *ent;
  struct stat statbuf;
  int count = 0;

  if (stat(path, &statbuf) || !S_ISDIR(statbuf.st_mode))
    return 0;

  if ((dir = opendir(path)) == NULL)
    fatal_perror(1, "archive.c: read_directory: ");

  while ((ent = readdir(dir))) {
    if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
      continue;
    if ((filepath = calloc(1, strlen(path) + strlen(ent->d_name) + 2)) == NULL)
      fatal(1, "archive.c: read_directory: No enough memory for filepath.\n");
    strcpy(filepath, path);
    if (filepath[strlen(filepath) - 1] != '/')
      strcat(filepath, "/");
    strcat(filepath, ent->d_name);
    if (!stat(filepath, &statbuf)) {
      if (S_ISDIR(statbuf.st_mode)) {
	if (depth > 1)
	  depth--;
	if (depth == 0 || depth > 1)
	  count += read_directory_recursively(dl, filepath, depth);
	else {
	  dlist_add_str(dl, filepath);
	  count++;
	}
      } else if (S_ISREG(statbuf.st_mode)) {
	dlist_add_str(dl, filepath);
	count++;
      }
    }
    free(filepath);
  }
  closedir(dir);

  return count;
}

static int
key_compare(const void *a, const void *b)
{
  char **pa = (char **)a, **pb = (char **)b;
  char *sa = *pa, *sb = *pb;

  return strcmp(sa, sb);
}

/* methods */

static int
read_directory(Archive *arc, char *path, int depth)
{
  int c;
  Dlist *dl;
  Dlist_data *dd;

  dl = dlist_create();
  dlist_set_compfunc(dl, key_compare);
  c = read_directory_recursively(dl, path, depth);
#if 0
  if (arc->nfiles != c) {
    fprintf(stderr, "BUG: arc->nfiles %d != %d read_directory_recursively()\n", arc->nfiles, c);
    return 0;
  }
#endif

  dlist_sort(dl);
  dlist_iter (dl, dd) {
    add(arc, dlist_data(dd), strdup(dlist_data(dd)));
  }
  dlist_destroy(dl);

  arc->format = (char *)"NORMAL";

  return 1;
}

static void
add(Archive *arc, char *path, void *reminder)
{
  arc->nfiles++;
  if (hash_define(arc->filehash, path, (void *)reminder) < 0)
    fprintf(stderr, "archive.c: add: %s already in filehash.\n", path);
}

static void *
get(Archive *arc, char *path)
{
  return hash_lookup(arc->filehash, path);
}

static void
delete(Archive *arc, char *path, int dir)
{
  arc->nfiles--;
  arc->current = (dir == 1) ? dlist_prev(arc->current) : dlist_next(arc->current);
  if (!hash_delete(arc->filehash, path, 1))
    fprintf(stderr, "archive.c: delete: delete %s failed.\n", path);
}

static char *
iteration_start(Archive *arc)
{
  Dlist *dl;

  arc->direction = 1;

  dl = hash_get_keys(arc->filehash);
  arc->current = dlist_get_top(dl);

  if (!arc->current)
    return NULL;

  return dlist_data(arc->current);
}

static char *
iteration_next(Archive *arc)
{
  arc->direction = 1;

  if (arc->current == NULL)
    return iteration_start(arc);

  if (dlist_next(arc->current) == NULL)
    return NULL;

  arc->current = dlist_next(arc->current);

  return dlist_data(arc->current);
}

static char *
iteration_prev(Archive *arc)
{
  arc->direction = -1;

  if (arc->current == NULL) {
    Dlist *dl;

    dl = hash_get_keys(arc->filehash);
    arc->current = dlist_get_head(dl);
  } else {
    if (dlist_prev(arc->current) == NULL)
      return NULL;
    arc->current = dlist_prev(arc->current);
  }

  return dlist_data(arc->current);
}

static void
iteration_delete(Archive *arc)
{
  delete(arc, dlist_data(arc->current), arc->direction);
}

static int
open(Archive *arc, Stream *st, char *path)
{
  return stream_make_filestream(st, path);
}

static void
destroy(Archive *arc)
{
  hash_destroy(arc->filehash, 0);
  free(arc);
}
