/*
 * libmpeg2_vo.h -- video output routine for libmpeg2 player plugin
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Thu Feb 22 01:41:44 2001.
 * $Id: libmpeg2_vo.h,v 1.2 2001/02/21 17:56:29 sian Exp $
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

#include "mpeg2/video_out.h"
#include "mpeg2/video_out_internal.h"
#include "mpeg2/yuv2rgb.h"

typedef struct _enfle_frame_t {
  vo_frame_t vo;
  uint8_t *rgb_ptr;
  int rgb_stride;
  int yuv_stride;
} enfle_frame_t;

typedef struct _enfle_instance_t {
  VideoWindow *vw;
  Movie *m;
  Image *p;
  vo_instance_t vo;
  int prediction_index;
  vo_frame_t * frame_ptr[3];
  enfle_frame_t frame[3];
  uint8_t * rgbdata;
  int rgbstride;
  int width;
} enfle_instance_t;

vo_instance_t *vo_enfle_rgb_open(VideoWindow *, Movie *, Image *);

#endif
