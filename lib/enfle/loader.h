/*
 * loader.h -- loader plugin interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Oct 17 22:52:46 2000.
 * $Id: loader.h,v 1.2 2000/10/17 14:04:01 sian Exp $
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

#include "enfle-plugins.h"
#include "loader-extra.h"
#include "stream.h"
#include "image.h"

typedef struct _loader Loader;
struct _loader {
  int (*identify)(EnflePlugins *, Image *, Stream *);
  LoaderStatus (*load_image)(EnflePlugins *, char *, Image *, Stream *);
};

#define loader_identify(l, eps, p, s) (l)->identify((eps), (p), (s))
#define loader_load_image(l, eps, n, p, s) (l)->load_image((eps), (n), (p), (s))
#define loader_destroy(l) if ((l)) free((l))

Loader *loader_create(void);

#endif
