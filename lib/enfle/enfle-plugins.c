/*
 * enfle-plugins.c -- enfle plugin interface
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Oct 17 22:59:23 2000.
 * $Id: enfle-plugins.c,v 1.1 2000/10/17 14:04:01 sian Exp $
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

#include <stdio.h>
#include <stdlib.h>

#include "common.h"

#include "enfle-plugin.h"
#include "enfle-plugins.h"

static char *load(EnflePlugins *, char *, PluginType *);
static int unload(EnflePlugins *, PluginType, char *);
static void *get(EnflePlugins *, PluginType, char *);
static void destroy(EnflePlugins *);
static Dlist *get_names(EnflePlugins *, PluginType);
static unsigned char *get_description(EnflePlugins *, PluginType, char *);
static unsigned char *get_author(EnflePlugins *, PluginType, char *);

static EnflePlugins template = {
  load: load,
  unload: unload,
  get: get,
  destroy: destroy,
  get_names: get_names,
  get_description: get_description,
  get_author: get_author
};

EnflePlugins *
enfle_plugins_create(void)
{
  EnflePlugins *eps;
  int i;

  if ((eps = (EnflePlugins *)calloc(1, sizeof(EnflePlugins))) == NULL)
    return NULL;
  memcpy(eps, &template, sizeof(EnflePlugins));

  if ((eps->pls = (PluginList **)calloc(ENFLE_PLUGIN_END, sizeof(PluginList *))) == NULL) {
    free(eps);
    return NULL;
  }

  for (i = 0; i < ENFLE_PLUGIN_END; i++)
    if ((eps->pls[i] = pluginlist_create()) == NULL) {
      for (i--; i >= 0; i--)
	pluginlist_destroy(eps->pls[i]);
      free(eps);
      return NULL;
    }

  return eps;
}

/* methods */

static char *
load(EnflePlugins *eps, char *path, PluginType *type_return)
{
  PluginList *pl;
  Plugin *p;
  EnflePlugin *ep;

  p = plugin_create();
  if (!plugin_load(p, path, "plugin_entry", "plugin_exit")) {
    plugin_destroy(p);
    return NULL;
  }

  ep = plugin_get(p);
  pl = eps->pls[ep->type];

  if (!pluginlist_add(pl, p, ep->name)) {
    plugin_destroy(p);
    return NULL;
  }

  *type_return = ep->type;
  return ep->name;
}

static int
unload(EnflePlugins *eps, PluginType type, char *pluginname)
{
  Plugin *p;

  if ((p = pluginlist_get(eps->pls[type], pluginname)) == NULL)
    return 0;
  plugin_unload(p);

  return pluginlist_delete(eps->pls[type], pluginname);
}

static void *
get(EnflePlugins *eps, PluginType type, char *pluginname)
{
  Plugin *p;

  if ((p = pluginlist_get(eps->pls[type], pluginname)) == NULL)
    return 0;
  return (void *)plugin_get(p);
}

static void
destroy(EnflePlugins *eps)
{
  int i;

  for (i = 0; i < ENFLE_PLUGIN_END; i++)
    pluginlist_destroy(eps->pls[i]);
  free(eps);
}

static Dlist *
get_names(EnflePlugins *eps, PluginType type)
{
  return pluginlist_get_names(eps->pls[type]);
}

static unsigned char *
get_description(EnflePlugins *eps, PluginType type, char *pluginname)
{
  Plugin *p;
  EnflePlugin *ep;

  if ((p = pluginlist_get(eps->pls[type], pluginname)) == NULL)
    return NULL;
  ep = plugin_get(p);

  return ep->description;
}

static unsigned char *
get_author(EnflePlugins *eps, PluginType type, char *pluginname)
{
  Plugin *p;
  EnflePlugin *ep;

  if ((p = pluginlist_get(eps->pls[type], pluginname)) == NULL)
    return NULL;
  ep = plugin_get(p);

  return ep->author;
}
