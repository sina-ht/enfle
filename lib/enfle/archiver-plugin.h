/*
 * archiver-plugin.h -- archiver plugin interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Oct 17 22:13:57 2000.
 * $Id: archiver-plugin.h,v 1.2 2000/10/17 14:04:01 sian Exp $
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

#ifndef _ARCHIVER_PLUGIN_H
#define _ARCHIVER_PLUGIN_H

#include "enfle-plugin.h"
#include "stream.h"
#include "archive.h"
#include "archiver-extra.h"

typedef struct _archiver_plugin {
  ENFLE_PLUGIN_COMMON_DATA;

  ArchiverStatus (*identify)(Archive *, Stream *);
  ArchiverStatus (*open)(Archive *, Stream *);
} ArchiverPlugin;

ENFLE_PLUGIN_ENTRIES;

#endif
