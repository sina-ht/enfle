/*
 * tar.c -- tar archiver plugin
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Oct 29 03:22:03 2000.
 * $Id: tar.c,v 1.2 2000/10/28 19:07:16 sian Exp $
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
#include <string.h>
#include <ctype.h>

#include "common.h"

#include "archiver-plugin.h"

#include "_tar.h"

DECLARE_ARCHIVER_PLUGIN_METHODS;

static ArchiverPlugin plugin = {
  type: ENFLE_PLUGIN_ARCHIVER,
  name: "TAR",
  description: "TAR Archiver plugin version 0.0.5",
  author: "Hiroshi Takekawa",

  identify: identify,
  open: open
};

void *
plugin_entry(void)
{
  ArchiverPlugin *arp;

  if ((arp = (ArchiverPlugin *)calloc(1, sizeof(ArchiverPlugin))) == NULL)
    return NULL;
  memcpy(arp, &plugin, sizeof(ArchiverPlugin));

  return (void *)arp;
}

void
plugin_exit(void *p)
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

  return (unsigned_sum == check_sum) || (signed_sum == check_sum);
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

  if (!stream_seek(arc->st, ti->offset, _SET) < 0) {
    show_message("tar_open: %s (at %ld %d bytes): seek failed.\n", ti->path, ti->offset, ti->size);
    return 0;
  }

  debug_message("tar_open: path %s offset %ld size %d\n", ti->path, ti->offset, ti->size);

  if ((p = calloc(1, ti->size)) == NULL) {
    show_message("tar_open: No enough memory.\n");
    return 0;
  }
  if ((have_read = stream_read(arc->st, p, ti->size)) != ti->size) {
    free(p);
    show_message("tar_open: No enough data(%d read, %d requested). Corrupted archive?\n", have_read, ti->size);
    return 0;
  }

  return stream_make_memorystream(st, p, ti->size);
}

static void /* overrides archive::destroy */
tar_destroy(Archive *arc)
{
  stream_destroy(arc->st);
  hash_destroy(arc->filehash, 1);
  free(arc);
};

/* methods */

#define READ_HEADER(p, st) (stream_read(st, (unsigned char *)p, sizeof(TarHeader)) == sizeof(TarHeader))

DEFINE_ARCHIVER_PLUGIN_IDENTIFY(a, st, priv)
{
  TarHeader th;

  if (!READ_HEADER(&th, st))
    return OPEN_NOT;
  if (!is_valid_header(&th))
    return OPEN_NOT;

  return OPEN_OK;
}

DEFINE_ARCHIVER_PLUGIN_OPEN(a, st, priv)
{
  TarHeader th;
  TarInfo *ti;
  unsigned int size, nrecord;

#ifdef IDENTIFY_BEFORE_OPEN
  if (identify(a, st, priv) == OPEN_NOT)
    return OPEN_NOT;
  stream_rewind(st);
#endif

  while (READ_HEADER(&th, st)) {
    if (th.name[0] == '\0')
      break;

    size = octal_to_value(th.size);

    switch ((int)th.linkflag) {
    case '\0':
    case '0':
      if ((ti = calloc(1, sizeof(TarInfo))) == NULL) {
	show_message("tar: open: No enough memory\n");
	return OPEN_ERROR;
      }
      ti->path = strdup(th.name);
      ti->size = size;
      ti->offset = stream_tell(st);
      debug_message("tar: add %s at %ld (%d bytes)\n", ti->path, ti->offset, ti->size);
      archive_add(a, th.name, (void *)ti);
      break;
    default:
      debug_message("tar: ignore %s\n", th.name);
      break;
    }
    if (size) {
      nrecord = size >> 9;
      if (size & 0x1ff)
	nrecord++;
      if (!stream_seek(st, nrecord << 9, _CUR)) {
	show_message("tar: open: Seek failed (name = %s, size = %d, nrecord = %d)\n",
		     th.name, size, nrecord);
	return OPEN_ERROR;
      }
    }
  }

  a->st = stream_transfer(st);
  a->open = tar_open;
  a->destroy = tar_destroy;

  return OPEN_OK;
}
