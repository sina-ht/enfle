/*
 * audio-plugin.h -- audio plugin interface header
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun May  1 17:00:17 2005.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _AUDIO_PLUGIN_H
#define _AUDIO_PLUGIN_H

#include "enfle-plugin.h"
#include "enfle/audio.h"

typedef struct _audio_plugin {
  ENFLE_PLUGIN_COMMON_DATA;

  AudioDevice *(*open_device)(void *, Config *);
  int (*set_params)(AudioDevice *, AudioFormat *, int *, unsigned int *);
  int (*write_device)(AudioDevice *, unsigned char *, int);
  int (*bytes_written)(AudioDevice *);
  int (*sync_device)(AudioDevice *);
  int (*close_device)(AudioDevice *);
} AudioPlugin;

#ifndef STATIC
ENFLE_PLUGIN_ENTRIES;
#endif

#endif
