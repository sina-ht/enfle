/*
 * stream-utils.h -- stream utility header
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Feb 20 20:10:41 2001.
 * $Id: stream-utils.h,v 1.2 2001/02/20 13:55:41 sian Exp $
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

#ifndef _STREAM_UTILS_H
#define _STREAM_UTILS_H

#include "stream.h"

int stream_read_little_uint32(Stream *, unsigned int *);
int stream_read_big_uint32(Stream *, unsigned int *);
int stream_read_little_uint16(Stream *, unsigned short int *);
int stream_read_big_uint16(Stream *, unsigned short int *);

#endif
