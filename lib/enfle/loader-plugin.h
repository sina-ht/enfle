/*
 * loader-plugin.h -- loader plugin interface header
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Jun 23 15:17:42 2002.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _LOADER_PLUGIN_H
#define _LOADER_PLUGIN_H

#include "enfle-plugin.h"
#include "stream.h"
#include "image.h"
#include "video.h"
#include "utils/libconfig.h"
#include "loader-extra.h"

typedef struct _loader_plugin {
  ENFLE_PLUGIN_COMMON_DATA;
  void *image_private;

  LoaderStatus (*identify)(Image *, Stream *, VideoWindow *, Config *, void *);
  LoaderStatus (*load)(Image *, Stream *, VideoWindow *, Config *, void *);
} LoaderPlugin;

#define DECLARE_LOADER_PLUGIN_METHODS \
 static LoaderStatus identify(Image *, Stream *, VideoWindow *, Config *, void *); \
 static LoaderStatus load(Image *, Stream *, VideoWindow *, Config *, void *)

#define DEFINE_LOADER_PLUGIN_IDENTIFY(p, st, vw, c, priv) \
 static LoaderStatus \
 identify(Image * p , Stream * st , VideoWindow * vw , Config * c , void * priv )
#define DEFINE_LOADER_PLUGIN_LOAD(p, st, vw, c, priv) \
 static LoaderStatus \
 load(Image * p , Stream * st , VideoWindow * vw , Config * c , void * priv )

#ifndef STATIC
ENFLE_PLUGIN_ENTRIES;
#endif

#endif
