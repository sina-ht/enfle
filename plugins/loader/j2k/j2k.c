/*
 * j2k.c -- j2k loader plugin
 * (C)Copyright 2004-2006 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Wed Apr 27 21:24:59 2016.
 * $Id: j2k.c,v 1.5 2006/04/30 15:52:48 sian Exp $
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

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "enfle/loader-plugin.h"
#define UTILS_NEED_GET_BIG_UINT32
#include "enfle/utils.h"
#include "utils/libstring.h"
#include "j2k/j2k.h"

DECLARE_LOADER_PLUGIN_METHODS;

#define LOADER_J2K_PLUGIN_DESCRIPTION "J2K Loader plugin version 0.1.1"

static LoaderPlugin plugin = {
  .type = ENFLE_PLUGIN_LOADER,
  .name = "J2K",
  .description = NULL,
  .author = "Hiroshi Takekawa",
  .image_private = NULL,

  .identify = identify,
  .load = load
};

ENFLE_PLUGIN_ENTRY(loader_j2k)
{
  LoaderPlugin *lp;
  String *s;

  if ((lp = (LoaderPlugin *)calloc(1, sizeof(LoaderPlugin))) == NULL)
    return NULL;
  memcpy(lp, &plugin, sizeof(LoaderPlugin));
  s = string_create();
  string_set(s, LOADER_J2K_PLUGIN_DESCRIPTION);
  string_catf(s, " with integrated libj2k " LIBJ2K_VERSION);
  lp->description = strdup(string_get(s));
  string_destroy(s);

  return (void *)lp;
}

#undef LOADER_J2K_PLUGIN_DESCRIPTION

ENFLE_PLUGIN_EXIT(loader_j2k, p)
{
  LoaderPlugin *lp = (LoaderPlugin *)p;

  if (lp->description)
    free((unsigned char *)lp->description);
  free(lp);
}

/* methods */

static inline int
ceildiv(int a, int b)
{
  return (a + b - 1) / b;
}

DEFINE_LOADER_PLUGIN_IDENTIFY(p, st, vw, c, priv)
{
  unsigned char buf[16];
  static unsigned char id[] = { 0xff, 0x4f, 0xff, 0x51 }; /* JPC/J2K: SOC, SIZ */
  static unsigned char id2[] = { 0x00, 0x00, 0x00, 0x0c, 0x6a, 0x50, 0x20, 0x20, 0x0d, 0x0a, 0x87, 0x0a }; /* JP2: Signature */

  if (stream_read(st, buf, 4) != 4)
    return LOAD_NOT;
  if (memcmp(buf, id, 4) == 0)
    return LOAD_OK; /* JPC/J2K */
  if (stream_read(st, buf + 4, 8) != 8)
    return LOAD_NOT;
  if (memcmp(buf, id2, 12) != 0)
    return LOAD_NOT;
  return LOAD_OK; /* JP2 */
}

DEFINE_LOADER_PLUGIN_LOAD(p, st, vw, c, priv)
{
  j2k_image_t *ji;
  j2k_cp_t *cp;
  unsigned char *d, *buf = NULL, *ptr;
  int i, size;

  //debug_message("J2K: load() called\n");

#ifdef IDENTIFY_BEFORE_LOAD
  {
    LoaderStatus status;

    if ((status = identify(p, st, vw, c, priv)) != LOAD_OK)
      return status;
    stream_rewind(st);
  }
#endif

  /* Read whole stream into buffer... */
  size = 0;
  {
    unsigned char *tmp;
    int len;
    int bufsize = 65536;

    
    for (;;) {
      if ((tmp = realloc(buf, bufsize)) == NULL) {
	free(buf);
	return LOAD_ERROR;
      }
      buf = tmp;
      len = stream_read(st, buf + size, bufsize - size);
      size += len;
      if (len < bufsize - size)
	break;
      bufsize += 65536;
    }
  }
  ptr = buf;

  /* loading... */
  if (ptr[0] == 0) {
    debug_message_fnc("JP2, so skip to SOC marker\n");
    int len;
    int offset = 0;
    static unsigned char jp2c[] = { 'j', 'p', '2', 'c' };
    for (;;) {
      if (offset > size) {
	free(buf);
	return LOAD_ERROR;
      }
      len = utils_get_big_uint32(ptr + offset);
      if (memcmp(ptr + offset + 4, jp2c, 4) == 0)
	break;
      offset += len;
    }
    ptr += offset + 8;
    if (ptr[0] != 0xff || ptr[1] != 0x4f) {
      err_message_fnc("J2K: hmm, where is SOC...?\n");
      return LOAD_ERROR;
    }
  }
  debug_message_fnc("call j2k_decode()\n");
  if (!j2k_decode(ptr, size, &ji, &cp)) {
    err_message_fnc("j2k_decode() failed.\n");
    goto error_clear;
  }
  free(buf);

  /* convert to enfle format */

  p->bits_per_pixel = 24;
  p->type = _RGB24;
  p->depth = 24;
  /* dimension */
  image_width(p)  = ceildiv(ji->x1 - ji->x0, ji->comps[0].dx);
  image_height(p) = ceildiv(ji->y1 - ji->y0, ji->comps[0].dy);
  image_left(p) = 0;
  image_top(p) = 0;
  image_bpl(p) = image_width(p) * 3;
  debug_message("J2K: (%d,%d)\n", image_width(p), image_height(p));
  if (ji->numcomps == 3) {
    debug_message("J2K: %d ncomponents, dx %d %d %d, dy %d %d %d, prec %d %d %d\n",
		  ji->numcomps,
		  ji->comps[0].dx, ji->comps[1].dx, ji->comps[2].dx,
		  ji->comps[0].dy, ji->comps[1].dy, ji->comps[2].dy,
		  ji->comps[0].prec, ji->comps[1].prec, ji->comps[2].prec);
  } else {
    debug_message("J2K: %d ncomponents, dx %d, dy %d, prec %d\n",
		  ji->numcomps,
		  ji->comps[0].dx,
		  ji->comps[0].dy,
		  ji->comps[0].prec);
  }

  /* memory allocation */
  if ((d = memory_alloc(image_image(p), image_bpl(p) * image_height(p))) == NULL) {
    err_message("No enough memory (%d bytes)\n", image_bpl(p) * image_height(p));
    goto error_destroy_free;
  }

  if (ji->numcomps == 3) {
    if (ji->comps[0].prec == 8) {
      for (i = 0; i < image_width(p) * image_height(p); i++) {
	*d++ = ji->comps[0].data[i];
	*d++ = ji->comps[1].data[i];
	*d++ = ji->comps[2].data[i];
      }
    } else if (ji->comps[0].prec > 8) {
      for (i = 0; i < image_width(p) * image_height(p); i++) {
	*d++ = ji->comps[0].data[i] >> (ji->comps[0].prec - 8);
	*d++ = ji->comps[1].data[i] >> (ji->comps[1].prec - 8);
	*d++ = ji->comps[2].data[i] >> (ji->comps[2].prec - 8);
      }
    } else if (ji->comps[0].prec < 8) {
      for (i = 0; i < image_width(p) * image_height(p); i++) {
	*d++ = ji->comps[0].data[i] << (8 - ji->comps[0].prec);
	*d++ = ji->comps[1].data[i] << (8 - ji->comps[1].prec);
	*d++ = ji->comps[2].data[i] << (8 - ji->comps[2].prec);
      }
    }
  } else {
    if (ji->comps[0].prec == 8) {
      for (i = 0; i < image_width(p) * image_height(p); i++) {
	*d++ = ji->comps[0].data[i];
	*d++ = ji->comps[0].data[i];
	*d++ = ji->comps[0].data[i];
      }
    } else if (ji->comps[0].prec > 8) {
      for (i = 0; i < image_width(p) * image_height(p); i++) {
	*d++ = ji->comps[0].data[i] >> (ji->comps[0].prec - 8);
	*d++ = ji->comps[0].data[i] >> (ji->comps[0].prec - 8);
	*d++ = ji->comps[0].data[i] >> (ji->comps[0].prec - 8);
      }
    } else if (ji->comps[0].prec < 8) {
      for (i = 0; i < image_width(p) * image_height(p); i++) {
	*d++ = ji->comps[0].data[i] << (8 - ji->comps[0].prec);
	*d++ = ji->comps[0].data[i] << (8 - ji->comps[0].prec);
	*d++ = ji->comps[0].data[i] << (8 - ji->comps[0].prec);
      }
    }
  }

  j2k_release(ji, cp);

  return LOAD_OK;

 error_destroy_free:
  j2k_release(ji, cp);
 error_clear:

  return LOAD_ERROR;
}
