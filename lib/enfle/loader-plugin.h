/*
 * loader-plugin.h -- loader plugin interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Sep 18 07:17:20 2000.
 * $Id: loader-plugin.h,v 1.1 2000/09/30 17:36:36 sian Exp $
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

typedef enum {
  LOAD_ERROR = -1,
  LOAD_NOT,
  LOAD_OK
} LoaderStatus;

typedef struct _loader_plugin {
  ENFLE_PLUGIN_COMMON_DATA;

  LoaderStatus (*identify)(Image *, Stream *);
  LoaderStatus (*load)(Image *, Stream *);
} LoaderPlugin;

void *plugin_entry(void);
void plugin_exit(void *);

#endif
