/*
 * demultiplexer_ogg.h -- OGG stream demultiplexer header
 * (C)Copyright 2001-2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Jul  3 17:11:56 2005.
 * $Id: demultiplexer_ogg.h,v 1.2 2005/07/08 18:14:27 sian Exp $
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

#ifndef _DEMULTIPLEXER_OGG_H
#define _DEMULTIPLEXER_OGG_H

#include "enfle/demultiplexer.h"
#include "enfle/stream.h"
#include "enfle/fourcc.h"

/* OGM packet defines */
#define PACKET_TYPE_HEADER       0x01
#define PACKET_TYPE_COMMENT      0x03
#define PACKET_TYPE_BITS         0x07
#define PACKET_LEN_BITS01        0xc0
#define PACKET_LEN_BITS2         0x02
#define PACKET_IS_SYNCPOINT      0x08

typedef struct _ogg_info {
  ogg_sync_state oy;
  int v_serialno, a_serialno;
  /* From video header */
  unsigned int vhandler, ahandler;
  unsigned int swidth, sheight, nframes, rate, length;
  struct rational framerate;
  unsigned int width, height, num_of_frames;
  /* From audio header */
  unsigned int nchannels, samples_per_sec, num_of_samples;
} OGGInfo;

Demultiplexer *demultiplexer_ogg_create(void);

#endif
