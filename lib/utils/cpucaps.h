/*
 * cpucaps.h -- Check if MMX support is available via cpuid, not using /proc/cpuinfo.
 * (C)Copyright 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Wed Sep 19 16:22:35 2001.
 * $Id: cpucaps.h,v 1.2 2001/09/19 07:45:36 sian Exp $
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

#ifndef _CPUCAPS_H
#define _CPUCAPS_H

#ifdef __i386__
typedef enum _cpucap {
  _MMX = 1,
  _MMXEXT = 2,
  _SSE = 2,
  _AMD_MMXEXT = 2,
  _SSE2 = 4,
  _3DNOW = 8
} CPUCap;

#define cpucaps_is_mmx(c) ((c) & _MMX)
#define cpucaps_is_mmxext(c) ((c) & _MMXEXT)
#define cpucaps_is_3dnow(c) ((c) & _3DNOW)

#else
typedef enum _cpucap {
} CPUCap;
#endif

typedef unsigned int CPUCaps;

CPUCaps cpucaps_get(void);

#endif
