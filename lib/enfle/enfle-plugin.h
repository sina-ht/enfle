/*
 * enfle-plugin.h -- enfle plugin interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Sep 18 07:16:50 2000.
 * $Id: enfle-plugin.h,v 1.1 2000/09/30 17:36:36 sian Exp $
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

#ifndef _ENFLE_PLUGIN_H
#define _ENFLE_PLUGIN_H

typedef enum {
  ENFLE_PLUGIN_UI,
  ENFLE_PLUGIN_LOADER,
  ENFLE_PLUGIN_STREAMER,
  ENFLE_PLUGIN_ARCHIVER,
  ENFLE_PLUGIN_SAVER,
  ENFLE_PLUGIN_EFFECT,
  ENFLE_PLUGIN_PLAYER
} PluginType;

#define ENFLE_PLUGIN_COMMON_DATA \
  PluginType type; \
  char *name; \
  unsigned char *description; \
  unsigned char *author

typedef struct _enfle_plugin {
  ENFLE_PLUGIN_COMMON_DATA;
} EnflePlugin;

#define ENFLE_PLUGIN_ENTRIES \
  void *plugin_entry(void); \
  void plugin_exit(void *)

void *enfle_plugin_load(char *, PluginType *);

#endif
