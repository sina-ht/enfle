/*
 * bz2.c -- bz2 streamer plugin
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Jun 23 15:27:03 2001.
 * $Id: bz2.c,v 1.3 2001/06/24 15:41:36 sian Exp $
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

#include <bzlib.h>

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "enfle/streamer-plugin.h"

#include "bz2.h"

static StreamerStatus identify(Stream *, char *);
static StreamerStatus open(Stream *, char *);

static StreamerPlugin plugin = {
  type: ENFLE_PLUGIN_STREAMER,
  name: "BZ2",
  description: "BZ2 Streamer plugin version 0.0.5",
  author: "Hiroshi Takekawa",

  identify: identify,
  open: open
};

void *
plugin_entry(void)
{
  StreamerPlugin *stp;

  if ((stp = (StreamerPlugin *)calloc(1, sizeof(StreamerPlugin))) == NULL)
    return NULL;
  memcpy(stp, &plugin, sizeof(StreamerPlugin));

  return (void *)stp;
}

void
plugin_exit(void *p)
{
  free(p);
}

/* for internal use */

#define BZSEEK_BUFFER_SIZE 8192

static int
bzseek_set(Stream *st, long pos)
{
  unsigned char buf[BZSEEK_BUFFER_SIZE];
  int bytes_to_read;

  if (st->ptr_offset < pos) {
    bytes_to_read = pos - st->ptr_offset;
  } else {
    BZCLOSE((BZFILE *)st->data);
    if ((st->data = (void *)BZOPEN(st->path, "rb")) == NULL)
      return -1;
    st->ptr_offset = 0;
    bytes_to_read = pos;
  }
  while (bytes_to_read > BZSEEK_BUFFER_SIZE) {
    if (BZREAD((BZFILE *)st->data, buf, BZSEEK_BUFFER_SIZE) < 0)
      return -1;
    st->ptr_offset += BZSEEK_BUFFER_SIZE;
    bytes_to_read -= BZSEEK_BUFFER_SIZE;
  }
  if (BZREAD((BZFILE *)st->data, buf, bytes_to_read) < 0)
    return -1;
  st->ptr_offset += bytes_to_read;

  return st->ptr_offset;
}

/* implementations */

static int
read(Stream *st, unsigned char *p, int size)
{
  BZFILE *bzfile;
  int r, err;

  bzfile = (BZFILE *)st->data;
  if ((r = BZREAD(bzfile, p, size)) < 0) {
    fprintf(stderr, "bz2 streamer plugin: read: %s\n", BZERROR(bzfile, &err));
    return -1;
  }
  st->ptr_offset += r;

  return r;
}

static int
seek(Stream *st, long pos, StreamWhence whence)
{
  switch (whence) {
  case _SET:
    return bzseek_set(st, pos);
  case _CUR:
    return bzseek_set(st, st->ptr_offset + pos);
  case _END:
    show_message("bz2 streamer: seek: _END cannot be implemented.\n");
    return 0;
  default:
    show_message("bz2 streamer: seek: Unknown whence %d\n", whence);
    return -1;
  }
}

static long
tell(Stream *st)
{
  return st->ptr_offset;
}

static int
close(Stream *st)
{
  if (st->data) {
    BZCLOSE((BZFILE *)st->data);
    st->data = NULL;
  }

  return 1;
}

/* methods */

static StreamerStatus
identify(Stream *st, char *filepath)
{
  FILE *fp;
  unsigned char buf[2];
  static unsigned char id[] = { 'B', 'Z' };

  if ((fp = fopen(filepath, "rb")) == NULL)
    return STREAM_ERROR;
  if (fread(buf, 1, 2, fp) != 2) {
    fclose(fp);
    return STREAM_NOT;
  }
  fclose(fp);
  if (memcmp(buf, id, 2))
    return STREAM_NOT;

  return STREAM_OK;
}

static StreamerStatus
open(Stream *st, char *filepath)
{
  BZFILE *bzfile;

#ifdef IDENTIFY_BEFORE_OPEN
  {
    StreamerStatus stst;

    if ((stst = identify(st, filepath)) != STREAM_OK)
      return stst;
  }
#endif

  if ((bzfile = BZOPEN(filepath, "rb")) == NULL)
    return STREAM_NOT;
  st->data = (void *)bzfile;
  st->ptr_offset = 0;
  st->read = read;
  st->seek = seek;
  st->tell = tell;
  st->close = close;

  return STREAM_OK;
}
