/*
 * glic.c -- GLIC(Grammer-based Lossless Image Code) Saver plugin
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * Last Modified: Thu Sep  6 12:34:34 2001.
 * $Id: glic.c,v 1.17 2001/09/07 04:46:19 sian Exp $
 */

#include <stdlib.h>
#include <math.h>

#include <sys/stat.h>
#include <errno.h>

#define REQUIRE_STRING_H
#define REQUIRE_UNISTD_H
#include "compat.h"

#include "common.h"

#include "utils/libstring.h"
#include "utils/libconfig.h"
#include "utils/misc.h"
#include "enfle/saver-plugin.h"

#include "vmpm.h"
#include "vmpm_error.h"
#include "vmpm_decompose.h"

#include "predict.h"
#include "scan.h"
#include "threshold.h"
#include "expand.h"

#include "floorlog2.h"

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

/* methods */

DEFINE_SAVER_PLUGIN_GET_EXT(c)
{
  return strdup("glic");
}

DEFINE_SAVER_PLUGIN_SAVE(p, fp, c, params)
{
  VMPM vmpm;
  unsigned char *predicted;
  unsigned int image_size;
  char *path;
  char *vmpm_path;
  char *predict_method, *decompose_method, *scan_method;
  PredictType predict_id;
  int scan_id;
  int b, count, result;

  memset(&vmpm, 0, sizeof(vmpm));

  if (p->width != p->height) {
    show_message("width %d != %d height\n", p->width, p->height);
    return 0;
  }

  b = floorlog2(p->width);
  if ((1 << b) != p->width) {
    show_message("width %d is not a power of 2.\n", p->width);
    return 0;
  }

  b = floorlog2(p->height);
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

  vmpm.bitwise = config_get_int(c, "/enfle/plugins/saver/glic/vmpm/bitwise", &result);
  if (!result)
    vmpm.bitwise = 0;

  image_size = p->width * p->height;
  vmpm_path = (char *)config_get(c, "/enfle/plugins/saver/glic/vmpm_path");
  decompose_method = (char *)config_get(c, "/enfle/plugins/saver/glic/decompose_method");
  predict_method = (char *)config_get(c, "/enfle/plugins/saver/glic/predict_method");
  scan_method = (char *)config_get(c, "/enfle/plugins/saver/glic/scan_method");
  vmpm.outfilepath = strdup(path);
  vmpm.outfile = fp;
  vmpm.r = config_get_int(c, "/enfle/plugins/saver/glic/vmpm/r", &result);
  if (!result)
    vmpm.r = 2;
  vmpm.I = config_get_int(c, "/enfle/plugins/saver/glic/vmpm/I", &result);
  if (!result)
    vmpm.I = -2;
  if (vmpm.I <= 0) {
    unsigned int t, Imax;

    for (Imax = 0, t = image_size; t > 1; Imax++)
      t /= vmpm.r;

    switch (vmpm.I) {
    case 0:
      /* Use log log */
      vmpm.I = log(log(image_size) / log(vmpm.r)) / log(vmpm.r);
      debug_message("glic: I = %d (log log)\n", vmpm.I);
      break;
    case -1:
      /* Use Max */
      vmpm.I = Imax;
      debug_message("glic: I = %d (max)\n", vmpm.I);
      break;
    case -2:
      /* Use the value optimized by differentiation. */
      vmpm.I = log(image_size) / log(vmpm.r) + log(log(vmpm.r)) / log(vmpm.r) - 1;
      if (vmpm.I < 1) {
	debug_message("glic: I = %d underflow, clipped to 1.\n", vmpm.I);
	vmpm.I = 1;
      } else if (vmpm.I > Imax) {
	debug_message("glic: I = %d overflow, clipped to %d.\n", vmpm.I, Imax);
	vmpm.I = Imax;
      }
      debug_message("glic: I = %d (optimized)\n", vmpm.I);
      break;
    default:
      show_message("glic: Invalid I = %d, defaults to Imax = %d\n", vmpm.I, Imax);
      vmpm.I = Imax;
      break;
    }
  }
  vmpm.nlowbits = config_get_int(c, "/enfle/plugins/saver/glic/vmpm/nlowbits", &result);
  if (!result)
    vmpm.nlowbits = 4;
  if ((vmpm.is_stat = config_get_boolean(c, "/enfle/plugins/saver/glic/vmpm/is_stat", &result))) {
    char *statfn = misc_replace_ext(vmpm.outfilepath, (char *)"stat");

    if ((vmpm.statfile = fopen(statfn, "wb")) == NULL) {
      perror(__FUNCTION__ ": Cannot open %s for writting");
      return 0;
    }
    setlinebuf(vmpm.statfile);
  }

  debug_message("glic: (%d, %d) vmpm_path %s, decompose %s, predict %s, scan %s\n", vmpm.r, vmpm.I, vmpm_path, decompose_method, predict_method, scan_method);

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
  if ((vmpm.buffer = malloc(image_size)) == NULL)
    memory_error(NULL, MEMORY_ERROR);
  vmpm.buffersize = image_size;
  debug_message("glic: %d bytes allocated.\n", image_size);

  /* prediction */
  if ((predict_id = predict_get_id_by_name(predict_method)) == 0) {
    show_message("Invalid prediction method: %s\n", predict_method);
    return 0;
  }
  predicted = predict(memory_ptr(p->image), p->width, p->height, predict_id);

  /* scanning */
  if ((scan_id = scan_get_id_by_name(scan_method)) == 0) {
    show_message("Invalid scanning method: %s\n", scan_method);
    return 0;
  }
  scan(vmpm.buffer, predicted, p->width, p->height, scan_id);
  free(predicted);

  vmpm.bufferused = image_size;

  if (vmpm.nlowbits > 0)
    threshold(&vmpm);

  if (vmpm.bitwise)
    expand(&vmpm);

  result = vmpm.decomposer->decompose(&vmpm, 0, vmpm.I, vmpm.bufferused);
  debug_message("glic: decomposed.\n");

  /* output header */
  fprintf(vmpm.outfile, "GC%s\n\x1a%c%c", decompose_method, scan_id, predict_id);

  vmpm.decomposer->encode(&vmpm);
  debug_message("glic: encoded.\n");

  if (vmpm.buffer_low) {
    free(vmpm.buffer_low);
    vmpm.buffer_low = NULL;
  }

  /* check */
  rewind(vmpm.outfile);
  {
    char tmp[256];
    int c;

    fgets(tmp, 256, vmpm.outfile);
    stat_message(&vmpm, "D:Header: %s\n", tmp);
    if ((c = fgetc(vmpm.outfile)) != 0x1a) {
      stat_message(&vmpm, "Invalid character after the header: %c\n", c);
      debug_message("glic: check failed.\n");
    } else {
      scan_id = fgetc(vmpm.outfile);
      predict_id = fgetc(vmpm.outfile);
      stat_message(&vmpm, "D:scan %s/predictor %s\n", scan_get_name_by_id(scan_id), predict_get_name_by_id(predict_id));
      debug_message("glic: checked (incomplete).\n");
    }
  }

  vmpm.decomposer->final(&vmpm);

  if (vmpm.is_stat)
    fclose(vmpm.statfile);

  return 1;
}
