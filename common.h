/*
 * common.h -- common header file, which is included by almost all files.
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Aug 18 20:55:21 2002.
 * $Id: common.h,v 1.26 2002/08/18 13:25:16 sian Exp $
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
#define show_message_fn(format, args...) printf("%s" format, __FUNCTION__ , ## args)
#define show_message_fnc(format, args...) printf("%s: " format, __FUNCTION__ , ## args)
#define warning(format, args...) printf("Warning: " format, ## args)
#define warning_fn(format, args...) printf("Warning: %s" format, __FUNCTION__ , ## args)
#define warning_fnc(format, args...) printf("Warning: %s: " format, __FUNCTION__ , ## args)
#define err_message(format, args...) fprintf(stderr, "Error: " format, ## args)
#define err_message_fn(format, args...) fprintf(stderr, "Error: %s" format, __FUNCTION__ , ## args)
#define err_message_fnc(format, args...) fprintf(stderr, "Error: %s: " format, __FUNCTION__ , ## args)

#if __GNUC__ == 2 && __GNUC_MINOR__ < 96
#define likely(x)   (x)
#define unlikely(x) (x)
#else
#define likely(x)   __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)
#endif

#include <signal.h>
#define __fatal(msg, format, args...) \
  do {fprintf(stderr, "%s" format, msg, ## args); raise(SIGABRT); exit(1);} while (0)
#define fatal(format, args...) __fatal(PACKAGE " FATAL ERROR: ", format, ## args)
#define fatal_perror(msg) do { perror(msg); raise(SIGABRT); exit(1); } while (0)
#define bug(format, args...) __fatal(PACKAGE " BUG: ", format, ## args) 
#define bug_on(cond) do { if (unlikely(cond)) __fatal(PACKAGE " BUG: cond: ", "%s", #cond); } while (0)

#if defined(__GNUC__)
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
#else
#define MAX(a,b) ((a>b)?a:b)
#define MIX(a,b) ((a<b)?a:b)
#define SWAP_T(a,b,t) { t __tmp = (a); a = (b); b = __tmp; }
#define SWAP(a,b) SWAP_T(a,b,int)
#endif

#ifdef DEBUG
#  define PROGNAME PACKAGE "-debug"
#  define debug_message(format, args...) fprintf(stderr, format, ## args)
#  define debug_message_fn(format, args...) fprintf(stderr, "%s" format, __FUNCTION__ , ## args)
#  define debug_message_fnc(format, args...) fprintf(stderr, "%s: " format, __FUNCTION__ , ## args)
#  define IDENTIFY_BEFORE_OPEN
#  define IDENTIFY_BEFORE_LOAD
#  define IDENTIFY_BEFORE_PLAY
#else
#  define PROGNAME PACKAGE
#  define debug_message(format, args...)
#  define debug_message_fn(format, args...)
#  define debug_message_fnc(format, args...)
#endif
#define COPYRIGHT_MESSAGE "(C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa"

#ifdef WITH_DMALLOCTH
#  include <dmalloc.h>
#endif
