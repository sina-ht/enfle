/*
 * videodecoder.h -- Video Decoder header
 * (C)Copyright 2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Jun  5 22:23:35 2006.
 * $Id: videodecoder.h,v 1.7 2006/06/05 13:56:45 sian Exp $
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

#if !defined(_ENFLE_VIDEO_DECODER_H)
#define _ENFLE_VIDEO_DECODER_H

typedef struct _video_decoder VideoDecoder;

#include "enfle/enfle-plugins.h"
#include "enfle/demultiplexer.h"
#include "enfle/movie.h"

typedef enum _video_decoder_status {
  VD_OK = 0,
  VD_ERROR,
  VD_NEED_MORE_DATA
} VideoDecoderStatus;

struct _video_decoder {
  const char *name;
  void *opaque;
  unsigned long ts_base;
  unsigned long pts;
  int to_render;
  pthread_mutex_t update_mutex;
  pthread_cond_t update_cond;

  int (*setup)(VideoDecoder *, Movie *, Image *, int, int);
  VideoDecoderStatus (*decode)(VideoDecoder *, Movie *, Image *, DemuxedPacket *, unsigned int, unsigned int *);
  void (*destroy)(VideoDecoder *);
};

#define videodecoder_setup(vdec,m,p,w,h) (vdec)->setup(vdec,m,p,w,h)
#define videodecoder_decode(vdec,m,p,dp,l,r) (vdec)->decode(vdec,m,p,dp,l,r)
#define videodecoder_destroy(vdec) (vdec)->destroy(vdec)

/* protected */
VideoDecoder *_videodecoder_init(void);
void _videodecoder_destroy(VideoDecoder *);

const char *videodecoder_codec_name(unsigned int);
int videodecoder_query(EnflePlugins *, Movie *, unsigned int, unsigned int *, Config *);
int videodecoder_select(EnflePlugins *, Movie *, unsigned int, Config *);
VideoDecoder *videodecoder_create(EnflePlugins *, const char *);

#endif
