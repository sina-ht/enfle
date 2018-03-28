/*
 * libconfig.h -- configuration file manipulation library header
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Jul  3 13:41:50 2005.
 *
 * SPDX-License-Identifier: GPL-2.0-only
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
