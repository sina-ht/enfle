/*
 * common.h -- common header file, which is included by almost all files.
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Sep 29 05:00:02 2000.
 * $Id: common.h,v 1.1 2000/09/30 17:36:36 sian Exp $
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#define show_message(format, args...) printf(format, ## args)

#ifdef DEBUG
#  define PROGNAME PACKAGE "-debug"
#  define debug_message(format, args...) fprintf(stderr, format, ## args)
#  define IDENTIFY_BEFORE_OPEN
#  define IDENTIFY_BEFORE_LOAD
#else
#  define PROGNAME PACKAGE
#  define debug_message(format, args...)
/* #  undef IDENTIFY_BEFORE_OPEN */
/* #  undef IDENTIFY_BEFORE_LOAD */
#endif

#ifdef WITH_DMALLOC
#  include <dmalloc.h>
#endif
