/*
 * module.c
 */

#include <stdlib.h>

#include "module.h"
#include "misc.h"

#define REQUIRE_STRING_H
#include "compat.h"

#include "common.h"

typedef struct _module_list ModuleList;
struct _module_list {
  char *filename;
  HMODULE module;
  ModuleList *prev;
  ModuleList *next;
};

ModuleList *mod_list = NULL;

int
module_register(const char *filename, HMODULE module)
{
  if (mod_list == NULL) {
    mod_list = malloc(sizeof(ModuleList));
    mod_list->next = mod_list->prev = NULL;
  } else {
    if (mod_list->filename && strcmp(mod_list->filename, filename) == 0) {
      return 1;
    }

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
    free(p);
    return 1;
  }

  return 0;
}

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
