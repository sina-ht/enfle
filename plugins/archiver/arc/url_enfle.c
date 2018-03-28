/*
 * url_enfle.c -- libarc URL handler for enfle
 * (C)Copyright 2008 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Apr 13 00:54:06 2008.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "libarc/url.h"

#include "enfle/stream.h"
#include "enfle/stream-utils.h"

typedef struct _URL_enfle {
  char common[sizeof(struct _URL)];
  Stream *st;
} URL_enfle;

static int
url_check(char *s) {
  if (strncasecmp(s, "enfle:", 6) == 0)
    return 1;
  return 0;
}

static long
url_enfle_read(URL url, void *buff, long n) {
  URL_enfle *urlp = (URL_enfle *)url;

  return stream_read(urlp->st, buff, n);
}

static char *
url_enfle_gets(URL url, char *buff, int n) {
  URL_enfle *urlp = (URL_enfle *)url;

  return stream_ngets(urlp->st, buff, n);
}

static int
url_enfle_fgetc(URL url) {
  URL_enfle *urlp = (URL_enfle *)url;
  int c;

  if ((c = stream_getc(urlp->st)) == -1)
    return EOF;
  return c;
}

static void
url_enfle_close(URL url) {
  URL_enfle *urlp = (URL_enfle *)url;

  stream_rewind(urlp->st);
  free(url);
}

static long
url_enfle_seek(URL url, long offset, int whence) {
  URL_enfle *urlp = (URL_enfle *)url;

  return stream_seek(urlp->st, offset, whence);
}

static long
url_enfle_tell(URL url) {
  URL_enfle *urlp = (URL_enfle *)url;

  return stream_tell(urlp->st);
}

URL
url_enfle_open(char *urlstr)
{
  URL_enfle *url;

  if ((url = (URL_enfle *)alloc_url(sizeof(URL_enfle))) == NULL)
    return NULL;

  /* common members */
  URLm(url, type)      = URL_extension_t;
  URLm(url, url_read)  = url_enfle_read;
  URLm(url, url_gets)  = url_enfle_gets;
  URLm(url, url_fgetc) = url_enfle_fgetc;
  URLm(url, url_seek)  = url_enfle_seek;
  URLm(url, url_tell)  = url_enfle_tell;
  URLm(url, url_close) = url_enfle_close;

  /* private members */
  sscanf(urlstr, "enfle:%p", &url->st);
  debug_message_fnc("st = %p\n", url->st);

  return (URL)url;
}

struct URL_module URL_module_enfle = {
  URL_extension_t,  /* type */
  url_check,        /* URL checker */
  NULL,             /* initializer */
  url_enfle_open,   /* open */
  NULL              /* must be NULL */
};
