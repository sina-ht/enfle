/*
 * demultiplexer_bitmapinfoheader.h
 * (C)Copyright 2001-2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Apr  6 00:11:12 2004.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#if !defined(__DEMULTIPLEXER_BITMAPINFOHEADER_H__)
#define __DEMULTIPLEXER_BITMAPINFOHEADER_H__

typedef struct __bitmapinfoheader BITMAPINFOHEADER;
struct __attribute__((__packed__)) __bitmapinfoheader {
  DWORD biSize;         /* size of this structure */
  LONG  biWidth;        /* width of image */
  LONG  biHeight;       /* width of height */
  WORD  biPlanes;       /* must be 1 */
  WORD  biBitCount;     /* bits per pixel */
  DWORD biCompression;  /* compression type */
  DWORD biSizeImage;    /* size of image */
  LONG  biXPixPerMeter; /* horizontal resolution */
  LONG  biYPixPerMeter; /* vertical resolution */
  DWORD biClrUsed;      /* color table related */
  DWORD biClrImportant; /* ditto */
};

#endif
