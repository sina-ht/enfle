/*
 * demultiplexer_mpeg.h -- MPEG stream demultiplexer header
 * (C)Copyright 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Jan 20 22:17:44 2004.
 * $Id: demultiplexer_mpeg.h,v 1.5 2004/01/24 07:08:10 sian Exp $
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

#ifndef _DEMULTIPLEXER_MPEG_H
#define _DEMULTIPLEXER_MPEG_H

#include "demultiplexer.h"
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

Demultiplexer *demultiplexer_mpeg_create(void);

#define demultiplexer_mpeg_set_input(de, st) ((MpegInfo *)(de)->private_data)->st = (st)
#define demultiplexer_mpeg_set_vst(de, v) ((MpegInfo *)(de)->private_data)->vstream = (v)
#define demultiplexer_mpeg_set_ast(de, a) ((MpegInfo *)(de)->private_data)->astream = (a)
#define demultiplexer_mpeg_nvideos(de) ((MpegInfo *)(de)->private_data)->nvstreams
#define demultiplexer_mpeg_naudios(de) ((MpegInfo *)(de)->private_data)->nastreams
#define demultiplexer_mpeg_set_video(de, nv) ((MpegInfo *)(de)->private_data)->nvstream = (nv)
#define demultiplexer_mpeg_set_audio(de, na) ((MpegInfo *)(de)->private_data)->nastream = (na)

#endif
