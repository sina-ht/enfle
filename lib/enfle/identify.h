/*
 * identify.c -- File or stream identify utility functions
 * (C)Copyright 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Jul 29 03:31:40 2001.
 * $Id: identify.h,v 1.1 2001/07/29 00:38:18 sian Exp $
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
