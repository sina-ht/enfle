/*
 * utils.h -- utility functions header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Thu Dec 28 07:05:51 2000.
 * $Id: utils.h,v 1.3 2000/12/27 23:29:29 sian Exp $
 *
 * Note: common.h should be included before including this header.
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

#ifndef _UTILS_H
#define _UTILS_H

static inline unsigned int
utils_get_little_uint32(unsigned char *p)
{
#ifdef WORDS_BIGENDIAN
  return (((((p[3] << 8) | p[2]) << 8) | p[1]) << 8) | p[0];
#else
  unsigned int *v;

  v = (unsigned int *)p;
  return *v;
#endif
}

static inline unsigned short int
utils_get_little_uint16(unsigned char *p)
{
#ifdef WORDS_BIGENDIAN
  return (p[1] << 8) | p[0];
#else
  unsigned short int *v;

  v = (unsigned short int *)p;
  return *v;
#endif
}

static inline unsigned int
utils_get_big_uint32(unsigned char *p)
{
#ifndef WORDS_BIGENDIAN
  return (((((p[0] << 8) | p[1]) << 8) | p[2]) << 8) | p[3];
#else
  unsigned int *v;

  v = (unsigned int *)p;
  return *v;
#endif
}

static inline unsigned short int
utils_get_big_uint16(unsigned char *p)
{
#ifndef WORDS_BIGENDIAN
  return (p[0] << 8) | p[1];
#else
  unsigned short int *v;

  v = (unsigned short int *)p;
  return *v;
#endif
}

#endif
