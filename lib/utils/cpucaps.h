/*
 * cpucaps.h -- Check if MMX support is available via cpuid, not using /proc/cpuinfo.
 * (C)Copyright 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Jul 24 00:00:47 2004.
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

#ifndef _CPUCAPS_H
#define _CPUCAPS_H

#ifdef __i386__
typedef enum _cpucap {
  _TSC = 1,
  _MMX = 2,
  _MMXEXT = 4,
  _SSE = 4,
  _AMD_MMXEXT = 4,
  _SSE2 = 8,
  _3DNOW = 16,
  _SSE3 = 32,
} CPUCap;

#define cpucaps_is_tsc(c) ((c) & _TSC)
#define cpucaps_is_mmx(c) ((c) & _MMX)
#define cpucaps_is_mmxext(c) ((c) & _MMXEXT)
#define cpucaps_is_3dnow(c) ((c) & _3DNOW)

#else
typedef enum _cpucap {
  _NONE = 1
} CPUCap;
#endif

typedef unsigned int CPUCaps;

int cpucaps_freq(void);
CPUCaps cpucaps_get(void);

#endif
