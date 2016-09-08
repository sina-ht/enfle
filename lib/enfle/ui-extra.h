/*
 * ui-extra.h -- UI plugin extra definition
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Jun  6 21:26:16 2009.
 * $Id: ui-extra.h,v 1.11 2006/01/27 06:27:53 sian Exp $
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _UI_EXTRA_H
#define _UI_EXTRA_H

typedef struct _ui_data UIData;

#include "utils/libconfig.h"
#include "utils/cache.h"
#include "enfle-plugins.h"
#include "archive.h"
#include "video.h"
#include "video-plugin.h"
#include "audio-plugin.h"

struct _ui_data {
  Config *c;
  Cache *cache;
  VideoPlugin *vp;
  VideoKey key;
  AudioPlugin *ap;
  VideoWindow *vw;
  Archive *a;
  void *disp;
  void *priv;
  int nth;
  int minw, minh;
  int if_readdir;
  int if_slideshow;
  int slide_interval;
  char *path;
};

#endif
