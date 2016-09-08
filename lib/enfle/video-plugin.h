/*
 * video-plugin.h -- video plugin interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Feb 10 00:03:37 2004.
 * $Id: video-plugin.h,v 1.6 2004/02/14 05:28:08 sian Exp $
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

#ifndef _VIDEO_PLUGIN_H
#define _VIDEO_PLUGIN_H

#include "enfle-plugin.h"
#include "video.h"

typedef struct _video_plugin {
  ENFLE_PLUGIN_COMMON_DATA;

  void *(*open_video)(void *, Config *);
  int (*close_video)(void *);
  VideoWindow *(*get_root)(void *);
  VideoWindow *(*open_window)(void *, VideoWindow *, unsigned int, unsigned int);
  void (*set_wallpaper)(void *, Image *);
  void (*destroy)(void *);
} VideoPlugin;

#ifndef STATIC
ENFLE_PLUGIN_ENTRIES;
#endif

#endif
