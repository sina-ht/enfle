/*
 * videodecoder.h -- Video Decoder header
 * (C)Copyright 2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Wed Jan 28 02:05:09 2004.
 * $Id: videodecoder.h,v 1.1 2004/01/30 12:39:04 sian Exp $
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

#if !defined(_ENFLE_VIDEO_DECODER_H)
#define _ENFLE_VIDEO_DECODER_H

#include "enfle/enfle-plugins.h"
#include "enfle/movie.h"

typedef enum _video_decoder_status {
  VD_OK = 0,
  VD_ERROR,
  VD_NEED_MORE_DATA
} VideoDecoderStatus;

typedef struct _video_decoder VideoDecoder;
struct _video_decoder {
  void *opaque;
  int to_render;
  pthread_mutex_t update_mutex;
  pthread_cond_t update_cond;

  int (*setup)(VideoDecoder *, Movie *, Image *, int, int);
  VideoDecoderStatus (*decode)(VideoDecoder *, Movie *, Image *, unsigned char *, unsigned int, unsigned int *);
  void (*destroy)(VideoDecoder *);
};

#define videodecoder_setup(vdec,m,p,w,h) (vdec)->setup(vdec,m,p,w,h)
#define videodecoder_decode(vdec,m,p,b,l,r) (vdec)->decode(vdec,m,p,b,l,r)
#define videodecoder_destroy(vdec) (vdec)->destroy(vdec)

/* protected */
VideoDecoder *_videodecoder_init(void);
void _videodecoder_destroy(VideoDecoder *);

VideoDecoder *videodecoder_create(EnflePlugins *, const char *);

#endif