/*
 * getopt-support.h -- getopt support header
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Jul 17 03:50:39 2005.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _GETOPT_SUPPORT_H
#define _GETOPT_SUPPORT_H

typedef enum _argument_requirement {
  _NO_ARGUMENT,
  _REQUIRED_ARGUMENT,
  _OPTIONAL_ARGUMENT
} ArgumentRequirement;

typedef struct _option {
  const char *longopt;
  char opt;
  ArgumentRequirement argreq;
  const char *description;
} Option;

char *gen_optstring(Option [], struct option **);
void print_option_usage(Option []);

#endif
