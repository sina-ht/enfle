/*
 * bz2.c -- bz2 streamer plugin
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Feb 18 03:28:43 2002.
 * $Id: bz2.c,v 1.5 2002/02/17 19:32:56 sian Exp $
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
#include "utils/libstring.h"

#include "bz2.h"

DECLARE_STREAMER_PLUGIN_METHODS;

#define STREAMER_BZ2_PLUGIN_DESCRIPTION "BZ2 Streamer plugin version 0.0.5"

static StreamerPlugin plugin = {
  type: ENFLE_PLUGIN_STREAMER,
  name: "BZ2",
  description: NULL,
  author: "Hiroshi Takekawa",

  identify: identify,
  open: open
};

void *
plugin_entry(void)
{
  StreamerPlugin *stp;
  String *s;

  if ((stp = (StreamerPlugin *)calloc(1, sizeof(StreamerPlugin))) == NULL)
    return NULL;
  memcpy(stp, &plugin, sizeof(StreamerPlugin));
  s = string_create();
  string_set(s, STREAMER_BZ2_PLUGIN_DESCRIPTION);
  /* The version string is fetched dynamically, not statically compiled-in. */
  string_catf(s, " with libbz2 %s", BZLIBVERSION());
  stp->description = (const unsigned char *)strdup(string_get(s));
  string_destroy(s);

  return (void *)stp;
}

#undef STREAMER_BZ2_PLUGIN_DESCRIPTION

void
plugin_exit(void *p)
{
  StreamerPlugin *stp = (StreamerPlugin *)p;

  if (stp->description)
    free((unsigned char *)stp->description);
  free(stp);
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

DEFINE_STREAMER_PLUGIN_IDENTIFY(st, filepath)
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

DEFINE_STREAMER_PLUGIN_OPEN(st, filepath)
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
