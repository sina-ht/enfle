/*
 * compat.h -- for compatibility
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Jul 17 01:17:10 2005.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

/* jconfig.h defines this. */
#undef HAVE_STDLIB_H
#undef HAVE_STDDEF_H

#include <signal.h>
#if defined(inline)
#undef inline
#endif

/*
 * Should check HAVE_STDLIB_H.
 * But autoconf bugs icc... so stdlib.h must be included before config.
 */
#include <stdlib.h>

#ifdef HAVE_CONFIG_H
# ifndef CONFIG_H_INCLUDED
#  include "enfle-config.h"
#  define CONFIG_H_INCLUDED
# endif
#endif

#ifdef REQUIRE_STRING_H
# if HAVE_STRING_H
#  if !STDC_HEADERS && HAVE_MEMORY_H
#   include <memory.h>
#  endif
#  include <string.h>
# elif HAVE_STRINGS_H
#  include <strings.h>
# elif !HAVE_MEMCPY
#  define memcpy(d, s, n) bcopy((s), (d), (n))
#  define memmove(d, s, n) bcopy((s), (d), (n))
# endif
#endif

#ifdef REQUIRE_UNISTD_H
# ifdef HAVE_UNISTD_H
#  include <sys/types.h>
#  include <unistd.h>
# endif
#endif

#ifdef REQUIRE_DIRENT_H
# if HAVE_DIRENT_H
#  include <dirent.h>
# else
#  define dirent direct
#  if HAVE_SYS_NDIR_H
#   include <sys/ndir.h>
#  endif
#  if HAVE_SYS_DIR_H
#   include <sys/dir.h>
#  endif
#  if HAVE_NDIR_H
#   include <ndir.h>
#  endif
# endif
#endif

#ifdef HAVE_MEMALIGN
#include <sys/types.h>
void *memalign(size_t, size_t);
#else
#define memalign(align, size) malloc(size)
#endif

#ifndef HAVE_GETPAGESIZE
# define getpagesize() 1024
#endif

#ifdef REQUIRE_FNMATCH_H
#  include <fnmatch.h>
#endif

#ifdef HAVE_GETOPT_LONG
#  include <getopt.h>
#else
struct option {
  const char *name;
  int has_arg;
  int *flag;
  int val;
};
#endif
