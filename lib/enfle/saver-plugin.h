/*
 * saver-plugin.h -- Saver plugin interface header
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Feb 10 00:03:23 2004.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _SAVER_PLUGIN_H
#define _SAVER_PLUGIN_H

#include "enfle-plugin.h"
#include "utils/libconfig.h"
#include "image.h"

typedef struct _saver_plugin {
  ENFLE_PLUGIN_COMMON_DATA;

  char *(*get_ext)(Config *);
  int (*save)(Image *, FILE *, Config *, void *);
} SaverPlugin;

#define DECLARE_SAVER_PLUGIN_METHODS \
 static char *get_ext(Config *); \
 static int save(Image *, FILE *, Config *, void *)

#define DEFINE_SAVER_PLUGIN_GET_EXT(c) \
 static char * \
 get_ext(Config * c )
#define DEFINE_SAVER_PLUGIN_SAVE(p, fp, c, params) \
 static int \
 save(Image * p , FILE * fp , Config * c , void * params )

#ifndef STATIC
ENFLE_PLUGIN_ENTRIES;
#endif

#endif
