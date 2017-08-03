/*
 * saver.c -- Saver plugin interface
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Feb 18 01:40:20 2002.
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
