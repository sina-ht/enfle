/*
 * identify.c -- File or stream identify utility functions
 * (C)Copyright 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Aug 11 05:31:04 2001.
 * $Id: identify.h,v 1.2 2001/08/15 06:39:03 sian Exp $
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

#ifndef _ENFLE_IDENTIFY_H
#define _ENFLE_IDENTIFY_H

#include "enfle-plugins.h"
#include "stream.h"
#include "archive.h"
#include "image.h"
#include "movie.h"
#include "video.h"
#include "utils/libconfig.h"

#define IDENTIFY_FILE_STREAM       0
#define IDENTIFY_FILE_DIRECTORY    1
#define IDENTIFY_FILE_NOTREG       2
#define IDENTIFY_FILE_SOPEN_FAILED 3
#define IDENTIFY_FILE_AOPEN_FAILED 4
#define IDENTIFY_FILE_STAT_FAILED  5

#define IDENTIFY_STREAM_MOVIE_FAILED -2
#define IDENTIFY_STREAM_IMAGE_FAILED -1
#define IDENTIFY_STREAM_FAILED        0
#define IDENTIFY_STREAM_IMAGE         1
#define IDENTIFY_STREAM_MOVIE         2

int identify_file(EnflePlugins *, char *, Stream *, Archive *);
int identify_stream(EnflePlugins *, Image *, Movie *, Stream *, VideoWindow *, Config *);

#endif
