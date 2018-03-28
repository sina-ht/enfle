/*
 * misc.h -- miscellaneous routines header
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Sep 13 02:52:57 2003.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _ENFLE_MISC_H
#define _ENFLE_MISC_H

char *misc_basename(char *);
char *misc_get_ext(const char *, int);
char *misc_trim_ext(const char *, const char *);
char *misc_replace_ext(char *, char *);
char *misc_canonical_pathname(char *);
char **misc_str_split(char *, char);
char **misc_str_split_delimiters(char *, char *, char **);
void misc_free_str_array(char **);
char *misc_str_tolower(char *);
char *misc_remove_preceding_space(char *);

#endif
