/*
 * streamer-plugin.h -- streamer plugin interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Sep 18 07:18:16 2000.
 * $Id: streamer-plugin.h,v 1.1 2000/09/30 17:36:36 sian Exp $
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

#ifndef _STREAMER_PLUGIN_H
#define _STREAMER_PLUGIN_H

#include "enfle-plugin.h"
#include "stream.h"

typedef enum {
  STREAM_ERROR = -1,
  STREAM_NOT,
  STREAM_OK
} StreamerStatus;

typedef struct _streamer_plugin {
  ENFLE_PLUGIN_COMMON_DATA;

  StreamerStatus (*identify)(Stream *, char *);
  StreamerStatus (*open)(Stream *, char *);
} StreamerPlugin;

ENFLE_PLUGIN_ENTRIES;

#endif
