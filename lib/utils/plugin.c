/*
 * plugin.c -- plugin interface
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Feb 18 02:53:37 2002.
 * $Id: plugin.c,v 1.9 2002/02/17 19:32:57 sian Exp $
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

static int load(Plugin *, char *, const char *, const char *);
static int unload(Plugin *);
static void *get(Plugin *);
static void destroy(Plugin *);

static Plugin plugin_template = {
  load: load,
  unload: unload,
  get: get,
  destroy: destroy
};

Plugin *
plugin_create(void)
{
  Plugin *p;

  if ((p = calloc(1, sizeof(Plugin))) == NULL)
    return NULL;

  memcpy(p, &plugin_template, sizeof(Plugin));

  return p;
}

static int
load(Plugin *p, char *filepath, const char *entry_symbol, const char *exit_symbol)
{
  void *(*entry)(void) = NULL;

  if ((p->handle = dlopen(filepath, RTLD_LAZY | RTLD_GLOBAL)) == NULL) {
    p->err = (char *)dlerror();
    return 0;
  }

  if ((p->filepath = strdup(filepath)) == NULL)
    fprintf(stderr, "No enough memory to keep a plugin filepath: %s\n", filepath);

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

static int
unload(Plugin *p)
{
  if (p->substance) {
    if (p->substance_unload)
      p->substance_unload(p->substance);
    p->substance = NULL;
  }

  if (p->handle) {
#if 0
    /* This causes segfault... */
    dlclose(p->handle);
#endif
    p->handle = NULL;
  }

  return 1;
}

static void *
get(Plugin *p)
{
  return p->substance;
}

static void
destroy(Plugin *p)
{
  unload(p);

  if (p->filepath)
    free(p->filepath);
  free(p);
}
