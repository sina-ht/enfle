/*
 * fourcc.h -- Four Character Codes
 * (C)Copyright 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Feb 22 01:36:22 2003.
 * $Id: fourcc.h,v 1.2 2003/10/12 03:57:56 sian Exp $
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
#define FCC_YUY2 FCC('Y', 'U', 'Y', '2')
#define FCC_YV12 FCC('Y', 'V', '1', '2')
#define FCC_IYUV FCC('I', 'Y', 'U', 'V')
#define FCC_UYVY FCC('U', 'Y', 'V', 'Y')

//   DivX ;-) 3.11
#define FCC_div3 FCC('d', 'i', 'v', '3')
#define FCC_DIV3 FCC('D', 'I', 'V', '3')
#define FCC_div4 FCC('d', 'i', 'v', '4')
#define FCC_DIV4 FCC('D', 'I', 'V', '4')
#define FCC_div5 FCC('d', 'i', 'v', '5')
#define FCC_DIV5 FCC('D', 'I', 'V', '5')
#define FCC_div6 FCC('d', 'i', 'c', '6')
#define FCC_DIV6 FCC('D', 'I', 'V', '6')
#define FCC_MP41 FCC('M', 'P', '4', '1')
#define FCC_mp43 FCC('m', 'p', '4', '3')
#define FCC_MP43 FCC('M', 'P', '4', '3')
//   DivX 4
#define FCC_divx FCC('d', 'i', 'v', 'x')
#define FCC_DIVX FCC('D', 'I', 'V', 'X')
//   DivX 5
#define FCC_DX50 FCC('D', 'X', '5', '0')
#define FCC_dx50 FCC('d', 'x', '5', '0')

#endif
