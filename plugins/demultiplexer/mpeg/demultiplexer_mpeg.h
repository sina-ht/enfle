/*
 * demultiplexer_mpeg.h -- MPEG stream demultiplexer header
 * (C)Copyright 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Jun  5 22:24:42 2006.
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

#ifndef _DEMULTIPLEXER_MPEG_H
#define _DEMULTIPLEXER_MPEG_H

#include "enfle/demultiplexer.h"
#include "enfle/stream.h"

typedef struct _mpeg_info {
  int ver;
  int has_valid_pts;
  unsigned long prev_pts;
} MpegInfo;

Demultiplexer *demultiplexer_mpeg_create(void);

#endif
