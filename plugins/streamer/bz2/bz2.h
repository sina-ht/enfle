/*
 * bz2.h -- bz2 streamer plugin header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Sep  2 11:37:37 2001.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _BZ2_H
#define _BZ2_H

/* API functions */
#ifdef BZAPI_NEEDS_PREFIX
# define BZLIBVERSION BZ2_bzlibVersion
# define BZOPEN  BZ2_bzopen
# define BZREAD  BZ2_bzread
# define BZERROR BZ2_bzerror
# define BZCLOSE BZ2_bzclose
#else
# define BZLIBVERSION bzlibVersion
# define BZOPEN  bzopen
# define BZREAD  bzread
# define BZERROR bzerror
# define BZCLOSE bzclose
#endif

#endif
