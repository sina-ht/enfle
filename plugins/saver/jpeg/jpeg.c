/*
 * jpeg.c -- JPEG Saver plugin
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Jun 19 02:00:25 2001.
 * $Id: jpeg.c,v 1.2 2001/06/19 08:16:19 sian Exp $
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

#include <sys/stat.h>
#include <errno.h>

#include <jpeglib.h>
#include <jerror.h>
#include <setjmp.h>

#define JPEG_SAVE_DEFAULT_QUALITY 100

#if BITS_IN_JSAMPLE != 8
#  error BITS_IN_JSAMPLE must be 8
#endif

#define REQUIRE_STRING_H
#define REQUIRE_UNISTD_H
#include "compat.h"
#include "common.h"

#include "utils/libstring.h"
#include "utils/libconfig.h"
#include "enfle/saver-plugin.h"

DECLARE_SAVER_PLUGIN_METHODS;

static SaverPlugin plugin = {
  type: ENFLE_PLUGIN_SAVER,
  name: "JPEG",
  description: "JPEG Saver plugin version 0.1",
  author: "Hiroshi Takekawa",

  save: save,
  get_ext: get_ext
};

struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */
  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr *my_error_ptr;

/* my error handler */
METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr)cinfo->err;

  if (cinfo->err->msg_code != JERR_NO_SOI)
    (*cinfo->err->output_message)(cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}

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

/* methods */

DEFINE_SAVER_PLUGIN_GET_EXT(c)
{
  return strdup("jpg");
}

DEFINE_SAVER_PLUGIN_SAVE(p, fp, c, params)
{
  struct jpeg_compress_struct *cinfo = malloc(sizeof(struct jpeg_compress_struct));
  struct my_error_mgr jerr;
  JSAMPROW buffer[1]; /* output row buffer */
  int quality, result;

  debug_message("jpeg: save (%s) (%d, %d) called.\n", image_type_to_string(p->type), p->width, p->height);

  if (cinfo == NULL)
    return 0;

  switch (p->type) {
  case _BITMAP_LSBFirst:
  case _BITMAP_MSBFirst:
  case _GRAY_ALPHA:
  case _INDEX:
  case _RGB_WITH_BITMASK:
  case _BGR_WITH_BITMASK:
  case _BGR24:
  case _RGBA32:
  case _ABGR32:
  case _ARGB32:
  case _BGRA32:
    show_message("Saving of %s type image is not yet implemented.\n", image_type_to_string(p->type));
    return 0;
  case _GRAY:
  case _RGB24:
    break;
  default:
    fprintf(stderr, "Unknown image type: %d (maybe bug)\n", p->type);
    return 0;
  }

  cinfo->err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  /* Establish the setjmp return context for my_error_exit to use. */
  if (setjmp(jerr.setjmp_buffer)) {
    jpeg_destroy_compress(cinfo);
    free(cinfo);
    return 0;
  }

  quality = config_get_int(c, "/enfle/plugins/saver/jpeg/quality", &result);
  if (!result)
    quality = JPEG_SAVE_DEFAULT_QUALITY;
  else if (quality < 1 || quality > 100) {
    show_message("Invalid quality %d: defaults to %d.\n", quality, JPEG_SAVE_DEFAULT_QUALITY);
    quality = JPEG_SAVE_DEFAULT_QUALITY;
  }

  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_compress(cinfo);
  jpeg_stdio_dest(cinfo, fp);

  cinfo->image_width = p->width;
  cinfo->image_height = p->height;
  if (p->type == _GRAY) {
    cinfo->input_components = 1;
    cinfo->in_color_space = JCS_GRAYSCALE;
  } else {
    cinfo->input_components = 3;
    cinfo->in_color_space = JCS_RGB;
  }
  jpeg_set_defaults(cinfo);
  jpeg_set_quality(cinfo, quality, TRUE);

  jpeg_start_compress(cinfo, TRUE);

  while (cinfo->next_scanline < cinfo->image_height) {
    buffer[0] = (JSAMPROW)(memory_ptr(p->image) + p->bytes_per_line * cinfo->next_scanline);
    (void)jpeg_write_scanlines(cinfo, buffer, 1);
  }

  (void)jpeg_finish_compress(cinfo);
  jpeg_destroy_compress(cinfo);
  free(cinfo);

  debug_message("jpeg: saved.\n");

  return 1;
}
