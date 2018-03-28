/*
 * ui-extra.h -- UI plugin extra definition
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Jun  6 21:26:16 2009.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _UI_EXTRA_H
#define _UI_EXTRA_H

typedef struct _ui_data UIData;

#include "utils/libconfig.h"
#include "utils/cache.h"
#include "enfle-plugins.h"
#include "archive.h"
#include "video.h"
#include "video-plugin.h"
#include "audio-plugin.h"

struct _ui_data {
  Config *c;
  Cache *cache;
  VideoPlugin *vp;
  VideoKey key;
  AudioPlugin *ap;
  VideoWindow *vw;
  Archive *a;
  void *disp;
  void *priv;
  int nth;
  int minw, minh;
  int if_readdir;
  int if_slideshow;
  int slide_interval;
  char *path;
};

#endif
