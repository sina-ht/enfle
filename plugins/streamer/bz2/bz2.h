/*
 * bz2.h -- bz2 streamer plugin header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Sep 30 05:48:38 2000.
 * $Id: bz2.h,v 1.1 2000/09/30 17:36:36 sian Exp $
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

#ifndef _BZ2_H
#define _BZ2_H

/* API functions */
#ifdef BZAPI_NEEDS_PREFIX
# define BZOPEN  BZ2_bzopen
# define BZREAD  BZ2_bzread
# define BZERROR BZ2_bzerror
# define BZCLOSE BZ2_bzclose
#else
# define BZOPEN  bzopen
# define BZREAD  bzread
# define BZERROR bzerror
# define BZCLOSE bzclose
#endif

#endif
