/*
 * compat.h -- for compatibility
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Nov 10 01:49:01 2003.
 * $Id: compat.h,v 1.11 2003/11/17 13:23:51 sian Exp $
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

#ifndef WITH_DMALLOCTH
#ifdef HAVE_MEMALIGN
#include <sys/types.h>
void *memalign(size_t, size_t);
#else
#define memalign(align, size) malloc(size)
#endif
#endif

#ifndef HAVE_GETPAGESIZE
# define getpagesize() 1024
#endif

#ifdef REQUIRE_FNMATCH_H
# ifndef HAVE_FNMATCH
#  include "compat/fnmatch.h"
# else
#  include <fnmatch.h>
# endif
#endif
