/*
 * streamer.h -- streamer header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Sep 18 07:18:33 2000.
 * $Id: streamer.h,v 1.1 2000/09/30 17:36:36 sian Exp $
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

#ifndef _STREAMER_H
#define _STREAMER_H

#include "pluginlist.h"
#include "stream.h"

typedef struct _streamer Streamer;
struct _streamer {
  PluginList *pl;

  char *(*load)(Streamer *, Plugin *);
  int (*unload)(Streamer *, char *);
  int (*identify)(Streamer *, Stream *, char *);
  int (*open)(Streamer *, Stream *, char *, char *);
  void (*destroy)(Streamer *);
  Dlist *(*get_names)(Streamer *);
  unsigned char *(*get_description)(Streamer *, char *);
  unsigned char *(*get_author)(Streamer *, char *);
};

#define streamer_load(st, p) (st)->load((st), (p))
#define streamer_unload(st, n) (st)->unload((st), (n))
#define streamer_identify(st, s, path) (st)->identify((st), (s), (path))
#define streamer_open(st, s, n, path) (st)->open((st), (s), (n), (path))
#define streamer_destroy(st) (st)->destroy((st))
#define streamer_get_names(st) (st)->get_names((st))
#define streamer_get_description(st, n) (st)->get_description((st), (n))
#define streamer_get_author(st, n) (st)->get_author((st), (n))

Streamer *streamer_create(void);

#endif
