/*
 * loader.h -- loader plugin interface header
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Jul  3 20:19:23 2001.
 * $Id: loader.h,v 1.4 2001/07/10 12:59:45 sian Exp $
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

int loader_identify(EnflePlugins *, Image *, Stream *, VideoWindow *, Config *);
LoaderStatus loader_load(EnflePlugins *, char *, Image *, Stream *, VideoWindow *, Config *);

#endif
