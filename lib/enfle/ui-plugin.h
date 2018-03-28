/*
 * ui-plugin.h -- UI plugin interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Feb 10 00:03:33 2004.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _UI_PLUGIN_H
#define _UI_PLUGIN_H

#include "ui-extra.h"
#include "enfle-plugin.h"
#include "utils/libconfig.h"

typedef struct _ui_plugin {
  ENFLE_PLUGIN_COMMON_DATA;

  int (*ui_main)(UIData *);
} UIPlugin;

#ifndef STATIC
ENFLE_PLUGIN_ENTRIES;
#endif

#endif
