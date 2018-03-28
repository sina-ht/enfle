/*
 * xz.c -- xz streamer plugin
 * (C)Copyright 2009 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Nov 18 23:04:30 2011.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include <stdio.h>
#include <stdlib.h>

#include <lzma.h>

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "enfle/streamer-plugin.h"
#include "utils/libstring.h"

DECLARE_STREAMER_PLUGIN_METHODS;

#define STREAMER_XZ_PLUGIN_DESCRIPTION "XZ Streamer plugin version 0.1"

static StreamerPlugin plugin = {
  .type = ENFLE_PLUGIN_STREAMER,
  .name = "XZ",
  .description = NULL,
  .author = "Hiroshi Takekawa",

  .identify = identify,
  .open = open
};

ENFLE_PLUGIN_ENTRY(streamer_xz)
{
  StreamerPlugin *stp;
  String *s;

  if ((stp = (StreamerPlugin *)calloc(1, sizeof(StreamerPlugin))) == NULL)
    return NULL;
  memcpy(stp, &plugin, sizeof(StreamerPlugin));
  s = string_create();
  string_set(s, STREAMER_XZ_PLUGIN_DESCRIPTION);
  /* 'compiled' means that the version string is compiled in. */
  string_catf(s, " with liblzma %s(compiled with %s)", lzma_version_string(), LZMA_VERSION_STRING);
  stp->description = strdup(string_get(s));
  string_destroy(s);

  return (void *)stp;
}

#undef STREAMER_XZ_PLUGIN_DESCRIPTION

ENFLE_PLUGIN_EXIT(streamer_xz, p)
{
  StreamerPlugin *stp = (StreamerPlugin *)p;

  if (stp->description)
    free((unsigned char *)stp->description);
  free(stp);
}

/* liblzma related */

#define XZ_BUF_SIZE (1024 * 1024)
#define XZ_MEM_LIMIT (128 * 1024 * 1024)

struct xz_priv {
  FILE *fp;
  lzma_stream strm;
  long pos;
  long size;
  unsigned char buf[XZ_BUF_SIZE];
  lzma_action action;
};

static lzma_ret
init_lzma(lzma_stream *strm)
{
  lzma_stream strm_init = LZMA_STREAM_INIT;

  *strm = strm_init;
  return lzma_stream_decoder(strm, XZ_MEM_LIMIT,
			     LZMA_TELL_UNSUPPORTED_CHECK | LZMA_CONCATENATED);
}

/* implementations */

static int
__read(Stream *st, unsigned char *p, int size)
{
  struct xz_priv *xz = st->data;
  lzma_ret ret;

  xz->strm.next_out = p;
  xz->strm.avail_out = size;

  if (xz->strm.avail_in == 0 && !feof(xz->fp)) {
    xz->strm.next_in = xz->buf;
    xz->strm.avail_in = fread(xz->buf, 1, XZ_BUF_SIZE, xz->fp);

    if (xz->strm.avail_in == SIZE_MAX) {
      err_message("xz streamer plugin: read: fread() error\n");
      return -1;
    }

    if (feof(xz->fp))
      xz->action = LZMA_FINISH;
  }

  ret = lzma_code(&xz->strm, xz->action);
  switch (ret) {
  case LZMA_OK:
  case LZMA_STREAM_END:
    break;
  case LZMA_MEMLIMIT_ERROR:
    err_message("xz streamer plugin: read: lzma_code() error: MEMLIMIT_ERROR %ld > %d\n", lzma_memusage(&xz->strm), XZ_MEM_LIMIT);
    return -1;
  default:
    err_message("xz streamer plugin: read: lzma_code() error: %d\n", ret);
    return -1;
  }
  xz->pos += size - xz->strm.avail_out;

  return size - xz->strm.avail_out;
}

static void
__rewind(Stream *st)
{
  struct xz_priv *xz = st->data;

  rewind(xz->fp);
  lzma_end(&xz->strm);
  init_lzma(&xz->strm);
  xz->action = LZMA_RUN;
  xz->pos = 0;
}

static int
seek(Stream *st, long offset, StreamWhence whence)
{
  struct xz_priv *xz = st->data;
  long skip_to = -1;
  long size, read_size;
  unsigned char dummy[XZ_BUF_SIZE];

  switch (whence) {
  case _SET:
    skip_to = offset;
    break;
  case _CUR:
    skip_to = xz->pos + offset;
    break;
  case _END:
    if (xz->size == -1) {
      do {
	read_size = __read(st, dummy, XZ_BUF_SIZE);
	if (read_size < 0) {
	  err_message_fnc("read error.\n");
	  return -1;
	}
	xz->pos += read_size;
      } while (read_size == XZ_BUF_SIZE);
      xz->size = xz->pos;
    }
    skip_to = xz->size + offset;
    break;
  }

  if (skip_to == xz->pos)
      return 0;
  if (skip_to < xz->pos) {
      __rewind(st);
      if (skip_to == 0)
	return 0;
  }

  show_message_fnc("skip_to = %ld\n", skip_to);
  for (; xz->pos < skip_to;) {
    size = skip_to - xz->pos;
    if (size > XZ_BUF_SIZE)
      size = XZ_BUF_SIZE;

    read_size = __read(st, dummy, size);
    if (read_size < 0) {
      err_message_fnc("read error.\n");
      return -1;
    }
    xz->pos += read_size;
  }

  return 0;
}

static long
tell(Stream *st)
{
  struct xz_priv *xz = st->data;

  return xz->pos;
}

static int
__close(Stream *st)
{
  int f;

  if (st->data) {
    struct xz_priv *xz = st->data;

    f = fclose(xz->fp);
    lzma_end(&xz->strm);
    free(xz);
    st->data = NULL;
    return (f == 0) ? 1 : 0;
  }

  return 1;
}

/* methods */

DEFINE_STREAMER_PLUGIN_IDENTIFY(st __attribute__((unused)), filepath)
{
  FILE *fp;
  unsigned char buf[6];
  static unsigned char id[] = { 0xfd, 0x37, 0x7a, 0x58, 0x5a, 0x00 };

  if ((fp = fopen(filepath, "rb")) == NULL)
    return STREAM_ERROR;
  if (fread(buf, 1, 6, fp) != 6) {
    fclose(fp);
    return STREAM_NOT;
  }
  fclose(fp);
  if (memcmp(buf, id, 6))
    return STREAM_NOT;

  return STREAM_OK;
}

DEFINE_STREAMER_PLUGIN_OPEN(st, filepath)
{
  struct xz_priv *xz;

#ifdef IDENTIFY_BEFORE_OPEN
  {
    StreamerStatus stst;

    if ((stst = identify(st, filepath)) != STREAM_OK)
      return stst;
  }
#endif

  if ((xz = calloc(1, sizeof(*xz))) == NULL)
    return STREAM_ERROR;

  init_lzma(&xz->strm);
  xz->action = LZMA_RUN;
  xz->pos = 0;
  xz->size = -1;

  if ((xz->fp = fopen(filepath, "rb")) == NULL) {
    free(xz);
    return STREAM_ERROR;
  }
  st->data = (void *)xz;
  st->read = __read;
  st->seek = seek;
  st->tell = tell;
  st->close = __close;

  return STREAM_OK;
}
