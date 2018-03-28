/*
 * enfle-plugins.h -- enfle plugin interface header
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun May  1 16:43:20 2005.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _ENFLE_PLUGINS_H
#define _ENFLE_PLUGINS_H

#include "utils/pluginlist.h"
#include "enfle-plugintype.h"

typedef struct _enfle_plugins EnflePlugins;
struct _enfle_plugins {
  const char *path;
  char *cache_path;
  int cache_to_be_created;
  PluginList **pls;

  char *(*load)(EnflePlugins *, char *, PluginType *);
  int (*unload)(EnflePlugins *, PluginType, char *);
  char *(*add)(EnflePlugins *, void *(*)(void), void (*)(void *), PluginType *);
  void *(*get)(EnflePlugins *, PluginType, char *);
  void (*destroy)(EnflePlugins *);
  Dlist *(*get_names)(EnflePlugins *, PluginType);
  const char *(*get_description)(EnflePlugins *, PluginType, char *);
  const char *(*get_author)(EnflePlugins *, PluginType, char *);
};

#define enfle_plugins_load(eps, p, pt) (eps)->load((eps), (p), (pt))
#define enfle_plugins_unload(eps, pt, n) (eps)->unload((eps), (pt), (n))
#define enfle_plugins_add(eps, ent, ext, pt) (eps)->add((eps), (ent), (ext), (pt))
#define enfle_plugins_get(eps, pt, n) (eps)->get((eps), (pt), (n))
#define enfle_plugins_destroy(eps) (eps)->destroy((eps))
#define enfle_plugins_path(eps) (eps)->path
#define enfle_plugins_get_names(eps, pt) (eps)->get_names((eps), (pt))
#define enfle_plugins_get_description(eps, pt, n) (eps)->get_description((eps), (pt), (n))
#define enfle_plugins_get_author(eps, pt, n) (eps)->get_author((eps), (pt), (n))

extern EnflePlugins *global_enfle_plugins;
#define set_enfle_plugins(eps) global_enfle_plugins = eps
#define get_enfle_plugins() global_enfle_plugins

EnflePlugins *enfle_plugins_create(const char *, int);

#endif
