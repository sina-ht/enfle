/*
 * saver.c -- Saver plugin interface
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Feb 18 01:40:20 2002.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include "compat.h"
#include "common.h"

#include "saver.h"
#include "saver-plugin.h"

int
saver_save(EnflePlugins *eps, char *pluginname, Image *src, FILE *fp, Config *c, void *params)
{
  Plugin *p;
  SaverPlugin *sp;

  if ((p = pluginlist_get(eps->pls[ENFLE_PLUGIN_SAVER], pluginname)) == NULL)
    return 0;
  sp = plugin_get(p);
  if (sp)
    return sp->save(src, fp, c, params);
  return 0;
}

char *
saver_get_ext(EnflePlugins *eps, char *pluginname, Config *c)
{
  Plugin *p;
  SaverPlugin *sp;

  if ((p = pluginlist_get(eps->pls[ENFLE_PLUGIN_SAVER], pluginname)) == NULL)
    return 0;
  sp = plugin_get(p);
  if (sp)
    return sp->get_ext(c);
  return 0;
}
