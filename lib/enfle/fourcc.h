/*
 * fourcc.h -- Four Character Codes
 * (C)Copyright 2002-2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Feb 10 22:35:03 2004.
 * $Id: fourcc.h,v 1.7 2004/02/14 05:28:48 sian Exp $
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

#ifndef _FOURCC_H
#define _FOURCC_H

#define FCC(a,b,c,d) ((((((d << 8) | c) << 8) | b) << 8) | a)

/*
 * Video Codecs
 */

// image formats
#define FCC_YUY2 FCC('Y', 'U', 'Y', '2')
#define FCC_YV12 FCC('Y', 'V', '1', '2')
#define FCC_IYUV FCC('I', 'Y', 'U', 'V')
#define FCC_UYVY FCC('U', 'Y', 'V', 'Y')
#define FCC_I420 FCC('I', '4', '2', '0')
#define FCC_ABGR FCC('A', 'B', 'G', 'R')
#define FCC_Y422 FCC('Y', '4', '2', '2')

#define FCC_CYUV FCC('C', 'Y', 'U', 'V')
#define FCC_HFYU FCC('H', 'F', 'Y', 'U')

// H263
#define FCC_H263 FCC('H', '2', '6', '3')
#define FCC_I263 FCC('I', '2', '6', '3')
#define FCC_U263 FCC('U', '2', '6', '3')
#define FCC_viv1 FCC('v', 'i', 'v', '1')

// MPEG4
#define FCC_MP4S FCC('M', 'P', '4', 'S')
#define FCC_M4S2 FCC('M', '4', 'S', '2')
#define FCC_0x04000000 FCC(4, 0, 0, 0)
#define FCC_DIV1 FCC('D', 'I', 'V', '1')
#define FCC_BLZ0 FCC('B', 'L', 'Z', '0')
#define FCC_mp4v FCC('m', 'p', '4', 'v')
#define FCC_UMP4 FCC('U', 'M', 'P', '4')
#define FCC_XVID FCC('X', 'V', 'I', 'D')

// DivX ;-) 3.11 (MS-MPEG4v3)
#define FCC_DIV3 FCC('D', 'I', 'V', '3')
#define FCC_DIV4 FCC('D', 'I', 'V', '4')
#define FCC_DIV5 FCC('D', 'I', 'V', '5')
#define FCC_DIV6 FCC('D', 'I', 'V', '6')
#define FCC_MP41 FCC('M', 'P', '4', '1')
// DivX 4
#define FCC_DIVX FCC('D', 'I', 'V', 'X')
#define FCC_divx FCC('d', 'i', 'v', 'x')
// DivX 5
#define FCC_DX50 FCC('D', 'X', '5', '0')

// MS-MPEG4v3
#define FCC_MP43 FCC('M', 'P', '4', '3')
#define FCC_MPG3 FCC('M', 'P', 'G', '3')
#define FCC_AP41 FCC('A', 'P', '4', '1')
#define FCC_COL1 FCC('C', 'O', 'L', '1')
#define FCC_COL0 FCC('C', 'O', 'L', '0')

// MS-MPEG4v2
#define FCC_MP42 FCC('M', 'P', '4', '2')
#define FCC_mp42 FCC('m', 'p', '4', '2')
#define FCC_DIV2 FCC('D', 'I', 'V', '2')

// MS-MPEG4v1
#define FCC_MPG4 FCC('M', 'P', 'G', '4')
#define FCC_mpg4 FCC('m', 'p', 'g', '4')

// WMV
#define FCC_WMV1 FCC('W', 'M', 'V', '1')
#define FCC_WMV2 FCC('W', 'M', 'V', '2')

// DVVIDEO
#define FCC_dvsd FCC('d', 'v', 's', 'd')
#define FCC_dvhd FCC('d', 'v', 'h', 'd')
#define FCC_dvsl FCC('d', 'v', 's', 'l')
#define FCC_dv25 FCC('d', 'v', '2', '5')

// MPEG1VIDEO
#define FCC_mpg1 FCC('m', 'p', 'g', '1')
#define FCC_mpg2 FCC('m', 'p', 'g', '2')
#define FCC_PIM1 FCC('P', 'I', 'M', '1')
#define FCC_VCR2 FCC('V', 'C', 'R', '2')

// MJPEG
#define FCC_MJPG FCC('M', 'J', 'P', 'G')
#define FCC_JPGL FCC('J', 'P', 'G', 'L')
#define FCC_LJPG FCC('L', 'J', 'P', 'G')

// INDEO
#define FCC_IV31 FCC('I', 'V', '3', '1')
#define FCC_IV32 FCC('I', 'V', '3', '2')

// MSRLE
#define FCC_mrle FCC('m', 'r', 'l', 'e')
#define FCC_0x01000000 FCC(1, 0, 0, 0)

// MSVIDEO1
#define FCC_MSVC FCC('M', 'S', 'V', 'C')
#define FCC_msvc FCC('m', 's', 'v', 'c')
#define FCC_CRAM FCC('C', 'R', 'A', 'M')
#define FCC_cram FCC('c', 'r', 'a', 'm')
#define FCC_WHAM FCC('W', 'H', 'A', 'M')
#define FCC_wham FCC('w', 'h', 'a', 'm')

// misc
#define FCC_VP31 FCC('V', 'P', '3', '1')
#define FCC_ASV1 FCC('A', 'S', 'V', '1')
#define FCC_ASV2 FCC('A', 'S', 'V', '2')
#define FCC_VCR1 FCC('V', 'C', 'R', '1')
#define FCC_FFV1 FCC('F', 'F', 'V', '1')
#define FCC_Xxan FCC('X', 'x', 'a', 'n')
#define FCC_cvid FCC('c', 'v', 'i', 'd')

/*
 * Audio Codecs
 */

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

#endif
