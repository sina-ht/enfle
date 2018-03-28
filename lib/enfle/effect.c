/*
 * effect.c -- Effect plugin interface
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Jul  7 23:05:20 2002.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "effect.h"
#include "effect-plugin.h"

int
effect_call(EnflePlugins *eps, char *pluginname, Image *p, int src, int dst)
{
  Plugin *pl;
  EffectPlugin *ep;

  if ((pl = pluginlist_get(eps->pls[ENFLE_PLUGIN_EFFECT], pluginname)) == NULL)
    return 0;
  ep = plugin_get(pl);
  if (ep)
    return ep->effect(p, src, dst);
  return 0;
}
