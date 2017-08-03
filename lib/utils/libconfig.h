/*
 * libconfig.h -- configuration file manipulation library header
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Jul  3 13:41:50 2005.
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

#ifndef _LIBCONFIG_H
#define _LIBCONFIG_H

#include "hash.h"

/* Must be prime */
#define LIBCONFIG_HASH_SIZE 8209

/* TODO: should be replaced by extensible hash */
typedef struct _config {
  Hash *hash;
} Config;

Config *config_create(void);
int config_load(Config *, const char *);
int config_save(Config *, char *);
int config_parse(Config *, char *);
void *config_get(Config *, const char *);
int config_set(Config *, char *, void *);
char *config_get_str(Config *, const char *);
int config_set_str(Config *, char *, char *);
int config_get_boolean(Config *, const char *, int *);
int config_set_boolean(Config *, char *, int);
int config_get_int(Config *, const char *, int *);
int config_set_int(Config *, char *, int);
char **config_get_list(Config *, const char *, int *);
int config_set_list(Config *, char *, char *);
void config_destroy(Config *);

#endif
