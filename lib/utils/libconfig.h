/*
 * libconfig.h -- configuration file manipulation library header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Apr 14 05:49:52 2001.
 * $Id: libconfig.h,v 1.3 2001/04/18 05:32:07 sian Exp $
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

#ifndef _LIBCONFIG_H
#define _LIBCONFIG_H

#include "hash.h"

typedef struct _config Config;

struct _config {
  Hash *hash;

  int (*load)(Config *, const char *);
  int (*save)(Config *, char *);
  void *(*get)(Config *, const char *);
  int (*set)(Config *, char *, void *);
  unsigned char *(*get_str)(Config *, const char *);
  int (*set_str)(Config *, char *, unsigned char *);
  int (*get_int)(Config *, const char *, int *);
  int (*set_int)(Config *, char *, int);
  void (*destroy)(Config *);
};

#define config_load(c, p) (c)->load((c), (p))
#define config_save(c, p) (c)->save((c), (p))
#define config_get(c, p) (c)->get((c), (p))
#define config_set(c, p, d) (c)->set((c), (p), (d))
#define config_get_str(c, p) (c)->get_str((c), (p))
#define config_set_str(c, p, d) (c)->set_str((c), (p), (d))
#define config_get_int(c, p, r) (c)->get_int((c), (p), (r))
#define config_set_int(c, p, d) (c)->set_int((c), (p), (d))
#define config_destroy(c) (c)->destroy((c))

Config *config_create(void);

/* TODO: should be replaced by extendable hash */
#define LIBCONFIG_HASH_SIZE 8192

#endif
