/*
 * demultiplexer_mpeg.h -- MPEG stream demultiplexer header
 * (C)Copyright 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Thu Feb 12 23:39:34 2004.
 * $Id: demultiplexer_mpeg.h,v 1.1 2004/02/14 05:22:15 sian Exp $
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

#include "enfle/demultiplexer.h"
#include "enfle/stream.h"

typedef struct _mpeg_info {
  int ver;
} MpegInfo;

Demultiplexer *demultiplexer_mpeg_create(void);

#endif
