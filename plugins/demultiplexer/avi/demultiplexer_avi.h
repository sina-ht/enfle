/*
 * demultiplexer_avi.h -- AVI stream demultiplexer header
 * (C)Copyright 2001-2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Thu Feb 12 23:39:29 2004.
 * $Id: demultiplexer_avi.h,v 1.1 2004/02/14 05:22:04 sian Exp $
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

#include "enfle/demultiplexer.h"
#include "enfle/stream.h"
#include "enfle/fourcc.h"
#include "libriff.h"

#define FCC_vids FCC('v', 'i', 'd', 's')
#define FCC_auds FCC('a', 'u', 'd', 's')
#define FCC_LIST FCC('L', 'I', 'S', 'T')
#define FCC_JUNK FCC('J', 'U', 'N', 'K')
#define FCC_hdrl FCC('h', 'd', 'r', 'l')
#define FCC_avih FCC('a', 'v', 'i', 'h')
#define FCC_strl FCC('s', 't', 'r', 'l')
#define FCC_strh FCC('s', 't', 'r', 'h')
#define FCC_strf FCC('s', 't', 'r', 'f')
#define FCC_strd FCC('s', 't', 'r', 'd')
#define FCC_movi FCC('m', 'o', 'v', 'i')
#define FCC_idx1 FCC('i', 'd', 'x', '1')
#define FCC_indx FCC('i', 'n', 'd', 'x')

typedef struct _avi_info {
  RIFF_File *rf;
  unsigned int movi_start;
  unsigned int vhandler, ahandler;
  /* From MainAVIHeader */
  unsigned int swidth, sheight, nframes, rate, length;
  double framerate;
  /* From BITMAPINFOHEADER */
  unsigned int width, height, num_of_frames;
  /* From WAVEFORMATEX */
  unsigned int nchannels, samples_per_sec, num_of_samples;
  /* From AVIINDEXENTRY */
  unsigned int *idx_offset;
  unsigned int *idx_length;
} AVIInfo;

Demultiplexer *demultiplexer_avi_create(void);

#endif
