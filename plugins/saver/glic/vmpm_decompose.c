/*
 * vmpm_decompose.c -- decompose plugin manager
 * (C)Copyright 2001 by Hiroshi Takekawa
 * Last Modified: Fri Aug  3 18:14:23 2001.
 * $Id: vmpm_decompose.c,v 1.3 2001/08/04 12:53:58 sian Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "vmpm.h"
#include "vmpm_error.h"

static int
load(VMPM *vmpm, char *path)
{
  VMPMDecomposer_init init;
  void *handle;
  char *error;

  if ((handle = dlopen(path, RTLD_LAZY)) == NULL) {
    fprintf(stderr, __FUNCTION__ ": dlopen: %s\n", dlerror());
    return 0;
  }

  if ((init = dlsym(handle, "decomposer_init")) == NULL && (error = dlerror())) {
    //fprintf(stderr, __FUNCTION__ ": dlsym: %s: %s\n", path, error);
    return 0;
  }

  if (vmpm->decomposer_top) {
    VMPMDecomposer *decomposer = init(vmpm);

    if (decomposer) {
      vmpm->decomposer_head->next = decomposer;
      vmpm->decomposer_head = vmpm->decomposer_head->next;
    }
  } else {
    VMPMDecomposer *decomposer = init(vmpm);

    if (decomposer)
      vmpm->decomposer_top = vmpm->decomposer_head = decomposer;
  }
  vmpm->decomposer_head->next = NULL;

  return 1;
}

int
decomposer_scan_and_load(VMPM *vmpm, char *path)
{
  char *filename, *extname;
  DIR *dir;
  struct dirent *ent;
  struct stat statbuf;
  int count = 0;

  if (!path) {
    fprintf(stderr, __FUNCTION__ ": path is NULL.\n");
    return 0;
  }
  
  if ((dir = opendir(path)) == NULL) {
    char buf[256];

    sprintf(buf, __FUNCTION__ ": opendir: %s", path);
    perror(buf);
    return 0;
  }

  while ((ent = readdir(dir))) {
    if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
      continue;
    if ((filename = malloc(strlen(path) + strlen(ent->d_name) + 2)) == NULL) {
      closedir(dir);
      memory_error(NULL, MEMORY_ERROR);
    }
    sprintf(filename, "%s/%s", path, ent->d_name);
    if (!stat(filename, &statbuf)) {
      if (S_ISDIR(statbuf.st_mode)) {
        /* count += decomposer_scan_and_load(vmpm, filename); */
	continue;
      } else if (S_ISREG(statbuf.st_mode) &&
		 (extname = strchr(ent->d_name, '.')) &&
		 (!strncmp(extname, ".so", 3))) {
        if (load(vmpm, filename))
	  count++;
      }
    }
    free(filename);
  }
  closedir(dir);

  return count;
}

VMPMDecomposer *
decomposer_search(VMPM *vmpm, char *name)
{
  VMPMDecomposer *p;

  if (!name)
    return NULL;

  for (p = vmpm->decomposer_top; p; p = p->next)
    if (strcmp(p->name, name) == 0)
      return p;

  return NULL;
}

void
decomposer_list(VMPM *vmpm, FILE *fp)
{
  VMPMDecomposer *p;

  for (p = vmpm->decomposer_top; p; p = p->next)
    fprintf(fp, "%s ", p->name);
}

void
decomposer_list_detail(VMPM *vmpm, FILE *fp)
{
  VMPMDecomposer *p;

  fprintf(fp, "Method description:\n");
  for (p = vmpm->decomposer_top; p; p = p->next)
    fprintf(fp, " %s: %s\n", p->name, p->description);
}
