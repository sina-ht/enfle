/*
 * audio.h -- Audio header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Dec 19 00:22:48 2000.
 * $Id: audio.h,v 1.1 2000/12/18 16:58:59 sian Exp $
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

#ifndef _AUDIO_H
#define _AUDIO_H

#include "libconfig.h"

typedef enum {
  _AUDIO_FORMAT_UNSET = 0,
  _AUDIO_FORMAT_MU_LAW,
  _AUDIO_FORMAT_A_LAW,
  _AUDIO_FORMAT_ADPCM,
  _AUDIO_FORMAT_U8,
  _AUDIO_FORMAT_S8,
  _AUDIO_FORMAT_U16_LE,
  _AUDIO_FORMAT_U16_BE,
  _AUDIO_FORMAT_S16_LE,
  _AUDIO_FORMAT_S16_BE,
#if 0
  _AUDIO_FORMAT_S32_LE,
  _AUDIO_FORMAT_S32_BE
#endif
} AudioFormat;

typedef struct _audio_device AudioDevice;
struct _audio_device {
  Config *c;
  int fd;
  AudioFormat format;
  int channels;
  int speed;
};

#endif
