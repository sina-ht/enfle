/*
 * stream-utils.h -- stream utility header
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Feb 15 03:42:50 2002.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _STREAM_UTILS_H
#define _STREAM_UTILS_H

#include "stream.h"

int stream_read_little_uint32(Stream *, unsigned int *);
int stream_read_big_uint32(Stream *, unsigned int *);
int stream_read_little_uint16(Stream *, unsigned short int *);
int stream_read_big_uint16(Stream *, unsigned short int *);
char *stream_gets(Stream *);
char *stream_ngets(Stream *, char *, int);
int stream_getc(Stream *);

#endif
