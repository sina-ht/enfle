/*
 * ui-plugin.h -- UI plugin interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Oct 15 22:20:35 2000.
 * $Id: ui-plugin.h,v 1.4 2000/10/16 19:30:29 sian Exp $
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

#ifndef _UI_PLUGIN_H
#define _UI_PLUGIN_H

typedef struct _ui_screen UIScreen;
typedef struct _ui_data UIData;

#include "enfle-plugin.h"
#include "libconfig.h"
#include "streamer.h"
#include "loader.h"
#include "archiver.h"
#include "player.h"

struct _ui_screen {
  unsigned int width, height;
  int depth, bits_per_pixel;
  void *private;
};

struct _ui_data {
  Config *c;
  Streamer *st;
  Loader *ld;
  Archiver *ar;
  Player *player;
  Archive *a;
  UIScreen *screen;
  void *private;
};

typedef struct _ui_plugin {
  ENFLE_PLUGIN_COMMON_DATA;

  int (*ui_main)(UIData *);
} UIPlugin;

ENFLE_PLUGIN_ENTRIES;

#endif
