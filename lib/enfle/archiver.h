/*
 * archiver.h -- archiver header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Thu Sep 28 17:33:42 2000.
 * $Id: archiver.h,v 1.1 2000/09/30 17:36:36 sian Exp $
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

#ifndef _ARCHIVER_H
#define _ARCHIVER_H

#include "pluginlist.h"
#include "archive.h"
#include "stream.h"
#include "archiver-plugin.h"

typedef struct _archiver Archiver;
struct _archiver {
  PluginList *pl;

  char *(*load)(Archiver *, Plugin *);
  int (*unload)(Archiver *, char *);
  int (*identify)(Archiver *, Archive *, Stream *);
  ArchiverStatus (*open)(Archiver *, Archive *, char *, Stream *);
  void (*destroy)(Archiver *);
  Dlist *(*get_names)(Archiver *);
  unsigned char *(*get_description)(Archiver *, char *);
  unsigned char *(*get_author)(Archiver *, char *);
};

#define archiver_load(ar, p) (ar)->load((ar), (p))
#define archiver_unload(ar, n) (ar)->unload((ar), (n))
#define archiver_identify(ar, a, s) (ar)->identify((ar), (a), (s))
#define archiver_open(ar, a, n, s) (ar)->open((ar), (a), (n), (s))
#define archiver_destroy(ar) (ar)->destroy((ar))
#define archiver_get_names(ar) (ar)->get_names((ar))
#define archiver_get_description(ar, n) (ar)->get_description((ar), (n))
#define archiver_get_author(ar, n) (ar)->get_author((ar), (n))

Archiver *archiver_create(void);

#endif
