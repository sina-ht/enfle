/*
 * cpucaps.c -- CPU identification, not using /proc/cpuinfo.
 * (C)Copyright 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Apr 18 01:35:26 2004.
 * $Id: cpucaps.c,v 1.7 2004/04/18 06:29:04 sian Exp $
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

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "cpucaps.h"

static int
cpufreq_default(void)
{
  return 1024 * 1024;
}
#if defined(__linux__)
int
cpucaps_freq(void)
{
  FILE *fp;
  char buf[256];
  static int freq = 0;

  if (freq)
    return freq;

  freq = cpufreq_default();
  if ((fp = fopen("/proc/cpuinfo", "rb")) == NULL)
    return freq;
  while (fgets(buf, 256, fp) != NULL) {
    if (strncmp(buf, "cpu MHz", 7) == 0) {
      char *s;

      if ((s = strchr(buf, ':')) == NULL)
	break;
      s += 2;
      freq = strtof(s, NULL) * 1000;
      break;
    }
  }
  fclose(fp);

  return freq;
}
#else
int
cpucaps_freq(void)
{
  return cpufreq_default();
}
#endif

#if defined(__i386__)
static inline void
cpuid(int op, unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx)
{
  __asm__ ("pushl %%ebx\n\t"
		       "cpuid\n\t"
		       "movl %%ebx, %%esi\n\t"
		       "popl %%ebx"
		       : "=a" (*eax), "=S" (*ebx), "=c" (*ecx), "=d" (*edx)
		       : "a"  (op));
}

CPUCaps
cpucaps_get(void)
{
  unsigned int eax, ebx, ecx, edx, caps = 0;
  int i, is_i486_or_higher, is_cpuid_available, is_amd;
  char vendor_string[13];

  /* 386 always clears AC bit when set. */
  __asm__ ("pushfl\n\t"
	   "popl %0\n\t"
	   "movl %0, %%ecx\n\t"
	   "xorl $0x40000, %0\n\t" /* Toggle AC bit */
	   "pushl %0\n\t"
	   "popfl\n\t"
	   "pushfl\n\t"
	   "popl %0\n\t"
	   "xorl %%ecx, %0\n\t"
	   "pushl %%ecx\n\t"
	   "popfl\n\t"
	   : "=r" (is_i486_or_higher)
	   :
	   : "cx");

  if (!is_i486_or_higher)
    /* I'm i386:-) */
    return 0;

  /* Try to toggle CPUID bit to see if cpuid instruction is available. */
  __asm__ ("pushfl\n\t"
	   "popl %0\n\t"
	   "movl %0, %%ecx\n\t"
	   "xorl $0x200000, %0\n\t" /* Toggle CPUID bit */
	   "pushl %0\n\t"
	   "popfl\n\t"
	   "pushfl\n\t"
	   "popl %0\n\t"
	   "xorl %%ecx, %0\n\t"
	   "pushl %%ecx\n\t"
	   "popfl\n\t"
	   : "=r" (is_cpuid_available)
	   :
	   : "cx");

  if (!is_cpuid_available)
    /* Older i486 or such */
    return 0;

  /* Fetch vendor string */
  cpuid(0, &eax, &ebx, &ecx, &edx);

  /* Capabilities exist? */
  if (!eax)
    return 0;

  /* Construct vendor string */
  i = 0;
  vendor_string[ 0] =  ebx        & 0xff;  vendor_string[ 1] = (ebx >>  8) & 0xff;
  vendor_string[ 2] = (ebx >> 16) & 0xff;  vendor_string[ 3] = (ebx >> 24) & 0xff;
  vendor_string[ 4] =  edx        & 0xff;  vendor_string[ 5] = (edx >>  8) & 0xff;
  vendor_string[ 6] = (edx >> 16) & 0xff;  vendor_string[ 7] = (edx >> 24) & 0xff;
  vendor_string[ 8] =  ecx        & 0xff;  vendor_string[ 9] = (ecx >>  8) & 0xff;
  vendor_string[10] = (ecx >> 16) & 0xff;  vendor_string[11] = (ecx >> 24) & 0xff;
  vendor_string[12] = '\0';
  debug_message("CPU vendor string: %s\n", vendor_string);
  is_amd = (strcmp(vendor_string, "AuthenticAMD") == 0);

  /* Fetch caps */
  cpuid(1, &eax, &ebx, &ecx, &edx);
  debug_message("CPU feature flag: %4X\n", edx);

  /*
0   FPU       Floating Point Unit
1   VME       V86 Mode Extensions
2   DE        Debug Extensions - I/O breakpoints
3   PSE       Page Size Extensions (4 MB pages)
4   TSC       Time Stamp Counter and RDTSC instruction
5   MSR       Model Specific Registers
6   PAE       Physical Address Extensions (36-bit address, 2MB pages)
7   MCE       Machine Check Exception
8   CX8       CMPXCHG8B instruction available
9   APIC      Local APIC present (multiprocesssor operation support)
              AMD K5 model 0 (SSA5) only: global pages supported !
10            reserved (Fast System Call on AMD K6 model 6 and Cyrix)
11  SEP       Fast system call (SYSENTER and SYSEXIT instructions)
              - (on Intel CPUs only if signature >= 0633!)
12  MTRR      Memory Type Range Registers
13  PGE       Page Global Enable - global oages support
14  MCA       Machine Check Architecture and MCG_CAP register
15  CMOV      Conditional MOVe instructions
16  PAT       Page Attribute Table (MSR 277h)
17  PSE36     36 bit Page Size Extenions (36-bit addressing for 4 MB pages with 4-byte descriptors)
18  PSN       Processor Serial Number
19  CFLSH     Cache Flush
20            ?
21  DTES      Debug Trace Store
22  ACPI      ACPI support
23  MMX       MultiMedia Extensions
24  FXSR      FXSAVE and FXRSTOR instructions
25  SSE       SSE instructions (introduced in Pentium III)
26  SSE2      SSE2 (WNI) instructions (introduced in Pentium 4)
27  SELFSNOOP
28            ?
29  ACC       Automatic clock control
30  IA64      IA64 instructions?
31            ?
  */

  if (edx & 0x00000010)
    caps |= _TSC;

  if (!(edx & 0x00800000))
    return 0;
  caps |= _MMX;

  if (edx & 0x02000000)
    caps |= _SSE;
  if (edx & 0x04000000)
    caps |= _SSE2;

  /* Extended capabilities exist? */
  cpuid (0x80000000, &eax, &ebx, &ecx, &edx);
  debug_message("CPU extended feature: %4X\n", eax);
  if (eax < 0x80000001)
    return caps;

  cpuid (0x80000001, &eax, &ebx, &ecx, &edx);
  debug_message("CPU extended feature flag: %4X\n", edx);
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
