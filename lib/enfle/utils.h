/*
 * utils.h -- utility functions header
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat May  4 23:25:47 2002.
 * $Id: utils.h,v 1.6 2002/05/04 14:36:00 sian Exp $
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _UTILS_H
#define _UTILS_H

#if !(defined(UTILS_NEED_GET_LITTLE_UINT32) || defined(UTILS_NEED_GET_LITTLE_UINT16) || defined(UTILS_NEED_GET_BIG_UINT32)    || defined(UTILS_NEED_GET_BIG_UINT16))
#error No UTILS_NEED_* defined
#endif

#ifdef UTILS_NEED_GET_LITTLE_UINT32
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
#endif

#ifdef UTILS_NEED_GET_LITTLE_UINT16
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
#endif

#ifdef UTILS_NEED_GET_BIG_UINT32
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
#endif

#ifdef UTILS_NEED_GET_BIG_UINT16
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

#endif
