/*
 * streamer-plugin.h -- streamer plugin interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Feb 10 00:03:28 2004.
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

#define DECLARE_STREAMER_PLUGIN_METHODS \
 static StreamerStatus identify(Stream *, char *); \
 static StreamerStatus open(Stream *, char *)

#define DEFINE_STREAMER_PLUGIN_IDENTIFY(st, p) \
 static StreamerStatus \
 identify(Stream * st , char * p )
#define DEFINE_STREAMER_PLUGIN_OPEN(st, p) \
 static StreamerStatus \
 open(Stream * st , char * p )

#ifndef STATIC
ENFLE_PLUGIN_ENTRIES;
#endif

#endif
