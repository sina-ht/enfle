/*
 * gz.c -- gz streamer plugin
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Thu Dec 28 21:59:58 2000.
 * $Id: gz.c,v 1.3 2000/12/30 07:19:56 sian Exp $
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

#include <zlib.h>

#define REQUIRE_FATAL
#include "common.h"

#include "streamer-plugin.h"

static StreamerStatus identify(Stream *, char *);
static StreamerStatus open(Stream *, char *);

static StreamerPlugin plugin = {
  type: ENFLE_PLUGIN_STREAMER,
  name: "GZ",
  description: "GZ Streamer plugin version 0.1",
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

/* implementations */

static int
read(Stream *st, unsigned char *p, int size)
{
  gzFile gzfile;
  int r, err;

  gzfile = (gzFile)st->data;
  if ((r = gzread(gzfile, p, size)) < 0) {
    fprintf(stderr, "gz streamer plugin: read: %s\n", gzerror(gzfile, &err));
    return -1;
  }

  return r;
}

static int
seek(Stream *st, long offset, StreamWhence whence)
{
  switch (whence) {
  case _SET:
    return (gzseek((gzFile)st->data, offset, SEEK_SET) == -1) ? 0 : 1;
    break;
  case _CUR:
    return (gzseek((gzFile)st->data, offset, SEEK_CUR) == -1) ? 0 : 1;
    break;
  case _END:
    fatal(1, "Not yet implemented.\n");
  }
  /* never reached */
  return -1;
}

static long
tell(Stream *st)
{
  return gztell((gzFile)st->data);
}

static int
close(Stream *st)
{
  int f;

  if (st->data) {
    f = gzclose((gzFile)st->data);
    st->data = NULL;
    return (f == 0) ? 1 : 0;
  }

  return 1;
}

/* methods */

static StreamerStatus
identify(Stream *st, char *filepath)
{
  FILE *fp;
  unsigned char buf[2];
  static unsigned char id[] = { 0x1f, 0x8b };

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
  gzFile gzfile;

#ifdef IDENTIFY_BEFORE_OPEN
  {
    StreamerStatus stst;

    if ((stst = identify(st, filepath)) != STREAM_OK)
      return stst;
  }
#endif

  if ((gzfile = gzopen(filepath, "rb")) == NULL)
    return STREAM_NOT;
  st->data = (void *)gzfile;
  st->read = read;
  st->seek = seek;
  st->tell = tell;
  st->close = close;

  return STREAM_OK;
}
