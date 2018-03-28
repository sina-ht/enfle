/*
 * streamer-plugin.h -- streamer plugin interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Feb 10 00:03:28 2004.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _STREAMER_PLUGIN_H
#define _STREAMER_PLUGIN_H

#include "enfle-plugin.h"
#include "stream.h"

typedef enum {
  STREAM_ERROR = -1,
  STREAM_NOT,
  STREAM_OK
} StreamerStatus;

typedef struct _streamer_plugin {
  ENFLE_PLUGIN_COMMON_DATA;

  StreamerStatus (*identify)(Stream *, char *);
  StreamerStatus (*open)(Stream *, char *);
} StreamerPlugin;

#define DECLARE_STREAMER_PLUGIN_METHODS \
 static StreamerStatus identify(Stream *, char *); \
 static StreamerStatus open(Stream *, char *)

#define DEFINE_STREAMER_PLUGIN_IDENTIFY(st, p) \
 static StreamerStatus \
 identify(Stream * st , char * p )
#define DEFINE_STREAMER_PLUGIN_OPEN(st, p) \
 static StreamerStatus \
 open(Stream * st , char * p )

#ifndef STATIC
ENFLE_PLUGIN_ENTRIES;
#endif

#endif
