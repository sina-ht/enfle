/*
 * plugin.h -- plugin interface header
 * (C)Copyright 2000, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Mar  9 22:28:56 2004.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _PLUGIN_H
#define _PLUGIN_H

typedef struct _plugin Plugin;
struct _plugin {
  void *handle;
  void *substance;
  void (*substance_unload)(void *);
  char *filepath;
  char *err;
};

Plugin *plugin_create(void);
Plugin *plugin_create_from_static(void *(*)(void), void (*)(void *));
int plugin_load(Plugin *, char *, const char *, const char *);
int plugin_autoload(Plugin *, char *);
void *plugin_get(Plugin *);
int plugin_unload(Plugin *);
void plugin_destroy(Plugin *);

#endif
