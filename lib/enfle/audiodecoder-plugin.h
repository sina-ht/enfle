/*
 * audiodecoder-plugin.h -- audio decoder plugin interface header
 * (C)Copyright 2000-2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Apr 10 17:45:14 2004.
 * $Id: audiodecoder-plugin.h,v 1.4 2004/04/12 04:14:10 sian Exp $
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
  void *ad_private;

  unsigned int (*query)(unsigned int, void *);
  AudioDecoder *(*init)(unsigned int, void *);
} AudioDecoderPlugin;

#define DECLARE_AUDIODECODER_PLUGIN_METHODS \
  static unsigned int query(unsigned int, void *); \
  static AudioDecoder *init(unsigned int, void *)

#ifndef STATIC
ENFLE_PLUGIN_ENTRIES;
#endif

#endif
