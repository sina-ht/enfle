/*
 * loader-plugin.h -- loader plugin interface header
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Jun 23 15:17:42 2002.
 * $Id: loader-plugin.h,v 1.7 2002/08/03 05:08:40 sian Exp $
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

#ifndef _LOADER_PLUGIN_H
#define _LOADER_PLUGIN_H

#include "enfle-plugin.h"
#include "stream.h"
#include "image.h"
#include "video.h"
#include "utils/libconfig.h"
#include "loader-extra.h"

typedef struct _loader_plugin {
  ENFLE_PLUGIN_COMMON_DATA;
  void *image_private;

  LoaderStatus (*identify)(Image *, Stream *, VideoWindow *, Config *, void *);
  LoaderStatus (*load)(Image *, Stream *, VideoWindow *, Config *, void *);
} LoaderPlugin;

#define DECLARE_LOADER_PLUGIN_METHODS \
 static LoaderStatus identify(Image *, Stream *, VideoWindow *, Config *, void *); \
 static LoaderStatus load(Image *, Stream *, VideoWindow *, Config *, void *)

#define DEFINE_LOADER_PLUGIN_IDENTIFY(p, st, vw, c, priv) \
 static LoaderStatus \
 identify(Image * p , Stream * st , VideoWindow * vw , Config * c , void * priv )
#define DEFINE_LOADER_PLUGIN_LOAD(p, st, vw, c, priv) \
 static LoaderStatus \
 load(Image * p , Stream * st , VideoWindow * vw , Config * c , void * priv )

#ifndef STATIC
ENFLE_PLUGIN_ENTRIES;
#endif

#endif
