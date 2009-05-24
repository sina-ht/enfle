/*
 * jasper.c -- jasper loader plugin
 * (C)Copyright 2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun May 24 23:48:23 2009.
 * $Id: jasper.c,v 1.5 2006/03/12 08:24:16 sian Exp $
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

#include <stdio.h>
#include <stdlib.h>

#include <jasper/jasper.h>
#undef PACKAGE
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef VERSION

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "enfle/loader-plugin.h"
#include "utils/libstring.h"

static const unsigned int types = IMAGE_RGB24;

DECLARE_LOADER_PLUGIN_METHODS;

#define LOADER_JASPER_PLUGIN_DESCRIPTION "JasPer Loader plugin version 0.1"

static LoaderPlugin plugin = {
  .type = ENFLE_PLUGIN_LOADER,
  .name = "JasPer",
  .description = NULL,
  .author = "Hiroshi Takekawa",
  .image_private = NULL,

  .identify = identify,
  .load = load
};

ENFLE_PLUGIN_ENTRY(loader_jasper)
{
  LoaderPlugin *lp;
  String *s;

  if ((lp = (LoaderPlugin *)calloc(1, sizeof(LoaderPlugin))) == NULL)
    return NULL;
  memcpy(lp, &plugin, sizeof(LoaderPlugin));
  s = string_create();
  string_set(s, LOADER_JASPER_PLUGIN_DESCRIPTION);
  string_catf(s, " with libjasper %s", jas_getversion());
  lp->description = strdup(string_get(s));
  string_destroy(s);

  if (jas_init()) {
    err_message_fnc("Initializing libjasper failed.\n");
    free(lp);
    return NULL;
  }

  return (void *)lp;
}

#undef LOADER_JASPER_PLUGIN_DESCRIPTION

ENFLE_PLUGIN_EXIT(loader_jasper, p)
{
  LoaderPlugin *lp = (LoaderPlugin *)p;

  if (lp->description)
    free((unsigned char *)lp->description);
  free(lp);
  jas_image_clearfmts();
}

/* methods */

DEFINE_LOADER_PLUGIN_IDENTIFY(p, st, vw, c, priv)
{
  unsigned char buf[16];
  static unsigned char id[] = { 0x00, 0x00, 0x00, 0x0c, 0x6a, 0x50, 0x20, 0x20, 0x0d, 0x0a, 0x87, 0x0a };

  if (stream_read(st, buf, 12) != 12)
    return LOAD_NOT;
  if (memcmp(buf, id, 12) != 0)
    return LOAD_NOT;

  return LOAD_OK;
}

DEFINE_LOADER_PLUGIN_LOAD(p, st, vw, c, priv)
{
  jas_image_t *ji;
  jas_stream_t *js;
  unsigned char *d;
  char *buf = NULL;
  int i, j, k, cmp[3];
  int tlx, tly;
  int vs, hs;

  //debug_message("JasPer: load() called\n");

#ifdef IDENTIFY_BEFORE_LOAD
  {
    LoaderStatus status;

    if ((status = identify(p, st, vw, c, priv)) != LOAD_OK)
      return status;
    stream_rewind(st);
  }
#endif

  /* Read whole stream into buffer... */
  {
    char *tmp;
    int size = 0, len;
    int bufsize = 65536;

    for (;;) {
      if ((tmp = realloc(buf, bufsize)) == NULL) {
	free(buf);
	return LOAD_ERROR;
      }
      buf = tmp;
      len = stream_read(st, (unsigned char *)(buf + size), bufsize - size);
      size += len;
      if (len < bufsize - size)
	break;
      bufsize += 65536;
    }
    if ((js = jas_stream_memopen(buf, size)) == NULL) {
      free(buf);
      return LOAD_ERROR;
    }
  }

  /* loading... */
  if ((ji = jas_image_decode(js, -1, 0)) == NULL) {
    err_message_fnc("jas_image_decode() failed.\n");
    goto error_clear;
  }

  /* colorspace conversion */
  {
    jas_cmprof_t *jc;
    jas_image_t *new_ji;
    if ((jc = jas_cmprof_createfromclrspc(JAS_CLRSPC_SRGB)) == NULL)
      goto error_destroy_free;
    if ((new_ji = jas_image_chclrspc(ji, jc, JAS_CMXFORM_INTENT_PER)) == NULL)
      goto error_destroy_free;
    jas_image_destroy(ji);
    ji = new_ji;
  }

  jas_stream_close(js);
  free(buf);
  debug_message("JasPer: jas_image_decode() OK: (%ld,%ld)\n", jas_image_cmptwidth(ji, 0), jas_image_cmptheight(ji, 0));

  /* convert to enfle format */

  p->bits_per_pixel = 24;
  p->type = _RGB24;
  p->depth = 24;
  cmp[0] = jas_image_getcmptbytype(ji, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_R));
  cmp[1] = jas_image_getcmptbytype(ji, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_G));
  cmp[2] = jas_image_getcmptbytype(ji, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_B));
  /* dimension */
  image_width(p)  = jas_image_cmptwidth(ji, cmp[0]);
  image_height(p) = jas_image_cmptheight(ji, cmp[0]);
  image_left(p) = 0;
  image_top(p) = 0;
  image_bpl(p) = image_width(p) * 3;
  tlx = jas_image_cmpttlx(ji, cmp[0]);
  tly = jas_image_cmpttly(ji, cmp[0]);
  vs = jas_image_cmptvstep(ji, cmp[0]);
  hs = jas_image_cmpthstep(ji, cmp[0]);
  debug_message("JasPer: tlx %d tly %d vs %d hs %d ncomponents %d\n", tlx, tly, vs, hs, jas_image_numcmpts(ji));
  /* memory allocation */
  if ((d = memory_alloc(image_image(p), image_bpl(p) * image_height(p))) == NULL) {
    err_message("No enough memory (%d bytes)\n", image_bpl(p) * image_height(p));
    goto error_destroy_free;
  }

  for (i = 0; i < image_height(p); i++) {
    for (j = 0; j < image_width(p); j++) {
      for (k = 0; k < 3; k++)
	*d++ = jas_image_readcmptsample(ji, cmp[k], j, i);
    }
  }

  jas_image_destroy(ji);

  return LOAD_OK;

 error_destroy_free:
  jas_image_destroy(ji);
 error_clear:

  return LOAD_ERROR;
}
