/*
 * demultiplexer_mpeg.h -- MPEG stream demultiplexer header
 * (C)Copyright 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Feb 13 00:06:12 2004.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _DEMULTIPLEXER_OLD_MPEG_H
#define _DEMULTIPLEXER_OLD_MPEG_H

#include "demultiplexer_old.h"
#include "enfle/stream.h"
#include "utils/fifo.h"

typedef struct _mpeg_info {
  Stream *st;
  FIFO *vstream;
  FIFO *astream;
  int ver;
  int nvstreams;
  int nastreams;
  int nvstream;
  int nastream;
} MpegInfo;

Demultiplexer_old *demultiplexer_mpeg_create(void);

#define demultiplexer_mpeg_set_input(de, st) ((MpegInfo *)(de)->private_data)->st = (st)
#define demultiplexer_mpeg_set_vst(de, v) ((MpegInfo *)(de)->private_data)->vstream = (v)
#define demultiplexer_mpeg_set_ast(de, a) ((MpegInfo *)(de)->private_data)->astream = (a)
#define demultiplexer_mpeg_nvideos(de) ((MpegInfo *)(de)->private_data)->nvstreams
#define demultiplexer_mpeg_naudios(de) ((MpegInfo *)(de)->private_data)->nastreams
#define demultiplexer_mpeg_set_video(de, nv) ((MpegInfo *)(de)->private_data)->nvstream = (nv)
#define demultiplexer_mpeg_set_audio(de, na) ((MpegInfo *)(de)->private_data)->nastream = (na)

#endif
