/*
 * enfle-plugin.h -- enfle plugin interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun May  1 16:42:19 2005.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _ENFLE_PLUGIN_H
#define _ENFLE_PLUGIN_H

#include "enfle-plugintype.h"

#define ENFLE_PLUGIN_COMMON_DATA \
  PluginType type; \
  const char *name; \
  const char *description; \
  const char *author

typedef struct _enfle_plugin {
  ENFLE_PLUGIN_COMMON_DATA;
} EnflePlugin;

#define ENFLE_PLUGIN_ENTRIES \
  void *plugin_entry(void); \
  void plugin_exit(void *)

#ifdef STATIC
#define ENFLE_PLUGIN_ENTRY(pre) \
  void * pre ## _entry(void)
#else
#define ENFLE_PLUGIN_ENTRY(pre) \
  void *plugin_entry(void)
#endif

#ifdef STATIC
#define ENFLE_PLUGIN_EXIT(pre, p) \
  void pre ## _exit(void * p)
#else
#define ENFLE_PLUGIN_EXIT(pre, p) \
  void plugin_exit(void * p)
#endif

#define enfle_plugin_name(p) (p)->name
#define enfle_plugin_description(p) (p)->description
#define enfle_plugin_author(p) (p)->author

const char *enfle_plugin_type_to_name(PluginType);
PluginType enfle_plugin_name_to_type(char *);

#endif
