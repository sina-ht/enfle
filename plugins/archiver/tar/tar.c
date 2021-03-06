/*
 * tar.c -- tar archiver plugin
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Wed Apr 27 21:26:49 2016.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "enfle/archiver-plugin.h"

#include "_tar.h"

DECLARE_ARCHIVER_PLUGIN_METHODS;

static ArchiverPlugin plugin = {
  .type = ENFLE_PLUGIN_ARCHIVER,
  .name = "TAR",
  .description = "TAR Archiver plugin version 0.2",
  .author = "Hiroshi Takekawa",
  .archiver_private = NULL,

  .identify = identify,
  .open = open
};

ENFLE_PLUGIN_ENTRY(archiver_tar)
{
  ArchiverPlugin *arp;

  if ((arp = (ArchiverPlugin *)calloc(1, sizeof(ArchiverPlugin))) == NULL)
    return NULL;
  memcpy(arp, &plugin, sizeof(ArchiverPlugin));

  return (void *)arp;
}

ENFLE_PLUGIN_EXIT(archiver_tar, p)
{
  free(p);
}

/* for internal use */

static unsigned int
octal_to_value(char *s)
{
  unsigned int value;
  int f;

  /* for fail safe. */
  while (isspace((int)*s))
    s++;

  value = 0;
  f = 1;
  do {
    switch ((int)*s) {
    case '0':
      value <<= 3;
      break;
    case '1':
      value = (value << 3) | 1;
      break;
    case '2':
      value = (value << 3) | 2;
      break;
    case '3':
      value = (value << 3) | 3;
      break;
    case '4':
      value = (value << 3) | 4;
      break;
    case '5':
      value = (value << 3) | 5;
      break;
    case '6':
      value = (value << 3) | 6;
      break;
    case '7':
      value = (value << 3) | 7;
      break;
    default:
      f = 0;
      break;
    }
    s++;
  } while (f);

  return value;
}

static int
is_valid_header(TarHeader *th)
{
  long signed_sum;
  unsigned long unsigned_sum, check_sum;
  char *p;
  int i;

  if (memcmp(th->magic, "ustar", 5) != 0)
    return 0;

  check_sum = octal_to_value(th->checksum);
  signed_sum = unsigned_sum = 0;
  for (p = (char *)th, i = sizeof(TarHeader); i > 0; i--, p++) {
    unsigned_sum += *p & 0xff;
    signed_sum   += *p;
  }
  /* adjusting */
  for (i = sizeof(th->checksum) - 1; i >= 0; i--) {
    unsigned_sum -= th->checksum[i] & 0xff;
    signed_sum   -= th->checksum[i];
  }
  unsigned_sum += ' ' * 8;
  signed_sum   += ' ' * 8;

  return (unsigned_sum == check_sum) || (signed_sum == (long)check_sum);
}

/* override methods */

static int /* overrides archive::open */
tar_open(Archive *arc, Stream *st, char *path)
{
  TarInfo *ti;
  unsigned char *p;
  unsigned int have_read;

  if ((ti = (TarInfo *)archive_get(arc, path)) == NULL)
    return 0;

  if (stream_seek(arc->st, ti->offset, _SET) < 0) {
    show_message_fnc("%s (at %ld %d bytes): seek failed.\n", ti->path, ti->offset, ti->size);
    return 0;
  }

  debug_message_fnc("path %s offset %ld size %d\n", ti->path, ti->offset, ti->size);

  if ((p = calloc(1, ti->size)) == NULL) {
    show_message_fnc("No enough memory.\n");
    return 0;
  }
  if ((have_read = stream_read(arc->st, p, ti->size)) != ti->size) {
    free(p);
    show_message_fnc("No enough data(%d read, %d requested). Corrupted archive?\n", have_read, ti->size);
    return 0;
  }

  st->path = strdup(ti->path);
  return stream_make_memorystream(st, p, ti->size);
}

static void /* overrides archive::destroy */
tar_destroy(Archive *arc)
{
  stream_destroy(arc->st);
  hash_destroy(arc->filehash);
  if (arc->path)
    free(arc->path);
  free(arc);
}

/* methods */

#define READ_HEADER(p, st) (stream_read(st, (unsigned char *)p, sizeof(TarHeader)) == sizeof(TarHeader))

DEFINE_ARCHIVER_PLUGIN_IDENTIFY(a __attribute__((unused)), st, priv __attribute__((unused)))
{
  TarHeader th;

  if (!READ_HEADER(&th, st))
    return OPEN_NOT;
  if (!is_valid_header(&th))
    return OPEN_NOT;

  return OPEN_OK;
}

DEFINE_ARCHIVER_PLUGIN_OPEN(a, st, priv __attribute__((unused)))
{
  TarHeader th;
  TarInfo *ti = NULL;
  unsigned int size, nrecord;
  Dlist *dl;
  Dlist_data *dd;
  Hash *hash;

#ifdef IDENTIFY_BEFORE_OPEN
  if (identify(a, st, priv) == OPEN_NOT)
    return OPEN_NOT;
  stream_rewind(st);
#endif

  dl = dlist_create();
  hash = hash_create(ARCHIVE_FILEHASH_SIZE);
  while (READ_HEADER(&th, st)) {
    if (th.name[0] == '\0')
      break;

    size = octal_to_value(th.size);

    switch ((int)th.linkflag) {
    case '\0':
    case '0':
      if ((ti = calloc(1, sizeof(TarInfo))) == NULL) {
	show_message("tar: %s: No enough memory\n", __FUNCTION__);
	return OPEN_ERROR;
      }
      ti->path = calloc(1, strlen(th.name) + 2);
      ti->path[0] = '#';
      strcat(ti->path, th.name);
      ti->size = size;
      ti->offset = stream_tell(st);
      dlist_add_str(dl, ti->path);
      if (hash_define_str_value(hash, ti->path, ti) < 0) {
	warning("%s: %s: %s already in hash.\n", __FILE__, __FUNCTION__, ti->path);
      }
      break;
    default:
      debug_message("tar: %s: ignore %s\n", __FUNCTION__, th.name);
      break;
    }
    if (size) {
      nrecord = size >> 9;
      if (size & 0x1ff)
	nrecord++;
      if (!stream_seek(st, nrecord << 9, _CUR)) {
	show_message("tar: %s: Seek failed (name = %s, size = %d, nrecord = %d)\n", __FUNCTION__,
		     th.name, size, nrecord);
	return OPEN_ERROR;
      }
    }
  }

  dlist_set_compfunc(dl, archive_key_compare);
  dlist_sort(dl);
  dlist_iter (dl, dd) {
    void *remainder = hash_lookup_str(hash, dlist_data(dd));

    if (!remainder) {
      warning("%s: %s: %s not in hash.\n", __FILE__, __FUNCTION__, (char *)dlist_data(dd));
    } else {
#if defined(DEBUG)
      TarInfo *ti = remainder;
      debug_message("tar: %s: add %s at %ld (%d bytes)\n", __FUNCTION__, ti->path, ti->offset, ti->size);
#endif
      archive_add(a, dlist_data(dd), remainder);
    }
  }
  hash_destroy(hash);
  dlist_destroy(dl);

  a->path = strdup(st->path);
  a->st = stream_transfer(st);
  a->open = tar_open;
  a->destroy = tar_destroy;

  return OPEN_OK;
}
