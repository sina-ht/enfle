/*
 * audio.h -- Audio header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue May  3 09:35:33 2005.
 * $Id: audio.h,v 1.5 2005/05/03 01:08:30 sian Exp $
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

#ifndef _ENFLE_AUDIO_H
#define _ENFLE_AUDIO_H

#include "utils/libconfig.h"

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
  void *private_data;
  Config *c;
  int opened;
  int fd;
  AudioFormat format;
  unsigned int bytes_written;
  int bytes_per_sample;
  unsigned int channels;
  unsigned int speed;
};

#endif
