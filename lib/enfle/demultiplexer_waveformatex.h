/*
 * demultiplexer_waveformatex.h
 * (C)Copyright 2001-2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Mar 20 18:08:26 2004.
 * $Id: demultiplexer_waveformatex.h,v 1.1 2004/03/24 14:54:25 sian Exp $
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

#if !defined(__DEMULTIPLEXER_WAVEFORMATEX_H__)
#define __DEMULTIPLEXER_WAVEFORMATEX_H__

typedef struct __waveformatex WAVEFORMATEX;
struct __waveformatex {
  WORD wFormatTag;
  WORD nChannels;
  DWORD nSamplesPerSec;
  DWORD nAvgBytesPerSec;
  WORD nBlockAlign;
  WORD wBitsPerSample;
  WORD cbSize;
};

#endif
