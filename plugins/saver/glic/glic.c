/*
 * glic.c -- GLIC(Grammer-based Lossless Image Code) Saver plugin
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * Last Modified: Mon Apr 30 03:45:35 2001.
 * $Id: glic.c,v 1.4 2001/04/30 01:09:08 sian Exp $
 */

#include <stdlib.h>

#include <sys/stat.h>
#include <errno.h>

#define REQUIRE_STRING_H
#define REQUIRE_UNISTD_H
#include "compat.h"

#include "common.h"

#include "utils/libstring.h"
#include "utils/libconfig.h"
#include "enfle/saver-plugin.h"

#include "vmpm.h"
#include "vmpm_error.h"
#include "vmpm_decompose.h"

DECLARE_SAVER_PLUGIN_METHODS;

static SaverPlugin plugin = {
  type: ENFLE_PLUGIN_SAVER,
  name: "GLIC",
  description: "GLIC(Grammar-based Lossless Image Code) Saver plugin version 0.1",
  author: "Hiroshi Takekawa",

  save: save,
  get_ext: get_ext
};

void *
plugin_entry(void)
{
  SaverPlugin *sp;

  if ((sp = (SaverPlugin *)calloc(1, sizeof(SaverPlugin))) == NULL)
    return NULL;
  memcpy(sp, &plugin, sizeof(SaverPlugin));

  return (void *)sp;
}

void
plugin_exit(void *p)
{
  free(p);
}

/* for internal use */

static void
quad_scan(unsigned char *d, unsigned char *s, int width, int w, int h, int x, int y)
{
  if (w > 2) {
    int hw = w >> 1;
    int hh = h >> 1;

    quad_scan(d              , s, width, hw, hh, x     , y     );
    quad_scan(d + hw * hh    , s, width, hw, hh, x + hw, y     );
    quad_scan(d + hw * hh * 2, s, width, hw, hh, x     , y + hh);
    quad_scan(d + hw * hh * 3, s, width, hw, hh, x + hw, y + hh);
  }

  *d++ = s[ y      * width + x    ];
  *d++ = s[ y      * width + x + 1];
  *d++ = s[(y + 1) * width + x    ];
  *d++ = s[(y + 1) * width + x + 1];
}

/* methods */

DEFINE_SAVER_PLUGIN_GET_EXT(c)
{
  return strdup("glic");
}

static int
count_bits(unsigned int n)
{
  int i;

  if (n == 0)
    return -1;
  else
    i = 0;

  while ((n >>= 1))
    i++;
  return i;
}

DEFINE_SAVER_PLUGIN_SAVE(p, fp, params)
{
  Config *c = (Config *)params;
  VMPM vmpm;
  unsigned char *s;
  unsigned int image_size;
  char *path;
  char *vmpm_path;
  char *decompose_method, *scan_method;
  int count, result;
  int b;

  b = count_bits(p->width);
  if ((1 << b) != p->width) {
    show_message("width %d is not a power of 2.\n", p->width);
    return 0;
  }
  b = count_bits(p->height);
  if ((1 << b) != p->height) {
    show_message("height %d is not a power of 2.\n", p->height);
    return 0;
  }

  if ((path = config_get_str(c, "/enfle/plugins/ui/convert/source_path")) == NULL) {
    show_message("Caller did not set source_path.\n");
    return 0;
  }

  debug_message("glic: save %s (%s) (%d, %d) called.\n", path, image_type_to_string(p->type), p->width, p->height);

  if (p->type != _INDEX && p->type != _GRAY) {
    show_message("glic: _INDEX or _GRAY expected.\n");
    return 0;
  }

  image_size = p->width * p->height;

  vmpm_path = (char *)config_get(c, "/enfle/plugins/saver/glic/vmpm_path");
  decompose_method = (char *)config_get(c, "/enfle/plugins/saver/glic/decompose_method");
  scan_method = (char *)config_get(c, "/enfle/plugins/saver/glic/scan_method");
  memset(&vmpm, 0, sizeof(vmpm));
  vmpm.outfilepath = strdup(path);
  vmpm.outfile = fp;
  vmpm.r = config_get_int(c, "/enfle/plugins/saver/glic/vmpm/r", &result);
  if (!result)
    vmpm.r = 2;
  vmpm.I = config_get_int(c, "/enfle/plugins/saver/glic/vmpm/I", &result);
  if (!result)
    vmpm.I = 4;
  vmpm.nlowbits = config_get_int(c, "/enfle/plugins/saver/glic/vmpm/nlowbits", &result);
  if (!result)
    vmpm.nlowbits = 2;

  debug_message("glic: (%d, %d) vmpm_path %s, decompose method %s, scan method %s\n", vmpm.r, vmpm.I, vmpm_path, decompose_method, scan_method);

  if ((count = decomposer_scan_and_load(&vmpm, vmpm_path)) == 0) {
    show_message("glic: No decomposer.\n");
    return 0;
  }

  debug_message("glic: %d decomposer(s) found.\n", count);

  if ((vmpm.decomposer = decomposer_search(&vmpm, decompose_method)) == NULL) {
    show_message("glic: No such decomposer: %s\n", decompose_method);
    return 0;
  }

  vmpm.decomposer->init(&vmpm);
  s = memory_ptr(p->image);
  if ((vmpm.buffer = malloc(image_size)) == NULL)
    memory_error(NULL, MEMORY_ERROR);
  vmpm.buffersize = image_size;
  debug_message("glic: %d bytes allocated.\n", image_size);

  /* scanning */
  if (!strcmp(scan_method, "Quad")) {
    quad_scan(vmpm.buffer, s, p->width, p->width, p->height, 0, 0);
    debug_message("glic: scanned.\n");
  } else {
    memcpy(vmpm.buffer, s, image_size);
  }

  vmpm.bufferused = image_size;
  result = vmpm.decomposer->decompose(&vmpm, 0, vmpm.I, vmpm.bufferused);
  debug_message("glic: decomposed.\n");
  vmpm.decomposer->encode(&vmpm);
  debug_message("glic: encoded.\n");
  vmpm.decomposer->final(&vmpm);

  return 1;
}
