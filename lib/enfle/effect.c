/*
 * effect.c -- Effect plugin interface
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Jul  7 23:05:20 2002.
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
