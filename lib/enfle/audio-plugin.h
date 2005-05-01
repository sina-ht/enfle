/*
 * audio-plugin.h -- audio plugin interface header
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun May  1 17:00:17 2005.
 * $Id: audio-plugin.h,v 1.8 2005/05/01 15:37:55 sian Exp $
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

#ifndef _AUDIO_PLUGIN_H
#define _AUDIO_PLUGIN_H

#include "enfle-plugin.h"
#include "enfle/audio.h"

typedef struct _audio_plugin {
  ENFLE_PLUGIN_COMMON_DATA;

  AudioDevice *(*open_device)(void *, Config *);
  int (*set_params)(AudioDevice *, AudioFormat *, int *, unsigned int *);
  int (*write_device)(AudioDevice *, unsigned char *, int);
  int (*bytes_written)(AudioDevice *);
  int (*sync_device)(AudioDevice *);
  int (*close_device)(AudioDevice *);
} AudioPlugin;

#ifndef STATIC
ENFLE_PLUGIN_ENTRIES;
#endif

#endif
