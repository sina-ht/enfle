/*
 * libmpeg2_vo.h -- video output routine for libmpeg2 player plugin
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Jun 22 19:56:04 2001.
 * $Id: libmpeg2_vo.h,v 1.6 2001/06/22 17:34:43 sian Exp $
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

#ifndef _LIBMPEG2_VO_H
#define _LIBMPEG2_VO_H

#include "mpglib/mpg123.h"
#include "mpglib/mpglib.h"
#include "mpeg2/video_out.h"
#include "mpeg2/video_out_internal.h"
#include "mpeg2/yuv2rgb.h"
#include "mpeg2/mpeg2.h"
#include "demultiplexer/demultiplexer_mpeg.h"
#include "utils/fifo.h"

typedef struct _enfle_frame_t {
  vo_frame_t vo;
  uint8_t *rgb_ptr;
  uint8_t *rgb_ptr_base;
  int rgb_stride;
  int yuv_stride;
} enfle_frame_t;

typedef struct _enfle_instance_t {
  vo_instance_t vo;
  int prediction_index;
  vo_frame_t *frame_ptr[3];
  enfle_frame_t frame[3];
  Movie *m;
  Image *p;
  int image_size;
  int rgbstride;
  int width;
  int height;
} enfle_instance_t;

typedef struct _libmpeg2_info {
  VideoWindow *vw;
  Image *p;
  Config *c;
  AudioDevice *ad;
  struct mpstr mp;
  mpeg2dec_t mpeg2dec;
  Demultiplexer *demux;
  vo_instance_t *vo;
  int rendering_type;
  int to_render;
  int if_initialized;
  pthread_mutex_t update_mutex;
  pthread_cond_t update_cond;
  int nvstreams;
  int nvstream;
  FIFO *vstream;
  pthread_t video_thread;
  int nastreams;
  int nastream;
  FIFO *astream;
  pthread_t audio_thread;
} Libmpeg2_info;

vo_instance_t *vo_enfle_rgb_open(VideoWindow *, Movie *, Image *);

#endif
