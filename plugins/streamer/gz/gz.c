/*
 * gz.c -- gz streamer plugin
 * (C)Copyright 2000, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun May  1 16:45:44 2005.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include <stdio.h>
#include <stdlib.h>

#include <zlib.h>

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "enfle/streamer-plugin.h"
#include "utils/libstring.h"

DECLARE_STREAMER_PLUGIN_METHODS;

#define STREAMER_GZ_PLUGIN_DESCRIPTION "GZ Streamer plugin version 0.1.1"

static StreamerPlugin plugin = {
  .type = ENFLE_PLUGIN_STREAMER,
  .name = "GZ",
  .description = NULL,
  .author = "Hiroshi Takekawa",

  .identify = identify,
  .open = open
};

ENFLE_PLUGIN_ENTRY(streamer_gz)
{
  StreamerPlugin *stp;
  String *s;

  if ((stp = (StreamerPlugin *)calloc(1, sizeof(StreamerPlugin))) == NULL)
    return NULL;
  memcpy(stp, &plugin, sizeof(StreamerPlugin));
  s = string_create();
  string_set(s, STREAMER_GZ_PLUGIN_DESCRIPTION);
  /* 'compiled' means that the version string is compiled in. */
  string_catf(s, " with zlib %s(compiled with %s)", zlib_version, ZLIB_VERSION);
  stp->description = strdup(string_get(s));
  string_destroy(s);

  return (void *)stp;
}

#undef STREAMER_GZ_PLUGIN_DESCRIPTION

ENFLE_PLUGIN_EXIT(streamer_gz, p)
{
  StreamerPlugin *stp = (StreamerPlugin *)p;

  if (stp->description)
    free((unsigned char *)stp->description);
  free(stp);
}

/* implementations */

static int
__read(Stream *st, unsigned char *p, int size)
{
  gzFile gzfile;
  int r, err;

  gzfile = (gzFile)st->data;
  if ((r = gzread(gzfile, p, size)) < 0) {
    err_message("gz streamer plugin: read: %s\n", gzerror(gzfile, &err));
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
    fatal("Not yet implemented.\n");
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
__close(Stream *st)
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

DEFINE_STREAMER_PLUGIN_IDENTIFY(st __attribute__((unused)), filepath)
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

DEFINE_STREAMER_PLUGIN_OPEN(st
#if !defined(IDENTIFY_BEFORE_OPEN)
 __attribute__((unused))
#endif
, filepath)
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
  st->read = __read;
  st->seek = seek;
  st->tell = tell;
  st->close = __close;

  return STREAM_OK;
}
