/*
 * loader.h -- loader plugin interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Sep 18 07:17:37 2000.
 * $Id: loader.h,v 1.1 2000/09/30 17:36:36 sian Exp $
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

#ifndef _LOADER_H
#define _LOADER_H

#include "pluginlist.h"
#include "stream.h"
#include "image.h"
#include "loader-plugin.h"

typedef struct _loader Loader;
struct _loader {
  PluginList *pl;

  char *(*load)(Loader *, Plugin *);
  int (*unload)(Loader *, char *);
  int (*identify)(Loader *, Image *, Stream *);
  LoaderStatus (*load_image)(Loader *, char *, Image *, Stream *);
  void (*destroy)(Loader *);
  Dlist *(*get_names)(Loader *);
  unsigned char *(*get_description)(Loader *, char *);
  unsigned char *(*get_author)(Loader *, char *);
};

#define loader_load(l, p) (l)->load((l), (p))
#define loader_unload(l, n) (l)->unload((l), (n))
#define loader_identify(l, p, s) (l)->identify((l), (p), (s))
#define loader_load_image(l, n, p, s) (l)->load_image((l), (n), (p), (s))
#define loader_destroy(l) (l)->destroy((l))
#define loader_get_names(l) (l)->get_names((l))
#define loader_get_description(l, n) (l)->get_description((l), (n))
#define loader_get_author(l, n) (l)->get_author((l), (n))

Loader *loader_create(void);

#endif
