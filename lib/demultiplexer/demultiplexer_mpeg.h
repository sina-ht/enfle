/*
 * demultiplexer_mpeg.h -- MPEG stream demultiplexer header
 * (C)Copyright 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Wed Feb 21 00:15:26 2001.
 * $Id: demultiplexer_mpeg.h,v 1.2 2001/02/20 15:16:35 sian Exp $
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

#include "stream.h"

Demultiplexer *demultiplexer_mpeg_create(void);

#define demultiplexer_mpeg_set_input(de, st) (de)->info->st = (st)
#define demultiplexer_mpeg_set_vfd(de, vfd) (de)->info->v_fd = (vfd)
#define demultiplexer_mpeg_set_afd(de, afd) (de)->info->a_fd = (afd)
#define demultiplexer_mpeg_nvideos(de) (de)->info->nvstreams
#define demultiplexer_mpeg_naudios(de) (de)->info->nastreams
#define demultiplexer_mpeg_set_video(de, nv) (de)->info->nvstream = (nv)
#define demultiplexer_mpeg_set_audio(de, na) (de)->info->nastream = (na)

#endif
