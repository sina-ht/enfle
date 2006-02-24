/*
 * archive.c -- archive interface
 * (C)Copyright 2000-2006 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Feb 25 02:49:48 2006.
 * $Id: archive.c,v 1.39 2006/02/24 17:55:48 sian Exp $
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

static void delete_path(Archive *, char *);
static int open(Archive *, Stream *, char *);
static void destroy(Archive *);

static Archive archive_template = {
  .open = open,
  .destroy = destroy
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

  if (parent == ARCHIVE_ROOT) {
    arc->path = strdup("");
  } else {
    arc->parent = parent;
    if (parent->pattern) {
      arc->fnmatch = parent->fnmatch;
      arc->pattern = strdup(parent->pattern);
    }
  }

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

  if (stat(path, &statbuf)) {
    err_message_fnc("stat for [%s] failed\n", path);
    return -1;
  }
  if (!S_ISDIR(statbuf.st_mode))
    return -1;

  if ((dir = opendir(path)) == NULL)
    fatal_perror(__FUNCTION__);

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

int
archive_key_compare(const void *a, const void *b)
{
  char **pa = (char **)a, **pb = (char **)b;
  char *sa = *pa, *sb = *pb;

  return strcmp(sa, sb);
}

/* methods */

int
archive_read_directory(Archive *arc, char *path, int depth)
{
  int c;
  Dlist *dl;
  Dlist_data *dd;

  if (arc->path == NULL) {
    if (!arc->parent) {
      arc->path = strdup(path);
    } else {
      int p_path_len = strlen(arc->parent->path);
      arc->path = malloc(p_path_len + 1 + strlen(path) +1);
      if (!arc->path)
	return 0;
      strcpy(arc->path, arc->parent->path);
      if (p_path_len > 0 && arc->path[p_path_len - 1] != '/') {
	arc->path[p_path_len] = '/';
	arc->path[p_path_len + 1] = '\0';
      }
      strcat(arc->path, path);
    }
    path = (char *)"";
  }

  dl = dlist_create();
  if ((c = read_directory_recursively(dl, arc->path, path, depth)) < 0)
    return 0;

  dlist_set_compfunc(dl, archive_key_compare);
  dlist_sort(dl);
  dlist_iter (dl, dd) {
    archive_add(arc, dlist_data(dd), strdup(dlist_data(dd)));
  }
  dlist_destroy(dl);

  arc->format = (char *)"NORMAL";

  return 1;
}

void
archive_set_fnmatch(Archive *arc, char *pattern, Archive_fnmatch fnm)
{
  if (arc->pattern)
    free(arc->pattern);
  arc->pattern = pattern;
  arc->fnmatch = fnm;
}

/* reminder should be a free-able region or NULL */
void
archive_add(Archive *arc, char *path, void *reminder)
{
  //debug_message_fnc("%s\n", path);

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
	fatal_perror(__FUNCTION__);
      else
	result = 0;
      break;
    case _ARCHIVE_FNMATCH_EXCLUDE:
      if ((result = fnmatch(arc->pattern, base_name, FNM_PATHNAME | FNM_PERIOD)) == FNM_NOMATCH)
	result = 1;
      else if (result)
	fatal_perror(__FUNCTION__);
      else
	result = 0;
      break;
    }

    if (!result)
      return;
  }

  if (unlikely(hash_define_str(arc->filehash, path, reminder) < 0)) {
    warning("%s: %s: %s already in filehash.\n", __FILE__, __FUNCTION__, path);
  } else {
    arc->nfiles++;
  }
}

void *
archive_get(Archive *arc, char *path)
{
  return hash_lookup_str(arc->filehash, path);
}

char *
archive_getpathname(Archive *arc, char *path)
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

  if (!hash_delete_str(arc->filehash, path)) {
    /* This is not always, but probably bug. */
    warning("%s: %s: failed to delete %s.\n", __FILE__, __FUNCTION__, path);
  }
}

#define __dlist_next_or_null(dl, dd) ((dlist_head(dl) != dd) ? dlist_next(dd) : NULL)
#define __dlist_prev_or_null(dl, dd) ((dlist_top(dl) != dd) ? dlist_prev(dd) : NULL)

static int
__iteration_index(Archive *arc)
{
  Dlist *dl = hash_get_keys(arc->filehash);
  Dlist_data *dd = dlist_top(dl), *dd_n;
  int i = 1;

  while ((dd_n = __dlist_next_or_null(dl, dd)) != NULL) {
    if (dd == arc->current)
      break;
    dd = dd_n;
    i++;
  }

  return i;
}

char *
archive_iteration_start(Archive *arc)
{
  if (!arc->current)
    return archive_iteration_first(arc);

  if (!dlist_data(arc->current))
    return NULL;

  arc->iteration_index = __iteration_index(arc);

  return hash_key_key((Hash_key *)dlist_data(arc->current));
}

char *
archive_iteration_first(Archive *arc)
{
  Dlist *dl = hash_get_keys(arc->filehash);

  if (dlist_size(dl) == 0)
    return NULL;

  arc->current = dlist_top(dl);
  arc->direction = 1;

  if (!dlist_data(arc->current))
    return NULL;

  arc->iteration_index = 1;

  return hash_key_key((Hash_key *)dlist_data(arc->current));
}

char *
archive_iteration_last(Archive *arc)
{
  Dlist *dl = hash_get_keys(arc->filehash);

  if (dlist_size(dl) == 0)
    return NULL;

  arc->direction = -1;
  arc->current = dlist_head(dl);

  if (!dlist_data(arc->current))
    return NULL;

  arc->iteration_index = arc->nfiles;

  return hash_key_key((Hash_key *)dlist_data(arc->current));
}

char *
archive_iteration_next(Archive *arc)
{
  Dlist *dl = hash_get_keys(arc->filehash);

  if ((arc->current = __dlist_next_or_null(dl, arc->current)) == NULL)
    return NULL;
  arc->direction = 1;

  if (!dlist_data(arc->current))
    return NULL;

  arc->iteration_index++;

  return hash_key_key((Hash_key *)dlist_data(arc->current));
}

char *
archive_iteration_prev(Archive *arc)
{
  Dlist *dl = hash_get_keys(arc->filehash);

  if ((arc->current = __dlist_prev_or_null(dl, arc->current)) == NULL)
    return NULL;
  arc->direction = -1;

  if (!dlist_data(arc->current))
    return NULL;

  arc->iteration_index--;

  return hash_key_key(dlist_data(arc->current));
}

char *
archive_iteration(Archive *arc)
{
  return arc->direction == 1 ? archive_iteration_next(arc) : archive_iteration_prev(arc);
}

char *
archive_delete(Archive *arc, int dir)
{
  Dlist *dl = hash_get_keys(arc->filehash);
  Dlist_data *dl_n;

  dl_n = (dir == 1) ?
    __dlist_next_or_null(dl, arc->current) :
    __dlist_prev_or_null(dl, arc->current);
  delete_path(arc, hash_key_key(dlist_data(arc->current)));
  if (!dl_n)
    return NULL;
  arc->current = dl_n;

  if (!dlist_data(arc->current))
    return NULL;

  return hash_key_key(dlist_data(arc->current));
}

#undef __dlist_next_or_null
#undef __dlist_prev_or_null

char *
archive_iteration_delete(Archive *arc)
{
  return archive_delete(arc, arc->direction);
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
  hash_destroy(arc->filehash);
  if (arc->path)
    free(arc->path);
  free(arc);
}
