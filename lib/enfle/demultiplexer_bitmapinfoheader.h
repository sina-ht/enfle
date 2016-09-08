/*
 * demultiplexer_bitmapinfoheader.h
 * (C)Copyright 2001-2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Apr  6 00:11:12 2004.
 * $Id: demultiplexer_bitmapinfoheader.h,v 1.1 2004/04/05 15:44:54 sian Exp $
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
