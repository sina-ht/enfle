/*
 * common.h -- common header file, which is included by almost all files.
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Oct 21 01:54:00 2000.
 * $Id: common.h,v 1.5 2000/10/20 18:13:57 sian Exp $
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

/* If compat.h didn't include, include config.h here. */
#ifdef HAVE_CONFIG_H
# ifndef CONFIG_H_INCLUDED
#  define CONFIG_H_INCLUDED
#  include "config.h"
# endif
#endif

#include <stdio.h>
#define show_message(format, args...) printf(format, ## args)

#ifdef DEBUG
#  define PROGNAME PACKAGE "-debug"
#  define debug_message(format, args...) fprintf(stderr, format, ## args)
#  define IDENTIFY_BEFORE_OPEN
#  define IDENTIFY_BEFORE_LOAD
#  define IDENTIFY_BEFORE_PLAY
#else
#  define PROGNAME PACKAGE
#  define debug_message(format, args...)
/* #  define IDENTIFY_BEFORE_OPEN */
/* #  define IDENTIFY_BEFORE_LOAD */
/* #  define IDENTIFY_BEFORE_PLAY */
#endif

#ifdef WITH_DMALLOC
#  include <dmalloc.h>
#endif
