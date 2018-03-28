/*
 * video-plugin.h -- video plugin interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Feb 10 00:03:37 2004.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _VIDEO_PLUGIN_H
#define _VIDEO_PLUGIN_H

#include "enfle-plugin.h"
#include "video.h"

typedef struct _video_plugin {
  ENFLE_PLUGIN_COMMON_DATA;

  void *(*open_video)(void *, Config *);
  int (*close_video)(void *);
  VideoWindow *(*get_root)(void *);
  VideoWindow *(*open_window)(void *, VideoWindow *, unsigned int, unsigned int);
  void (*set_wallpaper)(void *, Image *);
  void (*destroy)(void *);
} VideoPlugin;

#ifndef STATIC
ENFLE_PLUGIN_ENTRIES;
#endif

#endif
