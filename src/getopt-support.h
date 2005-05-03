/*
 * getopt-support.h -- getopt support header
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue May  3 09:49:05 2005.
 * $Id: getopt-support.h,v 1.2 2005/05/03 01:08:30 sian Exp $
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

#ifndef _GETOPT_SUPPORT_H
#define _GETOPT_SUPPORT_H

typedef enum _argument_requirement {
  _NO_ARGUMENT,
  _REQUIRED_ARGUMENT,
  _OPTIONAL_ARGUMENT
} ArgumentRequirement;

typedef struct _option {
  const char *longopt; /* not supported so far */
  char opt;
  ArgumentRequirement argreq;
  const char *description;
} Option;

char *gen_optstring(Option []);
void print_option_usage(Option []);

#endif
