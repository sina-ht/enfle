/*
 * archiver-plugin.h -- archiver plugin interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Feb 10 00:02:54 2004.
 * $Id: archiver-plugin.h,v 1.6 2004/02/14 05:28:08 sian Exp $
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
  void *archiver_private;

  ArchiverStatus (*identify)(Archive *, Stream *, void *);
  ArchiverStatus (*open)(Archive *, Stream *, void *);
} ArchiverPlugin;

#define DECLARE_ARCHIVER_PLUGIN_METHODS \
 static ArchiverStatus identify(Archive *, Stream *, void *); \
 static ArchiverStatus open(Archive *, Stream *, void *)

#define DEFINE_ARCHIVER_PLUGIN_IDENTIFY(a, st, priv) \
 static ArchiverStatus \
 identify(Archive * a , Stream * st , void * priv )
#define DEFINE_ARCHIVER_PLUGIN_OPEN(a, st, priv) \
 static ArchiverStatus \
 open(Archive * a , Stream * st , void * priv )

#ifndef STATIC
ENFLE_PLUGIN_ENTRIES;
#endif

#endif
