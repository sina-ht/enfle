/*
 * loader.h -- loader plugin interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Jun 18 20:23:48 2001.
 * $Id: loader.h,v 1.3 2001/06/18 16:23:47 sian Exp $
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
#include "video.h"

typedef struct _loader Loader;
struct _loader {
  int (*identify)(EnflePlugins *, Image *, Stream *, VideoWindow *, Config *);
  LoaderStatus (*load_image)(EnflePlugins *, char *, Image *, Stream *, VideoWindow *, Config *);
};

#define loader_identify(l, eps, p, s, vw, c) (l)->identify((eps), (p), (s), (vw), (c))
#define loader_load_image(l, eps, n, p, s, vw, c) (l)->load_image((eps), (n), (p), (s), (vw), (c))
#define loader_destroy(l) if ((l)) free((l))

Loader *loader_create(void);

#endif
