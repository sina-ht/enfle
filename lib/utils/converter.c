/*
 * converter.c -- Character code converter
 * (C)Copyright 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Oct  7 00:25:22 2001.
 * $Id: converter.c,v 1.1 2001/10/06 16:00:24 sian Exp $
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

#include <stdio.h>
#include <errno.h>

#define REQUIRE_STRING_H
#include "compat.h"

#ifdef USE_ICONV

#include "converter.h"

#define ICONV_OUTPUT_SIZE 65536
#ifndef USE_GICONV
# include <iconv.h>
#else
# include <giconv.h>
#endif

int converter_convert(char *s, char **d_r, size_t insize, char *from, char *to)
{
  iconv_t cd;
  char d[ICONV_OUTPUT_SIZE];
  char *inptr, *outptr;
  size_t avail, nconv;

  if (strcasecmp(from, "noconv") == 0) {
    *d_r = strdup(s);
    return strlen(s);
  }
  if (!s) {
    *d_r = NULL;
    return 0;
  }
  if (insize == 0) {
    *d_r = strdup("");
    return 0;
  }

  if ((cd = iconv_open(to, from)) == (iconv_t)-1) {
    if (errno == EINVAL) {
      *d_r = strdup(s);
      return strlen(s);
    }
    perror("converter_convert");
    return errno;
  }

  inptr = s;
  outptr = d;
  avail = ICONV_OUTPUT_SIZE - 1;

  if ((nconv = iconv(cd, (char **)&inptr, &insize, &outptr, &avail)) != (size_t)-1) {
    *outptr = '\0';
    *d_r = strdup(d);
  } else {
    switch (errno) {
    case E2BIG:
      fprintf(stderr, "Increase ICONV_OUTPUT_SIZE and recompile.\n");
      break;
    case EILSEQ:
      fprintf(stderr, "Invalid sequence passed.\n");
      break;
    case EINVAL:
      fprintf(stderr, "Incomplete multi-byte sequence passed.\n");
      break;
    default:
      perror("converter_convert");
      break;
    }
    *d_r = strdup(s);
  }

  iconv_close(cd);

  return strlen(*d_r);
}
#endif
