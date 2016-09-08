/*
 * rgbparse.h -- RGB database parser header
 * (C)Copyright 1999, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Feb 15 03:53:27 2002.
 * $Id: rgbparse.h,v 1.1 2002/02/14 19:20:16 sian Exp $
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _RGBPARSE_H
#define _RGBPARSE_H

#include "utils/hash.h"

typedef struct {
  int n;
  int c[3];
} NColor;

Hash *rgbparse(char *);

#endif
