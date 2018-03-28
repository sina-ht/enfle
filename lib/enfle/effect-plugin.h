/*
 * effect-plugin.h -- Effect plugin interface header
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Feb 10 00:03:05 2004.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _EFFECT_PLUGIN_H
#define _EFFECT_PLUGIN_H

#include "enfle-plugin.h"
#include "ui-action.h"
#include "utils/libconfig.h"
#include "image.h"

typedef struct _effect_plugin {
  ENFLE_PLUGIN_COMMON_DATA;

  UIAction *actions;
  int (*effect)(Image *, int, int);
  void *priv;
} EffectPlugin;

#ifndef STATIC
ENFLE_PLUGIN_ENTRIES;
#endif

#endif
