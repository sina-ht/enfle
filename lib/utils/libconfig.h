/*
 * libconfig.h -- configuration file manipulation library header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Sep 18 07:14:41 2000.
 * $Id: libconfig.h,v 1.1 2000/09/30 17:36:36 sian Exp $
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

  int (*load)(Config *, char *);
  int (*save)(Config *, char *);
  void *(*get)(Config *, char *);
  int (*set)(Config *, char *, void *);
  void (*destroy)(Config *);
};

#define config_load(c, p) (c)->load((c), (p))
#define config_save(c, p) (c)->save((c), (p))
#define config_get(c, p) (c)->get((c), (p))
#define config_set(c, p, d) (c)->set((c), (p), (d))
#define config_destroy(c) (c)->destroy((c))

Config *config_create(void);

/* TODO: should be replaced by extendable hash */
#define LIBCONFIG_HASH_SIZE 8192

#endif
