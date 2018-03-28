/*
 * archiver-plugin.h -- archiver plugin interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Feb 10 00:02:54 2004.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _ARCHIVER_PLUGIN_H
#define _ARCHIVER_PLUGIN_H

#include "enfle-plugin.h"
#include "stream.h"
#include "archive.h"
#include "archiver-extra.h"

typedef struct _archiver_plugin {
  ENFLE_PLUGIN_COMMON_DATA;
  void *archiver_private;

  ArchiverStatus (*identify)(Archive *, Stream *, void *);
  ArchiverStatus (*open)(Archive *, Stream *, void *);
} ArchiverPlugin;

#define DECLARE_ARCHIVER_PLUGIN_METHODS \
 static ArchiverStatus identify(Archive *, Stream *, void *); \
 static ArchiverStatus open(Archive *, Stream *, void *)

#define DEFINE_ARCHIVER_PLUGIN_IDENTIFY(a, st, priv) \
 static ArchiverStatus \
 identify(Archive * a , Stream * st , void * priv )
#define DEFINE_ARCHIVER_PLUGIN_OPEN(a, st, priv) \
 static ArchiverStatus \
 open(Archive * a , Stream * st , void * priv )

#ifndef STATIC
ENFLE_PLUGIN_ENTRIES;
#endif

#endif
