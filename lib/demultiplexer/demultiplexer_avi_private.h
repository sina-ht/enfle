/*
 * demultiplexer_avi_private.h
 * (C)Copyright 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Sep  2 23:27:52 2001.
 * $Id: demultiplexer_avi_private.h,v 1.1 2001/09/03 00:31:03 sian Exp $
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

typedef unsigned int DWORD;
typedef unsigned int LONG;
typedef unsigned int FOURCC;
typedef unsigned short int WORD;

typedef struct _mainaviheader MainAVIHeader;
struct _mainaviheader {
  DWORD dwMicroSecPerFrame;
  DWORD dwMaxBytesPerSec;
  DWORD dwReserved1;
  DWORD dwFlags;
  DWORD dwTotalFrames;
  DWORD dwInitialFrames;
  DWORD dwStreams;
  DWORD dwSuggestedBufferSize;
  DWORD dwWidth;
  DWORD dwHeight;
  DWORD dwScale;
  DWORD dwRate;
  DWORD dwStart;
  DWORD dwLength;
};

typedef struct _avistreamheader AVIStreamHeader;
struct _avistreamheader {
  FOURCC fccType;
  FOURCC fccHandler;
  DWORD dwFlags;
  DWORD dwReserved1;
  DWORD dwInitialFrames;
  DWORD dwScale;
  DWORD dwRate;
  DWORD dwStart;
  DWORD dwLength;
  DWORD dwSuggestedBufferSize;
  DWORD dwQuality;
  DWORD dwSampleSize;
};

#define BI_RGB  0
#define BI_RLE8 1
#define BI_RLE4 2
#define BI_DIV4 0x33564944

typedef struct _bitmapinfoheader BITMAPINFOHEADER;
struct _bitmapinfoheader {
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

#define WAVEFORMAT_TAG_PCM         0x01
#define WAVEFORMAT_TAG_MS_ADPCM    0x02
#define WAVEFORMAT_TAG_IMA_ADPCM   0x11
#define WAVEFORMAT_TAG_MS_GSM_6_10 0x31
#define WAVEFORMAT_TAG_MSN_Audio   0x32
#define WAVEFORMAT_TAG_MP1         0x50
#define WAVEFORMAT_TAG_MP2         0x50
#define WAVEFORMAT_TAG_MP3         0x55
#define WAVEFORMAT_TAG_Voxware     0x75
#define WAVEFORMAT_TAG_Acelp       0x130
#define WAVEFORMAT_TAG_DivX_WMA0   0x160
#define WAVEFORMAT_TAG_DivX_WMA1   0x161
#define WAVEFORMAT_TAG_IMC         0x401
#define WAVEFORMAT_TAG_AC3         0x2000

typedef struct _waveformatex WAVEFORMATEX;
struct _waveformatex {
  WORD wFormatTag;
  WORD nChannels;
  DWORD nSamplesPerSec;
  DWORD nAvgBytesPerSec;
  WORD nBlockAlign;
  WORD wBitsPerSample;
  WORD cbSize;
};

typedef struct _aviindexentry AVIINDEXENTRY;
struct _aviindexentry {
  DWORD ckid;
  DWORD dwFlags;
  DWORD dwChunkOffset;
  DWORD dwChunkLength;
};
