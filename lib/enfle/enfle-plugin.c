/*
 * enfle-plugin.c -- enfle plugin interface
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Sep 19 22:03:42 2000.
 * $Id: enfle-plugin.c,v 1.1 2000/09/30 17:36:36 sian Exp $
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

#include "plugin.h"
#include "enfle-plugin.h"

void *
enfle_plugin_load(char *filepath, PluginType *type_return)
{
  Plugin *p;
  EnflePlugin *ep;

  p = plugin_create();
  if (!plugin_load(p, filepath, "plugin_entry", "plugin_exit"))
    return NULL;
  ep = plugin_get(p);
  *type_return = ep->type;

  return (void *)p;
}
