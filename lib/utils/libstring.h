/*
 * libstring.h -- String manipulation library header
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Aug 18 23:23:15 2002.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _LIBSTRING_H
#define _LIBSTRING_H

typedef struct _string String;

struct _string {
  unsigned int len;
  unsigned int buffer_size;
  char *buffer;
};

String *string_create(void);
int string_set(String *, const char *);
int string_copy(String *, String *);
int string_cat_ch(String *, char);
int string_ncat(String *, const char *, unsigned int);
int string_cat(String *, const char *);
int string_catf(String *, const char *, ...);
int string_append(String *, String *);
void string_shrink(String *, unsigned int);
String *string_dup(String *);
void string_destroy(String *);

#define string_buffer_size(s) (s)->buffer_size
#define string_buffer(s) (s)->buffer
#define string_length(s) (s)->len
#define string_get(s) string_buffer(s)

#endif
