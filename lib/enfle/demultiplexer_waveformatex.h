/*
 * demultiplexer_waveformatex.h
 * (C)Copyright 2001-2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Apr  6 00:41:46 2004.
 *
 * SPDX-License-Identifier: GPL-2.0-only
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
