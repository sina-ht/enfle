/*
 * video-plugin.h -- video plugin interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Wed Dec 13 00:25:24 2000.
 * $Id: video-plugin.h,v 1.3 2000/12/12 17:03:58 sian Exp $
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

#ifndef _VIDEO_PLUGIN_H
#define _VIDEO_PLUGIN_H

#include "enfle-plugin.h"
#include "video.h"

typedef struct _video_plugin {
  ENFLE_PLUGIN_COMMON_DATA;

  void *(*open_video)(void *);
  int (*close_video)(void *);
  VideoWindow *(*open_window)(void *, Config *, unsigned int, unsigned int);
  void (*destroy)(void *);
} VideoPlugin;

ENFLE_PLUGIN_ENTRIES;

#endif
