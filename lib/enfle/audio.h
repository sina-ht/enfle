/*
 * audio.h -- Audio header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue May  3 09:35:33 2005.
 *
 * SPDX-License-Identifier: GPL-2.0-only
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
