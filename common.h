/*
 * common.h -- common header file, which is included by almost all files.
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Wed Jan  2 00:00:46 2002.
 * $Id: common.h,v 1.20 2002/01/02 11:23:01 sian Exp $
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
#ifndef CONFIG_H_INCLUDED
#  undef HAVE_STDLIB_H
#  undef HAVE_STDDEF_H
#endif

/* If compat.h didn't include, include config.h here. */
#ifdef HAVE_CONFIG_H
# ifndef CONFIG_H_INCLUDED
#  include "enfle-config.h"
#  define CONFIG_H_INCLUDED
# endif
#endif

#include <stdio.h>
#define show_message(format, args...) printf(format, ## args)
#define show_message_fn(format, args...) printf("%s", __FUNCTION__);printf(format, ## args)
#define show_message_fnc(format, args...) printf("%s: ", __FUNCTION__);printf(format, ## args)
#define warning(format, args...) printf("Warning: " format, ## args)

#ifdef REQUIRE_FATAL
#include <stdarg.h>
#include <stdlib.h>
static void fatal(int, const char *, ...) __attribute__ ((__noreturn__, __format__(printf, 2, 3)));
static void
fatal(int code, const char *format, ...)
{
  va_list args;

  va_start(args, format);
  fprintf(stderr, "*** " PACKAGE " FATAL: ");
  vfprintf(stderr, format, args);
  va_end(args);

  exit(code);
}
#define bug(code, format, args...) fatal(code, "BUG: " format, ## args)
#endif

#ifdef REQUIRE_FATAL_PERROR
static inline void fatal_perror(int, const char *) __attribute__ ((__noreturn__));
static inline void
fatal_perror(int code, const char *msg)
{
  perror(msg);
  exit(code);
}
#endif

#ifndef MAX
#define MAX(a, b) \
 ({typedef _ta = (a), _tb = (b); \
           _ta _a = (a); _tb _b = (b); \
           _a > _b ? _a : _b; })
#endif
#ifndef MIN
#define MIN(a, b) \
 ({typedef _ta = (a), _tb = (b); \
           _ta _a = (a); _tb _b = (b); \
           _a < _b ? _a : _b; })
#endif
#ifndef SWAP
#define SWAP(a, b) \
 {typedef _ta = a, _tb = b; \
          _ta *_a = &a; _tb *_b = &b; _ta _t; \
          _t = *_a; *_a = *_b; *_b = _t; }
#endif

#ifdef DEBUG
#  define PROGNAME PACKAGE "-debug"
#  define debug_message(format, args...) fprintf(stderr, format, ## args)
#  define debug_message_fn(format, args...) fprintf(stderr, "%s", __FUNCTION__);fprintf(stderr, format, ## args); 
#  define debug_message_fnc(format, args...) fprintf(stderr, "%s: ", __FUNCTION__);fprintf(stderr, format, ## args)
#  define IDENTIFY_BEFORE_OPEN
#  define IDENTIFY_BEFORE_LOAD
#  define IDENTIFY_BEFORE_PLAY
#else
#  define PROGNAME PACKAGE
#  define debug_message(format, args...)
#  define debug_message_fn(format, args...)
#  define debug_message_fnc(format, args...)
/* #  define IDENTIFY_BEFORE_OPEN */
/* #  define IDENTIFY_BEFORE_LOAD */
/* #  define IDENTIFY_BEFORE_PLAY */
#endif
#define COPYRIGHT_MESSAGE "(C)Copyright 2000, 2001 by Hiroshi Takekawa"

#ifdef WITH_DMALLOCTH
#  include <dmalloc.h>
#endif
