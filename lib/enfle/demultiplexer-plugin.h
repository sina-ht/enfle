/*
 * demultiplexer-plugin.h -- demultiplexer plugin interface header
 * (C)Copyright 2000-2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Feb 13 00:23:06 2004.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _DEMULTIPLEXER_PLUGIN_H
#define _DEMULTIPLEXER_PLUGIN_H

#include "utils/libconfig.h"
#include "enfle-plugin.h"
#include "enfle/demultiplexer.h"

typedef struct _demultiplexer_plugin {
  ENFLE_PLUGIN_COMMON_DATA;

  DemultiplexerStatus (*identify)(Stream *, Config *, void *);
  Demultiplexer *(*examine)(Movie *, Stream *, Config *, void *);
} DemultiplexerPlugin;

#define DECLARE_DEMULTIPLEXER_PLUGIN_METHODS \
 static DemultiplexerStatus identify(Stream *, Config *, void *); \
 static Demultiplexer *examine(Movie *, Stream *, Config *, void *)

#define DEFINE_DEMULTIPLEXER_PLUGIN_IDENTIFY(st, c, priv) \
 static DemultiplexerStatus \
 identify(Stream * st , Config * c , void * priv )
#define DEFINE_DEMULTIPLEXER_PLUGIN_EXAMINE(m, st, c, priv) \
 static Demultiplexer * \
 examine(Movie * m , Stream * st , Config * c , void * priv )

#ifndef STATIC
ENFLE_PLUGIN_ENTRIES;
#endif

#endif
