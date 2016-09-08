/*
 * demultiplexer-plugin.h -- demultiplexer plugin interface header
 * (C)Copyright 2000-2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Feb 13 00:23:06 2004.
 * $Id: demultiplexer-plugin.h,v 1.1 2004/02/14 05:15:36 sian Exp $
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

#ifndef _DEMULTIPLEXER_PLUGIN_H
#define _DEMULTIPLEXER_PLUGIN_H

#include "utils/libconfig.h"
#include "enfle-plugin.h"
#include "enfle/demultiplexer.h"

typedef struct _demultiplexer_plugin {
  ENFLE_PLUGIN_COMMON_DATA;

  DemultiplexerStatus (*identify)(Stream *, Config *, void *);
  Demultiplexer *(*examine)(Movie *, Stream *, Config *, void *);
} DemultiplexerPlugin;

#define DECLARE_DEMULTIPLEXER_PLUGIN_METHODS \
 static DemultiplexerStatus identify(Stream *, Config *, void *); \
 static Demultiplexer *examine(Movie *, Stream *, Config *, void *)

#define DEFINE_DEMULTIPLEXER_PLUGIN_IDENTIFY(st, c, priv) \
 static DemultiplexerStatus \
 identify(Stream * st , Config * c , void * priv )
#define DEFINE_DEMULTIPLEXER_PLUGIN_EXAMINE(m, st, c, priv) \
 static Demultiplexer * \
 examine(Movie * m , Stream * st , Config * c , void * priv )

#ifndef STATIC
ENFLE_PLUGIN_ENTRIES;
#endif

#endif
