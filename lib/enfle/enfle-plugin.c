/*
 * enfle-plugin.c -- enfle plugin interface
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Dec 24 07:44:46 2000.
 * $Id: enfle-plugin.c,v 1.4 2000/12/24 15:27:45 sian Exp $
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <stdlib.h>

#include "common.h"

#include "enfle-plugin.h"

static const char *plugintype_to_name[] = {
  "ui", "video", "audio", "loader", "saver", "player", "streamer", "archiver", "effect", "end"
};

const char *
enfle_plugin_type_to_name(PluginType type)
{
  return plugintype_to_name[type];
}
