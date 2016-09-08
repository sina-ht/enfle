/*
 * demultiplexer_waveformatex.h
 * (C)Copyright 2001-2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Apr  6 00:41:46 2004.
 * $Id: demultiplexer_waveformatex.h,v 1.2 2004/04/05 15:46:42 sian Exp $
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

#if !defined(__DEMULTIPLEXER_WAVEFORMATEX_H__)
#define __DEMULTIPLEXER_WAVEFORMATEX_H__

typedef struct __waveformatex WAVEFORMATEX;
struct __attribute__((__packed__)) __waveformatex {
  WORD wFormatTag;
  WORD nChannels;
  DWORD nSamplesPerSec;
  DWORD nAvgBytesPerSec;
  WORD nBlockAlign;
  WORD wBitsPerSample;
  WORD cbSize;
};

#endif
