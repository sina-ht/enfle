/*
 * saver.c -- Saver plugin interface
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Apr 14 05:44:02 2001.
 * $Id: saver.c,v 1.1 2001/04/18 05:12:50 sian Exp $
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

#include <stdlib.h>

#include "common.h"

#include "saver.h"
#include "saver-plugin.h"

static int save(EnflePlugins *, char *, Image *, FILE *, void *);
static char *get_ext(EnflePlugins *, char *, Config *);

static Saver template = {
  save: save,
  get_ext: get_ext
};

Saver *
saver_create(void)
{
  Saver *saver;

  if ((saver = (Saver *)calloc(1, sizeof(Saver))) == NULL)
    return NULL;
  memcpy(saver, &template, sizeof(Saver));

  return saver;
}

static int
save(EnflePlugins *eps, char *pluginname, Image *src, FILE *fp, void *params)
{
  Plugin *p;
  SaverPlugin *sp;

  if ((p = pluginlist_get(eps->pls[ENFLE_PLUGIN_SAVER], pluginname)) == NULL)
    return 0;
  sp = plugin_get(p);
  if (sp)
    return sp->save(src, fp, params);
  return 0;
}

static char *
get_ext(EnflePlugins *eps, char *pluginname, Config *c)
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
