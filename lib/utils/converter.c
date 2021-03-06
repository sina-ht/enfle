/*
 * converter.c -- Character code converter
 * (C)Copyright 2001-2005 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Mar  6 12:21:18 2005.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include <stdio.h>
#include <errno.h>

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "converter.h"

#ifdef HAVE_ICONV

#define ICONV_OUTPUT_SIZE 65536
#include <iconv.h>

int
converter_convert(char *s, char **d_r, size_t insize, char *from, char *to)
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
      err_message("%s: conversion from %s to %s is not supported by iconv().\n", __FUNCTION__, from, to);
      *d_r = strdup(s);
      return strlen(s);
    }
    perror("converter_convert");
    return -errno;
  }

  inptr = s;
  outptr = d;
  avail = ICONV_OUTPUT_SIZE - 1;

  if ((nconv = iconv(cd, (ICONV_CONST char **)&inptr, &insize, &outptr, &avail)) != (size_t)-1) {
    /* For state-dependent character sets we have to flush the state now. */
    iconv(cd, NULL, NULL, &outptr, &avail);
    *outptr = '\0';
    *d_r = strdup(d);
  } else {
    //debug_message_fnc("iconv error occurred: nconv = %d.\n", nconv);
    switch (errno) {
    case E2BIG:
      debug_message("Increase ICONV_OUTPUT_SIZE and recompile.\n");
      break;
    case EILSEQ:
      //debug_message_fnc("Invalid sequence passed: %s\n", inptr);
      break;
    case EINVAL:
      debug_message("Incomplete multi-byte sequence passed.\n");
      break;
    default:
      perror("converter_convert");
      break;
    }
    *outptr = '\0';
    *d_r = NULL;
    iconv_close(cd);
    return -errno;
  }

  iconv_close(cd);

  return strlen(*d_r);
}
#else
int
converter_convert(char *s, char **d_r, size_t insize __attribute__((unused)), char *from __attribute__((unused)), char *to __attribute__((unused)))
{
  *d_r = strdup(s);
  return strlen(*d_r);
}
#endif
