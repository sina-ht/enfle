/*
 * audiodecoder.h -- Audio Decoder header
 * (C)Copyright 2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Apr 10 17:52:39 2004.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#if !defined(_ENFLE_AUDIO_DECODER_H)
#define _ENFLE_AUDIO_DECODER_H

typedef struct _audio_decoder AudioDecoder;

#include "enfle/enfle-plugins.h"
#include "enfle/movie.h"

typedef enum _audio_decoder_status {
  AD_OK = 0,
  AD_ERROR,
  AD_NEED_MORE_DATA
} AudioDecoderStatus;

struct _audio_decoder {
  const char *name;
  void *opaque;

  int (*setup)(AudioDecoder *, Movie *m);
  AudioDecoderStatus (*decode)(AudioDecoder *, Movie *, AudioDevice *, unsigned char *, unsigned int, unsigned int *);
  void (*destroy)(AudioDecoder *);
};

#define audiodecoder_setup(adec,m) (adec)->setup(adec,m)
#define audiodecoder_decode(adec,m,ad,b,l,r) (adec)->decode(adec,m,ad,b,l,r)
#define audiodecoder_destroy(adec) (adec)->destroy(adec)

/* protected */
AudioDecoder *_audiodecoder_init(void);
void _audiodecoder_destroy(AudioDecoder *);

const char *audiodecoder_codec_name(unsigned int);
int audiodecoder_query(EnflePlugins *, Movie *, unsigned int, unsigned int *, Config *);
int audiodecoder_select(EnflePlugins *, Movie *, unsigned int, Config *);
AudioDecoder *audiodecoder_create(EnflePlugins *, const char *);

#endif
