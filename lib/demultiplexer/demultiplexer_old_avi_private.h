/*
 * demultiplexer_avi_private.h
 * (C)Copyright 2001-2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Jan 12 04:15:39 2004.
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

typedef unsigned int DWORD;
typedef unsigned int LONG;
typedef unsigned int FOURCC;
typedef unsigned short int WORD;

/*
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
#define AVIF_HASINDEX            0x00000010
#define AVIF_MUSTUSEINDEX        0x00000020
#define AVIF_ISINTERLEAVED       0x00000100
#define AVIF_WASCAPTUREFILE      0x00010000
#define AVIF_COPYRIGHTED         0x00020000

typedef struct __mainaviheader MainAVIHeader;
struct __mainaviheader {
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

typedef struct __avistreamheader AVIStreamHeader;
struct __avistreamheader {
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

typedef struct __bitmapinfoheader BITMAPINFOHEADER;
struct __bitmapinfoheader {
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

#define AVIIF_LIST     0x00000001 // chunk is LIST
#define AVIIF_KEYFRAME 0x00000010 // key frame.
#define AVIIF_NOTIME   0x00000100 // doesn't take any time (e.g. palette)

typedef struct __aviindexentry AVIINDEXENTRY;
struct __aviindexentry {
  DWORD ckid;
  DWORD dwFlags;
  DWORD dwChunkOffset;
  DWORD dwChunkLength;
};
