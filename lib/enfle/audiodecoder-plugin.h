/*
 * audiodecoder-plugin.h -- audio decoder plugin interface header
 * (C)Copyright 2000-2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Wed Jan 28 02:06:22 2004.
 * $Id: audiodecoder-plugin.h,v 1.1 2004/01/30 12:39:04 sian Exp $
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

#ifndef _AUDIODECODER_PLUGIN_H
#define _AUDIODECODER_PLUGIN_H

#include "enfle-plugin.h"
#include "enfle/audiodecoder.h"

typedef struct _audiodecoder_plugin {
  ENFLE_PLUGIN_COMMON_DATA;

  AudioDecoder *(*init)(void);
} AudioDecoderPlugin;

ENFLE_PLUGIN_ENTRIES;

#endif
