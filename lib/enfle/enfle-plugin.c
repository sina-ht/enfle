/*
 * enfle-plugin.c -- enfle plugin interface
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Mar  9 22:15:09 2004.
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

#include <stdlib.h>

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "enfle-plugin.h"

static const char *plugintype_to_name[] = {
  "ui", "video", "audio", "loader", "saver", "player", "streamer", "archiver", "effect", "audiodecoder", "videodecoder", "demultiplexer", "end"
};

const char *
enfle_plugin_type_to_name(PluginType type)
{
  if (type < ENFLE_PLUGIN_START || type >= ENFLE_PLUGIN_END)
    return NULL;
  return plugintype_to_name[type];
}

PluginType
enfle_plugin_name_to_type(char *name)
{
  int i;

  for (i = ENFLE_PLUGIN_START; i < ENFLE_PLUGIN_END; i++) {
    if (strcasecmp(name, plugintype_to_name[i]) == 0)
      return i;
  }
  return ENFLE_PLUGIN_INVALID;
}
