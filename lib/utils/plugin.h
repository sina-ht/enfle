/*
 * plugin.h -- plugin interface header
 * (C)Copyright 2000, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Mar  9 22:28:56 2004.
 * $Id: plugin.h,v 1.5 2004/03/09 13:59:24 sian Exp $
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
