/*
 * enfle-plugins.c -- enfle plugin interface
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Mar 19 00:27:24 2004.
 * $Id: enfle-plugins.c,v 1.16 2004/03/24 14:50:37 sian Exp $
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
#include <errno.h>
#include <sys/stat.h>

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "utils/stdio-support.h"
#include "utils/misc.h"
#include "enfle-plugin.h"
#include "enfle-plugins.h"

EnflePlugins *global_enfle_plugins;

static char *load(EnflePlugins *, char *, PluginType *);
static int unload(EnflePlugins *, PluginType, char *);
static char *add(EnflePlugins *, void *(*)(void), void (*)(void *), PluginType *);
static void *get(EnflePlugins *, PluginType, char *);
static void destroy(EnflePlugins *);
static Dlist *get_names(EnflePlugins *, PluginType);
static const unsigned char *get_description(EnflePlugins *, PluginType, char *);
static const unsigned char *get_author(EnflePlugins *, PluginType, char *);

static EnflePlugins template = {
  .load = load,
  .unload = unload,
  .add = add,
  .get = get,
  .destroy = destroy,
  .get_names = get_names,
  .get_description = get_description,
  .get_author = get_author
};

EnflePlugins *
enfle_plugins_create(const char *path, int use_cache)
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
  eps->path = path;

  if (use_cache) {
    static const char *cache_file = ".cache";
    char cache_path[strlen(path) + strlen(cache_file) + 2];
    struct stat st;

    strcpy(cache_path, path);
    cache_path[strlen(path)] = '/';
    cache_path[strlen(path) + 1] = '\0';
    strcat(cache_path, cache_file);
    eps->cache_path = strdup(cache_path);
    if (use_cache == 1 && stat(cache_path, &st) == 0) {
      FILE *fp;

      if ((fp = fopen(cache_path, "rb")) == NULL) {
	err_message_fnc("Cannot open cache file %s\n", cache_path);
	goto error;
      } else {
	char *line;
	char **list;

	while ((line = stdios_gets(fp)) != NULL) {
	  Plugin *p;

	  if (line[strlen(line) - 1] == '\n')
	    line[strlen(line) - 1] = '\0';

	  if ((list = misc_str_split(line, ':')) == NULL) {
	    fclose(fp);
	    goto error;
	  }
	  
	  p = plugin_create();
	  plugin_autoload(p, list[2]);
	  if (!pluginlist_add(eps->pls[enfle_plugin_name_to_type(list[0])], p, list[1])) {
	    plugin_destroy(p);
	    fclose(fp);
	    goto error;
	  }
	  //debug_message_fnc("autoload: %s %s %s\n", list[0], list[1], list[2]);
	  misc_free_str_array(list);
	}
	if (!feof(fp)) {
	  fclose(fp);
	  goto error;
	}
	fclose(fp);
      }
    } else if (use_cache == 2 || errno == ENOENT) {
      eps->cache_to_be_created = 1;
    } else {
      err_message_fnc("Cannot open cache file %s\n", cache_path);
      goto error;
    }
  }

  return eps;

 error:
  free(eps->cache_path);
  eps->cache_path = NULL;
  return eps;
}

/* methods */

static char *
load(EnflePlugins *eps, char *path, PluginType *type_return)
{
  PluginList *pl;
  EnflePlugin *ep;
  Plugin *p = plugin_create();

  if (!plugin_load(p, path, "plugin_entry", "plugin_exit")) {
    show_message_fnc("%s\n", p->err);
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
  return (char *)ep->name;
}

static int
unload(EnflePlugins *eps, PluginType type, char *pluginname)
{
  Plugin *p;
  int f;

  if ((p = pluginlist_get(eps->pls[type], pluginname)) == NULL)
    return 0;

  f =  pluginlist_delete(eps->pls[type], pluginname);

  plugin_unload(p);

  return f;
}

static char *
add(EnflePlugins *eps, void *(*plugin_entry)(void), void (*plugin_exit)(void *), PluginType *type_return)
{
  Plugin *p = plugin_create_from_static(plugin_entry, plugin_exit);
  PluginList *pl;
  EnflePlugin *ep;

  if (!p)
    return NULL;

  ep = plugin_get(p);
  pl = eps->pls[ep->type];

  if (!pluginlist_add(pl, p, ep->name)) {
    plugin_destroy(p);
    return NULL;
  }

  *type_return = ep->type;
  return (char *)ep->name;
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
  free(eps->pls);
  free(eps);
}

static Dlist *
get_names(EnflePlugins *eps, PluginType type)
{
  return pluginlist_get_names(eps->pls[type]);
}

static const unsigned char *
get_description(EnflePlugins *eps, PluginType type, char *pluginname)
{
  Plugin *p;
  EnflePlugin *ep;

  if ((p = pluginlist_get(eps->pls[type], pluginname)) == NULL)
    return NULL;
  ep = plugin_get(p);

  return enfle_plugin_description(ep);
}

static const unsigned char *
get_author(EnflePlugins *eps, PluginType type, char *pluginname)
{
  Plugin *p;
  EnflePlugin *ep;

  if ((p = pluginlist_get(eps->pls[type], pluginname)) == NULL)
    return NULL;
  ep = plugin_get(p);

  return enfle_plugin_author(ep);
}
