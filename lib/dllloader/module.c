/*
 * module.c
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Feb 18 01:39:10 2002.
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

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "module.h"
#include "utils/misc.h"

typedef struct _module_list ModuleList;
struct _module_list {
  char *filename;
  HMODULE module;
  ModuleList *prev;
  ModuleList *next;
};

ModuleList *mod_list = NULL;

static ModuleList *
module_find(const char *filename)
{
  ModuleList *p = mod_list;

  if (!mod_list)
    return NULL;

  if (filename == NULL) {
    while (p && p->filename) {
      p = p->prev;
    }
  } else {
    char *trimmed = misc_trim_ext(p->filename, "dll");

    do {
      if (strcasecmp(p->filename, filename) == 0 ||
	  strcasecmp(trimmed, filename) == 0) {
	free(trimmed);
	return p;
      }
      p = p->prev;
    } while (p);
    free(trimmed);
  }

  return p;
}

int
module_register(const char *filename, HMODULE module)
{
  if (mod_list == NULL) {
    mod_list = malloc(sizeof(ModuleList));
    mod_list->next = mod_list->prev = NULL;
  } else {
    if (module_find(filename))
      return 1;

    mod_list->next = malloc(sizeof(ModuleList));
    mod_list->next->prev = mod_list;
    mod_list->next->next = NULL;
    mod_list = mod_list->next;
  }

  mod_list->filename = filename ? strdup(filename) : NULL;
  mod_list->module = module;

  return 1;
}

static int
module_delete(ModuleList *p)
{
  if (p) {
    if (p->prev)
      p->prev->next = p->next;
    if (p->next)
      p->next->prev = p->prev;
    if (mod_list == p)
      mod_list = mod_list->prev;
    if (p->module)
      free(p->module);
    free(p);
    return 1;
  }

  return 0;
}

int
module_deregister(const char *filename)
{
  ModuleList *p;

  p = module_find(filename);
  return module_delete(p);
}

HMODULE
module_lookup(const char *filename)
{
  ModuleList *p;

  if ((p = module_find(filename)) == NULL)
    return NULL;
  return p->module;
}
