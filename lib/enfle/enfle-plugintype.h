/*
 * enfle-plugintype.h -- enfle plugin type definition
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Dec 18 22:20:56 2000.
 * $Id: enfle-plugintype.h,v 1.4 2000/12/18 16:59:39 sian Exp $
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

#ifndef _ENFLE_PLUGINTYPE_H
#define _ENFLE_PLUGINTYPE_H

typedef enum {
  ENFLE_PLUGIN_UI,
  ENFLE_PLUGIN_VIDEO,
  ENFLE_PLUGIN_AUDIO,
  ENFLE_PLUGIN_LOADER,
  ENFLE_PLUGIN_SAVER,
  ENFLE_PLUGIN_PLAYER,
  ENFLE_PLUGIN_STREAMER,
  ENFLE_PLUGIN_ARCHIVER,
  ENFLE_PLUGIN_EFFECT,
  ENFLE_PLUGIN_END
} PluginType;

#endif
