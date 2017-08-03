/*
 * effect-plugin.h -- Effect plugin interface header
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Feb 10 00:03:05 2004.
 *
 * Enfle is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Enfle is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
