/*
 * ui-plugin.h -- UI plugin interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Sep 18 07:18:42 2000.
 * $Id: ui-plugin.h,v 1.1 2000/09/30 17:36:36 sian Exp $
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

#include "enfle-plugin.h"
#include "libconfig.h"
#include "streamer.h"
#include "loader.h"
#include "archiver.h"

typedef struct _ui_data {
  Config *c;
  Streamer *st;
  Loader *ld;
  Archiver *ar;
  Archive *a;
} UIData;

typedef struct _ui_plugin {
  ENFLE_PLUGIN_COMMON_DATA;

  int (*ui_main)(UIData *);
} UIPlugin;

ENFLE_PLUGIN_ENTRIES;

#endif
