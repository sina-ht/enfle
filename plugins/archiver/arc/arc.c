/*
 * arc.c -- libarc archiver plugin
 * (C)Copyright 2002 by Junji Hashimoto
 * Adapted for newer version by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Mar  6 12:08:40 2004.
 * $Id: arc.c,v 1.4 2004/03/06 03:43:36 sian Exp $
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

#include <libarc/arc.h>

extern struct URL_module URL_module_file;

#include "common.h"

#include "enfle/archiver-plugin.h"

DECLARE_ARCHIVER_PLUGIN_METHODS;

static ArchiverPlugin plugin = {
  .type = ENFLE_PLUGIN_ARCHIVER,
  .name = "ARC",
  .description = "libarc Archiver plugin version 0.0.8 compiled with libarc-" ARC_LIB_VERSION,
  .author = "Junji Hashimoto",
  .archiver_private = NULL,

  .identify = identify,
  .open = open
};

ENFLE_PLUGIN_ENTRY(archiver_arc)
{
  ArchiverPlugin *arp;

  url_add_module(&URL_module_file);
  if ((arp = (ArchiverPlugin *)calloc(1, sizeof(ArchiverPlugin))) == NULL)
    return NULL;
  memcpy(arp, &plugin, sizeof(ArchiverPlugin));

  return (void *)arp;
}

ENFLE_PLUGIN_EXIT(archiver_arc, p)
{
  free(p);
}

static int /* overrides archive::open */
arc_open(Archive *arc, Stream *st, char *path)
{
  int file_size = 0, get_num, mem_size = 0;
  unsigned char *region, *read_pos;
  URL url;
  char fullpath[strlen(arc->path) + strlen(path) + 1];

  if (strchr(path, '#') == NULL) {
    /* normal file */
    return OPEN_NOT;
  }

  strcpy(fullpath, arc->path);
  strcat(fullpath, path);
  url = url_arc_open(fullpath);
  if(url == NULL){
    err_message("Can't open: %s\n", fullpath);
    return OPEN_NOT;
  }

  mem_size = BUFSIZ * 2;
  region = read_pos = (unsigned char *)malloc(mem_size);
  while ((get_num = url_read(url, read_pos, BUFSIZ)) > 0) {
    file_size += get_num;
    read_pos += get_num;
    if ((mem_size - file_size) < BUFSIZ) {
      mem_size *= 2;
      region = (unsigned char *)realloc(region, mem_size);
      read_pos = region + file_size;
    }
  }
  url_close(url);

  st->path = strdup(path);
  //debug_message_fnc("OK: %s\n", st->path);

  return stream_make_memorystream(st, region, file_size);
}

static void /* overrides archive::destroy */
arc_destroy(Archive *arc)
{
  debug_message_fnc("%s\n", arc->st->path);
  stream_destroy(arc->st);
  hash_destroy(arc->filehash);
  if (arc->path)
    free(arc->path);
  free(arc);
}

/* methods */
DEFINE_ARCHIVER_PLUGIN_IDENTIFY(a, st, priv)
{
  // debug_message_fnc("%s\n", st->path);
  if (strchr(st->path, '#') != NULL) {
    return OPEN_NOT;
  }

  if (get_archive_type(st->path) < 0)
    return OPEN_NOT;
  return OPEN_OK;
}

DEFINE_ARCHIVER_PLUGIN_OPEN(a, st, priv)
{
  char *filesbuf[2];
  char **files;
  int i, nfiles = 1;
  Dlist *dl;
  Dlist_data *dd;

  filesbuf[0] = st->path;
  filesbuf[1] = NULL;
  files = expand_archive_names(&nfiles, filesbuf);
  if (files == NULL)
    return OPEN_NOT;
  //debug_message_fnc("path = %s\n",st->path);

  dl = dlist_create();
  for (i = 0; i < nfiles; i++) {
    char *base = files[i] + strlen(st->path);
    //debug_message_fnc("%d: %s\n", i, base);
    dlist_add_str(dl, base);
  }
  arc_list_free(files);

  dlist_set_compfunc(dl, archive_key_compare);
  dlist_sort(dl);
  dlist_iter (dl, dd) {
    archive_add(a, dlist_data(dd), strdup(dlist_data(dd)));
  }
  dlist_destroy(dl);

  a->path = strdup(st->path);
  a->st = stream_transfer(st);
  a->open = arc_open;
  a->destroy = arc_destroy;

  return OPEN_OK;
}
