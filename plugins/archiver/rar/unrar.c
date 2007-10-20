/*
 * rar.c -- unrarlib archiver plugin
 * (C)Copyright 2000-2007 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Oct  8 19:50:23 2007.
 * $Id: unrar.c,v 1.1 2007/10/20 13:41:55 sian Exp $
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

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "unrar/dll.hpp"

#include "common.h"

#include "enfle/archiver-plugin.h"

DECLARE_ARCHIVER_PLUGIN_METHODS;

typedef struct _rar_data {
  HANDLE r;
  int nfiles;
  int current_index;
} Rar_data;

typedef struct _rar_info {
  Rar_data *rar;
  int idx;
} Rar_info;

static ArchiverPlugin plugin = {
  .type = ENFLE_PLUGIN_ARCHIVER,
  .name = "RAR",
  .description = "rar Archiver plugin version 0.2",
  .author = "Hiroshi Takekawa",
  .archiver_private = NULL,

  .identify = identify,
  .open = open
};

ENFLE_PLUGIN_ENTRY(archiver_rar)
{
  ArchiverPlugin *arp;

  if ((arp = (ArchiverPlugin *)calloc(1, sizeof(ArchiverPlugin))) == NULL)
    return NULL;
  memcpy(arp, &plugin, sizeof(ArchiverPlugin));

  return (void *)arp;
}

ENFLE_PLUGIN_EXIT(arciver_rar, p)
{
  free(p);
}

static int /* overrides archive::open */
rar_open(Archive *arc, Stream *st, char *path)
{
  struct RAROpenArchiveData ro;
  Rar_info *info = (Rar_info *)archive_get(arc, path);
  Rar_data *rar;
  int i;

  if (info == NULL) {
    err_message_fnc("archive_get(%s) failed.\n", path);
    return OPEN_ERROR;
  }
  rar = info->rar;

  if (rar->current_index > info->idx) {
    if (rar->r)
        RARCloseArchive(rar->r);
    memset(&ro, 0, sizeof(ro));
    ro.ArcName = arc->path;
    ro.OpenMode = RAR_OM_EXTRACT;
    rar->r = RAROpenArchive(&ro);
    rar->current_index = 0;
  }
  if (rar->r == NULL)
    return 0;

  for (i = 0;; i++) {
    struct RARHeaderData rh;
    int res;

    if ((res = RARReadHeader(rar->r, &rh)) != 0) {
      err_message_fnc("RARReadHeader() returned %d\n", res);
      RARCloseArchive(rar->r);
      rar->r = NULL;
      return 0;
    }
    rar->current_index++;
    if (rh.UnpSize > 0 && strcmp(rh.FileName, path + 1) == 0)
      break;
    RARProcessFile(rar->r, RAR_SKIP, NULL, NULL);
  }
  debug_message_fnc("current_index = %d, file idx = %d\n", rar->current_index, info->idx);

  RARProcessFile(rar->r, RAR_EXTRACT, (char *)"", (char *)"/tmp/unrar-extracted");

  if (!stream_make_filestream(st, (char *)"/tmp/unrar-extracted"))
    return 0;
  free(st->path);
  st->path = strdup(path);

  return 1;
}

static void /* overrides archive::destroy */
rar_destroy(Archive *arc)
{
  Rar_data *rar = arc->data;

  debug_message_fnc("%s\n", arc->st->path);
  if (rar) {
    if (rar->r)
      RARCloseArchive(rar->r);
    free(rar);
  }
  stream_destroy(arc->st);
  hash_destroy(arc->filehash);
  if (arc->path)
    free(arc->path);
  free(arc);
}

/* methods */
DEFINE_ARCHIVER_PLUGIN_IDENTIFY(a, st, priv)
{
  unsigned char buf[7];
  const char signature[] = { 0x52, 0x61, 0x72, 0x21, 0x1a, 0x07, 0x00 };

  if (stream_read(st, buf, 7) != 7)
    return OPEN_NOT;
  if (memcmp(buf, signature, 7) != 0)
    return OPEN_NOT;

  return OPEN_OK;
}

DEFINE_ARCHIVER_PLUGIN_OPEN(a, st, priv)
{
  struct RAROpenArchiveData ro;
  Rar_data *rar;
  Rar_info *info;
  int i;
  Hash *hash;
  Dlist *dl;
  Dlist_data *dd;
  ArchiverStatus res = OPEN_NOT;

  if ((rar = calloc(1, sizeof(*rar))) == NULL)
    return OPEN_NOT;

  memset(&ro, 0, sizeof(ro));
  ro.ArcName = st->path;
  ro.OpenMode = RAR_OM_LIST;
  rar->r = RAROpenArchive(&ro);
  if (rar->r == NULL)
    goto out_free_rar;
  a->data = rar;

  dl = dlist_create();
  hash = hash_create(ARCHIVE_FILEHASH_SIZE);
  for (i = 0;; i++) {
    struct RARHeaderData rh;
    int result;
    char *path;

    if ((result = RARReadHeader(rar->r, &rh)) == 0) {
      if (rh.UnpSize > 0) {
	if ((path = calloc(1, strlen(rh.FileName) + 2)) == NULL) {
	  res = OPEN_ERROR;
	  goto out_free_rar;
	}
	path[0] = '#';
	strcat(path, rh.FileName);
	dlist_add(dl, path);
	rar->nfiles++;
	if ((info = calloc(1, sizeof(*info))) == NULL) {
	  res = OPEN_ERROR;
	  goto out_free_rar;
	}
	info->rar = rar;
	info->idx = i;
	show_message_fnc("%d: %s: %d -> %d\n", i, rh.FileName, rh.PackSize, rh.UnpSize);
	if (hash_define_str_value(hash, path, info) < 0)
	  warning("%s: %s: %s already in hash.\n", __FILE__, __FUNCTION__, path);
      }
      RARProcessFile(rar->r, RAR_SKIP, NULL, NULL);
    } else if (result == ERAR_END_ARCHIVE)
      break;
    else {
      err_message_fnc("RARReadHeader() returned %d\n", result);
      break;
    }
  }
  RARCloseArchive(rar->r);
  rar->r = NULL;
  rar->current_index = i;

  /* Don't sort */
  //dlist_set_compfunc(dl, archive_key_compare);
  //dlist_sort(dl);
  dlist_iter (dl, dd) {
    void *reminder = hash_lookup_str(hash, dlist_data(dd));

    if (!reminder) {
      warning("%s: %s: %s not in hash.\n", __FILE__, __FUNCTION__, (char *)dlist_data(dd));
    } else {
      Rar_info *info = reminder;
      debug_message("rar: %s: add %s as index %d\n", __FUNCTION__, (char *)dlist_data(dd), info->idx);
      archive_add(a, dlist_data(dd), (void *)info);
    }
  }
  hash_destroy(hash);
  dlist_destroy(dl);

  a->path = strdup(st->path);
  a->st = stream_transfer(st);
  a->open = rar_open;
  a->destroy = rar_destroy;

  return OPEN_OK;

 out_free_rar:
  if (hash)
    hash_destroy(hash);
  if (dl)
    dlist_destroy(dl);
  RARCloseArchive(rar->r);
  if (rar)
    free(rar);

  return res;
}
