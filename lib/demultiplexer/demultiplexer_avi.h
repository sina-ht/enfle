/*
 * demultiplexer_avi.h -- AVI stream demultiplexer header
 * (C)Copyright 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Sep  3 00:33:49 2001.
 * $Id: demultiplexer_avi.h,v 1.2 2003/02/05 15:20:51 sian Exp $
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

#ifndef _DEMULTIPLEXER_AVI_H
#define _DEMULTIPLEXER_AVI_H

#include "demultiplexer.h"
#include "enfle/stream.h"
#include "utils/fifo.h"
#include "libriff.h"

typedef struct _avi_packet {
  unsigned int size;
  void *data;
} AVIPacket;

typedef struct _avi_info {
  Stream *st;
  RIFF_File *rf;
  FIFO *vstream, *astream;
  unsigned int movi_start;
  unsigned int vhandler, ahandler;
  /* From MainAVIHeader */
  unsigned int swidth, sheight, nframes, rate, length;
  double framerate;
  /* From BITMAPINFOHEADER */
  unsigned int width, height, num_of_frames;
  /* From WAVEFORMATEX */
  unsigned int nchannels, samples_per_sec, num_of_samples;
  int nvstreams, nastreams;
  int nvstream, nastream;
} AVIInfo;

Demultiplexer *demultiplexer_avi_create(void);

#define demultiplexer_avi_set_input(de, st) ((AVIInfo *)(de)->private_data)->st = (st)
#define demultiplexer_avi_set_vst(de, v) ((AVIInfo *)(de)->private_data)->vstream = (v)
#define demultiplexer_avi_set_ast(de, a) ((AVIInfo *)(de)->private_data)->astream = (a)
#define demultiplexer_avi_nvideos(de) ((AVIInfo *)(de)->private_data)->nvstreams
#define demultiplexer_avi_naudios(de) ((AVIInfo *)(de)->private_data)->nastreams
#define demultiplexer_avi_set_video(de, nv) ((AVIInfo *)(de)->private_data)->nvstream = (nv)
#define demultiplexer_avi_set_audio(de, na) ((AVIInfo *)(de)->private_data)->nastream = (na)
#define demultiplexer_avi_info(de) ((AVIInfo *)(de)->private_data)

#endif
