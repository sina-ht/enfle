/*
 * audiodecoder.h -- Audio Decoder header
 * (C)Copyright 2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Apr 10 17:52:39 2004.
 * $Id: audiodecoder.h,v 1.6 2004/04/12 04:14:10 sian Exp $
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
