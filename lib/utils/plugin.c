/*
 * plugin.c -- plugin interface
 * (C)Copyright 2000, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Mar  9 22:28:24 2004.
 * $Id: plugin.c,v 1.14 2004/03/09 13:59:24 sian Exp $
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
#include <dlfcn.h>

#define REQUIRE_STRING_H
#include "compat.h"

#include "common.h"

#include "plugin.h"

Plugin *
plugin_create(void)
{
  Plugin *p;

  if ((p = calloc(1, sizeof(Plugin))) == NULL)
    return NULL;

  return p;
}

Plugin *
plugin_create_from_static(void *(*plugin_entry)(void), void (*plugin_exit)(void *))
{
  Plugin *p = plugin_create();

  if (!p)
    return NULL;

  p->handle = NULL;
  p->filepath = NULL;
  if (plugin_entry)
    p->substance = plugin_entry();
  p->substance_unload = plugin_exit;

  return p;
}

int
plugin_load(Plugin *p, char *filepath, const char *entry_symbol, const char *exit_symbol)
{
  void *(*entry)(void) = NULL;

  if ((p->handle = dlopen(filepath, RTLD_LAZY | RTLD_GLOBAL)) == NULL) {
    p->err = (char *)dlerror();
    return 0;
  }

  if (!p->filepath && (p->filepath = strdup(filepath)) == NULL)
    err_message("No enough memory to keep a plugin filepath: %s\n", filepath);

  if (entry_symbol) {
    entry = (void *(*)(void))dlsym(p->handle, entry_symbol);
    if ((p->err = (char *)dlerror())) {
      dlclose(p->handle);
      return 0;
    }
  }

  if (exit_symbol) {
    p->substance_unload = (void (*)(void *))dlsym(p->handle, exit_symbol);
    if ((p->err = (char *)dlerror())) {
      dlclose(p->handle);
      return 0;
    }
  }

  if (entry)
    p->substance = entry();

  return 1;
}

int
plugin_autoload(Plugin *p, char *filepath)
{
  if ((p->filepath = strdup(filepath)) == NULL) {
    err_message("No enough memory to keep a plugin filepath: %s\n", filepath);
    return 0;
  }

  return 1;
}

void *
plugin_get(Plugin *p)
{
  if (p->substance)
    return p->substance;
  if (p->filepath) {
    debug_message_fnc("autoloading %s\n", p->filepath);
    if (plugin_load(p, p->filepath, "plugin_entry", "plugin_exit"))
      return p->substance;
  }
  return NULL;
}

int
plugin_unload(Plugin *p)
{
  if (p->substance) {
    if (p->substance_unload)
      p->substance_unload(p->substance);
    p->substance = NULL;
  }

  if (p->handle) {
    /* XXX: This might cause segfault... older glibc's bug? */
    dlclose(p->handle);
    p->handle = NULL;
  }

  return 1;
}

void
plugin_destroy(Plugin *p)
{
  plugin_unload(p);

  if (p->filepath)
    free(p->filepath);
  free(p);
}
