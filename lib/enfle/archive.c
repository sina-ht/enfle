/*
 * archive.c -- archive interface
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Jul 29 21:35:16 2002.
 * $Id: archive.c,v 1.25 2002/08/03 04:54:06 sian Exp $
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

#define REQUIRE_FNMATCH_H
#define REQUIRE_UNISTD_H
#define REQUIRE_STRING_H
#define REQUIRE_DIRENT_H
#include "compat.h"

#define REQUIRE_FATAL_PERROR
#include "common.h"

#include "archive.h"
#include "utils/misc.h"

static int read_directory(Archive *, char *, int);
static void set_fnmatch(Archive *, char *, Archive_fnmatch);
static void add(Archive *, char *, void *);
static void *get(Archive *, char *);
static char *getpathname(Archive *, char *);
static void delete_path(Archive *, char *);
static char *delete_and_get(Archive *, int);
static char *iteration_start(Archive *);
static char *iteration_first(Archive *);
static char *iteration_last(Archive *);
static char *iteration_next(Archive *);
static char *iteration_prev(Archive *);
static char *iteration(Archive *);
static char *iteration_delete(Archive *);
static int open(Archive *, Stream *, char *);
static void destroy(Archive *);

static Archive archive_template = {
  read_directory: read_directory,
  set_fnmatch: set_fnmatch,
  add: add,
  get: get,
  delete_and_get: delete_and_get,
  getpathname: getpathname,
  iteration_start: iteration_start,
  iteration_first: iteration_first,
  iteration_last: iteration_last,
  iteration_next: iteration_next,
  iteration_prev: iteration_prev,
  iteration: iteration,
  iteration_delete: iteration_delete,
  open: open,
  destroy: destroy
};

Archive *
archive_create(Archive *parent)
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
  if (parent) {
    if (parent->pattern) {
      arc->fnmatch = parent->fnmatch;
      arc->pattern = strdup(parent->pattern);
    }
    if (parent->path)
      arc->path = strdup(parent->path);
  } else
    arc->path = NULL;

  return arc;
}

/* for internal use */

static int
read_directory_recursively(Dlist *dl, char *basepath, char *addpath, int depth)
{
  DIR *dir;
  struct dirent *ent;
  struct stat statbuf;
  int count = 0;
  char path[strlen(basepath) + strlen(addpath) + 1];

  strcpy(path, basepath);
  strcat(path, addpath);

  if (stat(path, &statbuf) || !S_ISDIR(statbuf.st_mode))
    return -1;

  if ((dir = opendir(path)) == NULL)
    fatal_perror(1, __FUNCTION__);

  while ((ent = readdir(dir))) {
    char fullpath[strlen(path) + strlen(ent->d_name) + 1];
    char filepath[strlen(addpath) + strlen(ent->d_name) + 2];
    /* +2 is needed for last '/'. */

    if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
      continue;
    strcpy(fullpath, path);
    strcat(fullpath, ent->d_name);
    strcpy(filepath, addpath);
    strcat(filepath, ent->d_name);
    if (!stat(fullpath, &statbuf)) {
      if (S_ISDIR(statbuf.st_mode)) {
	strcat(filepath, "/");
	if (depth > 1)
	  depth--;
	if (depth == 0 || depth > 1) {
	  count += read_directory_recursively(dl, basepath, filepath, depth);
	} else {
	  dlist_add_str(dl, filepath);
	  count++;
	}
      } else if (S_ISREG(statbuf.st_mode)) {
	dlist_add_str(dl, filepath);
	count++;
      }
    }
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

  if (arc->path == NULL) {
    if ((arc->path = misc_canonical_pathname(path)) == NULL)
      return 0;
    dl = dlist_create();
    if ((c = read_directory_recursively(dl, arc->path, (char *)"", depth)) < 0)
      return 0;
  } else {
    dl = dlist_create();
    if ((c = read_directory_recursively(dl, arc->path, path, depth)) < 0)
      return 0;
  }

  dlist_set_compfunc(dl, key_compare);
  dlist_sort(dl);
  dlist_iter (dl, dd) {
    add(arc, dlist_data(dd), strdup(dlist_data(dd)));
  }
  dlist_destroy(dl, 1);

  arc->format = (char *)"NORMAL";

  return 1;
}

static void
set_fnmatch(Archive *arc, char *pattern, Archive_fnmatch fnm)
{
  if (arc->pattern)
    free(arc->pattern);
  arc->pattern = pattern;
  arc->fnmatch = fnm;
}

static void
add(Archive *arc, char *path, void *reminder)
{
  if (arc->pattern) {
    int result = 0;
    char *base_name;

    base_name = misc_basename(path);
    switch (arc->fnmatch) {
    case _ARCHIVE_FNMATCH_ALL:
      result = 1;
      break;
    case _ARCHIVE_FNMATCH_INCLUDE:
      if ((result = fnmatch(arc->pattern, base_name, FNM_PATHNAME | FNM_PERIOD)) == 0)
	result = 1;
      else if (result != FNM_NOMATCH)
	fatal_perror(3, __FUNCTION__);
      else
	result = 0;
      break;
    case _ARCHIVE_FNMATCH_EXCLUDE:
      if ((result = fnmatch(arc->pattern, base_name, FNM_PATHNAME | FNM_PERIOD)) == FNM_NOMATCH)
	result = 1;
      else if (result)
	fatal_perror(3, __FUNCTION__);
      else
	result = 0;
      break;
    }

    if (!result)
      return;
  }

  arc->nfiles++;
  if (hash_define_str(arc->filehash, path, reminder) < 0) {
    warning("%s: %s: %s already in filehash.\n", __FILE__, __FUNCTION__, path);
  }
}

static void *
get(Archive *arc, char *path)
{
  return hash_lookup_str(arc->filehash, path);
}

static char *
getpathname(Archive *arc, char *path)
{
  char *fullpath;

  if (!arc->path || strlen(arc->path) == 0)
    return strdup(path);
  if ((fullpath = malloc(strlen(arc->path) + strlen(path) + 2)) == NULL)
    return NULL;
  strcpy(fullpath, arc->path);
  if (strcmp(arc->format, "NORMAL") != 0)
    strcat(fullpath, "#");
  strcat(fullpath, path);

  return fullpath;
}

static void
delete_path(Archive *arc, char *path)
{
  arc->nfiles--;
  if (arc->nfiles < 0) {
    warning("%s: %s: arc->nfiles = %d < 0\n", __FILE__, __FUNCTION__, arc->nfiles);
  }

  if (!hash_delete_str(arc->filehash, path, 1)) {
    /* This is not always, but probably bug. */
    warning("%s: %s: failed to delete %s.\n", __FILE__, __FUNCTION__, path);
  }
}

static char *
iteration_start(Archive *arc)
{
  Dlist *dl;

  dl = hash_get_keys(arc->filehash);
  if (!arc->current)
    return iteration_first(arc);

  if (arc->current == dlist_guard(dl))
    return NULL;

  if (!dlist_data(arc->current))
    return NULL;

  return hash_key_key((Hash_key *)dlist_data(arc->current));
}

static char *
iteration_first(Archive *arc)
{
  Dlist *dl;

  arc->direction = 1;

  dl = hash_get_keys(arc->filehash);
  arc->current = dlist_top(dl);

  if (arc->current == dlist_guard(dl))
    return NULL;

  if (!dlist_data(arc->current))
    return NULL;

  return hash_key_key((Hash_key *)dlist_data(arc->current));
}

static char *
iteration_last(Archive *arc)
{
  Dlist *dl;

  arc->direction = -1;

  dl = hash_get_keys(arc->filehash);
  arc->current = dlist_head(dl);

  if (arc->current == dlist_guard(dl))
    return NULL;

  if (!dlist_data(arc->current))
    return NULL;

  return hash_key_key((Hash_key *)dlist_data(arc->current));
}

static char *
iteration_next(Archive *arc)
{
  Dlist *dl;
  dl = hash_get_keys(arc->filehash);

  arc->direction = 1;

  if (arc->current == dlist_guard(dl))
    return iteration_start(arc);

  if (dlist_next(arc->current) == dlist_guard(dl))
    return NULL;

  arc->current = dlist_next(arc->current);

  if (!dlist_data(arc->current))
    return NULL;

  return hash_key_key((Hash_key *)dlist_data(arc->current));
}

static char *
iteration_prev(Archive *arc)
{
  Dlist *dl;
  dl = hash_get_keys(arc->filehash);

  arc->direction = -1;

  if (arc->current == dlist_guard(dl)) {
    Dlist *_dl;

    _dl = hash_get_keys(arc->filehash);
    arc->current = dlist_head(_dl);
  } else {
    if (dlist_prev(arc->current) == dlist_guard(dl))
      return NULL;
    arc->current = dlist_prev(arc->current);
  }

  if (!dlist_data(arc->current))
    return NULL;

  return hash_key_key(dlist_data(arc->current));
}

static char *
iteration(Archive *arc)
{
  return arc->direction == 1 ? iteration_next(arc) : iteration_prev(arc);
}

static char *
delete_and_get(Archive *arc, int dir)
{
  Dlist *dl;
  Dlist_data *dl_n;
  dl = hash_get_keys(arc->filehash);

  if (arc->current != dlist_guard(dl)) {
    dl_n = (dir == 1) ? dlist_next(arc->current) : dlist_prev(arc->current);
    delete_path(arc, hash_key_key(dlist_data(arc->current)));
    arc->current = dl_n;
    if (arc->current == dlist_guard(dl))
      return NULL;
    if (!dlist_data(arc->current))
      return NULL;
    return hash_key_key(dlist_data(arc->current));
  } else
    return NULL;
}

static char *
iteration_delete(Archive *arc)
{
  return delete_and_get(arc, arc->direction);
}

static int
open(Archive *arc, Stream *st, char *path)
{
  char fullpath[strlen(arc->path) + strlen(path) + 1];

  strcpy(fullpath, arc->path);
  strcat(fullpath, path);
  return stream_make_filestream(st, fullpath);
}

static void
destroy(Archive *arc)
{
  if (arc->pattern)
    free(arc->pattern);
  hash_destroy(arc->filehash, 1);
  if (arc->path)
    free(arc->path);
  free(arc);
}
