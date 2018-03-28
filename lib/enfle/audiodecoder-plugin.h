/*
 * audiodecoder-plugin.h -- audio decoder plugin interface header
 * (C)Copyright 2000-2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Apr 10 17:45:14 2004.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _AUDIODECODER_PLUGIN_H
#define _AUDIODECODER_PLUGIN_H

#include "enfle-plugin.h"
#include "enfle/audiodecoder.h"

typedef struct _audiodecoder_plugin {
  ENFLE_PLUGIN_COMMON_DATA;
  void *ad_private;

  unsigned int (*query)(unsigned int, void *);
  AudioDecoder *(*init)(unsigned int, void *);
} AudioDecoderPlugin;

#define DECLARE_AUDIODECODER_PLUGIN_METHODS \
  static unsigned int query(unsigned int, void *); \
  static AudioDecoder *init(unsigned int, void *)

#ifndef STATIC
ENFLE_PLUGIN_ENTRIES;
#endif

#endif
