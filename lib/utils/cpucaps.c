/*
 * cpucaps.c -- Check if MMX support is available via cpuid, not using /proc/cpuinfo.
 * (C)Copyright 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Sep  7 23:24:22 2001.
 * $Id: cpucaps.c,v 1.2 2001/09/10 11:57:24 sian Exp $
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

#include "cpucaps.h"

#ifdef __i386__
static void
cpuid(int op, unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx)
{
  asm("pushl %%ebx\n\t"
      "cpuid\n\t"
      "movl %%ebx, %%esi\n\t"
      "popl %%ebx"
      : "=a" (*eax), "=S" (*ebx), "=c" (*ecx), "=d" (*edx)
      : "a"  (op));
}

CPUCaps
cpucaps_get(void)
{
  unsigned int eax, ebx, ecx, edx;
  int is_amd;
  unsigned int caps = 0;

  asm("pushfl\n\t"
      "popl %0\n\t"
      "movl %0,%1\n\t"
      "xorl $0x200000,%0\n\t"
      "pushl %0\n\t"
      "popfl\n\t"
      "pushfl\n\t"
      "popl %0"
      : "=a" (eax),
        "=c" (ebx));

  /* CPUID exists? */
  if (eax == ebx)
    return 0;

  /* Fetch vendor string and caps */
  cpuid(0, &eax, &ebx, &ecx, &edx);

  /* Capabilities exist? */
  if (!eax)
    return 0;

  /* AuthenticAMD -> h t u A                D M A c                i t n e */
  is_amd = (ebx == 0x68747541) && (ecx == 0x444d4163) && (edx == 0x69746e65);
  cpuid(1, &eax, &ebx, &ecx, &edx);

  if (!(edx & 0x00800000))
    return 0;
  caps = _MMX;

  if (edx & 0x02000000)
    caps |= _SSE;

  /* Extended capabilities exist? */
  cpuid (0x80000000, &eax, &ebx, &ecx, &edx);
  if (eax < 0x80000001)
    return caps;

  cpuid (0x80000001, &eax, &ebx, &ecx, &edx);
  if (edx & 0x80000000)
    caps |= _3DNOW;

  if (is_amd && (edx & 0x00400000))
    caps |= _AMD_MMXEXT;

  return caps;
}
#else
CPUCaps
cpucaps_get(void)
{
  return 0;
}
#endif
