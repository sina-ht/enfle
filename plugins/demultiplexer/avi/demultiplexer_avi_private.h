/*
 * demultiplexer_avi_private.h
 * (C)Copyright 2001-2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Apr  5 23:31:33 2004.
 * $Id: demultiplexer_avi_private.h,v 1.3 2004/04/05 15:43:54 sian Exp $
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

#include "enfle/demultiplexer_types.h"

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

#include "enfle/demultiplexer_bitmapinfoheader.h"
#include "enfle/demultiplexer_waveformatex.h"

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
