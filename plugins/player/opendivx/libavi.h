/*
 * libavi.h -- AVI file read library header
 * (C)Copyright 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Wed Jan 24 06:54:24 2001.
 * $Id: libavi.h,v 1.2 2001/01/23 23:26:52 sian Exp $
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

#ifndef _LIBAVI_H
#define _LIBAVI_H

#if 0
/*
RIFF: 'AVI ' 27664216 bytes
[LIST]:hdrl 306 bytes
 [avih]   56 bytes:
 MicroSecPerFrame: 56 82 00 00
 MaxBytesPerSec: A3 26 03 00
 Reserved1: 00 00 00 00
 Flags: 10 08 00 00
 TotalFrames: 9A 18 00 00 (=6298)
 InitialFrames: 00 00 00 00
 Streams: 02 00 00 00
 SuggestedBufferSize: 43 51 00 00
 Width:  40 01 00 00 (=320)
 Height: F0 00 00 00 (=240)
 Scale: 00 00 00 00
 Rate: 00 00 00 00
 Start: 00 00 00 00
 Length: 00 00 00 00
 [LIST]:strl 116 bytes
  [strh]   56 bytes: 
  Type: 76 69 64 73 (vids)
  Handler: 64 69 76 34 (div4)
  Flags: 00 00 00 00
  Reserved1: 00 00 00 00
  InitialFrames: 00 00 00 00
  Scale: 56 82 00 00
  Rate: 40 42 0F 00
  Start: 00 00 00 00
  Length: 9A 18 00 00 (=6298)
  SuggestedBufferSize: 43 51 00 00
  Quality: 10 27 00 00
  SampleSize 00 00 00 00
  Unknown: 00 00 00 00
  Width?:  40 01 (=320)
  Height?: F0 00 (=240)
  [strf]   40 bytes: // BITMAPINFOHEADER
  Size: 28 00 00 00 (=40)
  Width:  40 01 00 00 (=320)
  Height: F0 00 00 00 (=240)
  Planes: 01 00
  BitCount: 18 00 (=24)
  Compression: 44 49 56 33 (DIV3)
  SizeImage: 00 84 03 00
  XPixPerMeter: 00 00 00 00
  YPixPerMeter: 00 00 00 00
  ClrUsed: 00 00 00 00
  ClrImportant: 00 00 00 00
 [/LIST]
 [LIST]:strl 106 bytes
  [strh]   56 bytes: 
  Type: 61 75 64 73 (auds)
  Handler: 00 00 00 00 (see FormatTag)
  Flags: 00 00 00 00
  Reserved1: 00 00 00 00
  InitialFrames: 00 00 00 00
  Scale: 01 00 00 00
  Rate: 80 3E 00 00 (=16000)
  Start: 00 00 00 00
  Length: 0B 2D 33 00
  SuggestedBufferSize: E9 1D 00 00
  Quality: FF FF FF FF
  SampleSize: 01 00 00 00
  unknown: 00 00 00 00 00 00 00 00
  [strf]   30 bytes: // WAVEFORMATEX
  FormatTag: 55 00 (mp3)
  Channels: 02 00
  SamplesPerSec: 44 AC 00 00 (=44100)
  AvgBytesPerSec: 80 3E 00 00 (=16000)
  BlockAlign: 01 00
  BitsPerSample: 00 00
  Size: 0C 00
  unknown: 01 00 02 00 00 00 A1 01 01 00 71 05
 [/LIST]
[/LIST]
--
RIFF: 'AVI ' 187347548 bytes
[LIST]:hdrl 306 bytes
 [avih]   56 bytes:
 MicroSecPerFrame: 55 82 00 00
 MaxBytesPerSec: BB DF 03 00
 Reserved1: 00 00 00 00
 Flags: 10 08 00 00
 TotalFrames: 4D A5 00 00 (=42317)
 InitialFrames: 00 00 00 00
 Streams: 02 00 00 00
 SuggestedBufferSize: 3F 61 00 00
 Width:  40 01 00 00 (=320)
 Height: F0 00 00 00 (=240)
 Scale: 00 00 00 00
 Rate: 00 00 00 00
 Start: 00 00 00 00
 Length: 00 00 00 00
 [LIST]:strl 116 bytes
  [strh]   56 bytes:
  Type: 76 69 64 73 (vids)
  Handler: 64 69 76 34 (div4)
  Flags: 00 00 00 00
  Reserved1: 00 00 00 00
  InitialFrames: 00 00 00 00
  Scale: 55 82 00 00
  Rate: 40 42 0F 00
  Start: 00 00 00 00
  Length: 4D A5 00 00 (=42317)
  SuggestedBufferSize: 3F 61 00 00
  Quality: 10 27 00 00
  SampleSize: 00 00 00 00
  Unknown: 00 00 00 00
  Width?:  40 01 (=320)
  Height?: F0 00 (=240)
  [strf]   40 bytes: // BITMAPINFOHEADER
  Size: 28 00 00 00 (=40)
  Width:  40 01 00 00 (=320)
  Height: F0 00 00 00 (=240)
  Planes: 01 00
  BitCount: 18 00 (=24)
  Compression: 44 49 56 33 (DIV3)
  SizeImage: 00 84 03 00
  XPixPerMeter: 00 00 00 00
  YPixPerMeter: 00 00 00 00
  ClrUsed: 00 00 00 00
  ClrImportant: 00 00 00 00
 [/LIST]
 [LIST]:strl 106 bytes
  [strh]   56 bytes:
  Type: 61 75 64 73 (auds)
  Handler: 00 00 00 00 (see FormatTag)
  Flags: 00 00 00 00
  Reserved1: 00 00 00 00
  InitialFrames: 00 00 00 00
  Scale: 01 00 00 00
  Rate: 80 3E 00 00 (=16000)
  Start: 00 00 00 00
  Length: 5C B1 58 01
  SuggestedBufferSize: 7F 1E 00 00
  Quality: FF FF FF FF
  SampleSize: 01 00 00 00
  unknown: 00 00 00 00 00 00 00 00
  [strf]   30 bytes: // WAVEFORMATEX
  FormatTag: 55 00 (mp3)
  Channels: 02 00
  SamplesPerSec: 00 7D 00 00 (=32000)
  AvgBytesPerSec: 80 3E 00 00 (=16000)
  BlockAling: 01 00
  BitsPerSample: 00 00
  Size: 0C 00
  unknown: 01 00 02 00 00 00 40 02 01 00 71 05
 [/LIST]
[/LIST]

##wb: wave data
##db: DIB
##dc: encoded by handler
--

AVIF_HASINDEX
 Indicates the AVI file has an idx1 chunk.
AVIF_MUSTUSEINDEX
 Indicates the index should be used to determine the order of presentation of the data. 
AVIF_ISINTERLEAVED
 Indicates the AVI file is interleaved.
AVIF_WASCAPTUREFILE
 Indicates the AVI file is a specially allocated file used for capturing real-time video. 
AVIF_COPYRIGHTED
 Indicates the AVI file contains copyrighted data.

The AVIF_HASINDEX and AVIF_MUSTUSEINDEX flags applies to files with an index chu
nk. The AVI_HASINDEX flag indicates an index is present. The AVIF_MUSTUSEINDEX f
lag indicates the index should be used to determine the order of the presentatio
n of the data. When this flag is set, it implies the physical ordering of the ch
unks in the file does not correspond to the presentation order.

The AVIF_ISINTERLEAVED flag indicates the AVI file has been interleaved. The sys
tem can stream interleaved data from a CD-ROM more efficiently than non-interlea
ved data. For more information on interleaved files, see special Information fo
r Interleaved Files.<94>

The AVIF_WASCAPTUREFILE flag indicates the AVI file is a specially allocated fil
e used for capturing real-time video. Typically, capture files have been defragm
ented by user so video capture data can be efficiently streamed into the file. I
f this flag is set, an application should warn the user before writing over the 
file with this flag.

The AVIF_COPYRIGHTED flag indicates the AVI file contains copyrighted data. When
 this flag is set, applications should not let users duplicate the file or the d
ata in the file.

*/
#endif

#include "libriff.h"

typedef unsigned int DWORD;
typedef unsigned int FOURCC;
typedef unsigned short int WORD;

#define AVIF_HASINDEX            0x00000010
#define AVIF_MUSTUSEINDEX        0x00000020
#define AVIF_ISINTERLEAVED       0x00000100
#define AVIF_WASCAPTUREFILE      0x00010000
#define AVIF_COPYRIGHTED         0x00020000

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
  unsigned long  biSize;         /* size of this structure */
  long biWidth;                  /* width of image */
  long biHeight;                 /* width of height */
  unsigned short biPlanes;       /* must be 1 */
  unsigned short biBitCount;     /* bits per pixel */
  unsigned long  biCompression;  /* compression type */
  unsigned long  biSizeImage;    /* size of image */
  long biXPixPerMeter;           /* horizontal resolution */
  long biYPixPerMeter;           /* vertical resolution */
  unsigned long  biClrUsed;      /* color table related */
  unsigned long  biClrImportant; /* ditto */
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

typedef struct _avifilereader AviFileReader;
struct _avifilereader {
  RIFF_File *rf;
  MainAVIHeader header;
  AVIStreamHeader vsheader;
  AVIStreamHeader asheader;
  BITMAPINFOHEADER vformat;
  WAVEFORMATEX aformat;
  //AVIINDEXENTRY *index;
  RIFF_Chunk **vchunks;
  RIFF_Chunk **achunks;
  int vnchunks, anchunks;

  int (*open_avi)(AviFileReader *, RIFF_File *);
  int (*close_avi)(AviFileReader *);
  void (*destroy)(AviFileReader *);
};

#define avifilereader_open(afr, rf) (afr)->open_avi((afr), (rf))
#define avifilereader_close(afr) (afr)->close_avi((afr)) 
#define avifilereader_destroy(afr) (afr)->destroy((afr))

/* class method */
int avifilereader_identify(char *);

AviFileReader *avifilereader_create(void);


#endif
