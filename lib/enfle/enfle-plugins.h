/*
 * enfle-plugins.h -- enfle plugin interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Oct 17 22:57:00 2000.
 * $Id: enfle-plugins.h,v 1.1 2000/10/17 14:04:01 sian Exp $
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

#ifndef _ENFLE_PLUGINS_H
#define _ENFLE_PLUGINS_H

#include "pluginlist.h"
#include "enfle-plugintype.h"

typedef struct _enfle_plugins EnflePlugins;
struct _enfle_plugins {
  PluginList **pls;

  char *(*load)(EnflePlugins *, char *, PluginType *);
  int (*unload)(EnflePlugins *, PluginType, char *);
  void *(*get)(EnflePlugins *, PluginType, char *);
  void (*destroy)(EnflePlugins *);
  Dlist *(*get_names)(EnflePlugins *, PluginType);
  unsigned char *(*get_description)(EnflePlugins *, PluginType, char *);
  unsigned char *(*get_author)(EnflePlugins *, PluginType, char *);
};

#define enfle_plugins_load(eps, p, pt) (eps)->load((eps), (p), (pt))
#define enfle_plugins_unload(eps, pt, n) (eps)->unload((eps), (pt), (n))
#define enfle_plugins_get(eps, pt, n) (eps)->get((eps), (pt), (n))
#define enfle_plugins_destroy(eps) (eps)->destroy((eps))
#define enfle_plugins_get_names(eps, pt) (eps)->get_names((eps), (pt))
#define enfle_plugins_get_description(eps, pt, n) (eps)->get_description((eps), (pt), (n))
#define enfle_plugins_get_author(eps, pt, n) (eps)->get_author((eps), (pt), (n))

EnflePlugins *enfle_plugins_create(void);

#endif
