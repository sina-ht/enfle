/*
 * png.c -- PNG Saver plugin
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Thu Apr 19 20:25:15 2001.
 * $Id: png.c,v 1.2 2001/04/20 08:09:56 sian Exp $
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

#include <sys/stat.h>
#include <errno.h>

#include <png.h>

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
  name: "PNG",
  description: "PNG Saver plugin version 0.1",
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

/* methods */

DEFINE_SAVER_PLUGIN_GET_EXT(c)
{
  return strdup("png");
}

DEFINE_SAVER_PLUGIN_SAVE(p, fp, params)
{
  //Config *c = (Config *)params;
  png_structp png_ptr;
  png_infop info_ptr;
  png_text comment[2];
  png_uint_32 k;
  png_bytep *row_pointers;

  debug_message("png: save (%s) (%d, %d) called.\n", image_type_to_string(p->type), p->width, p->height);

  if ((png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)) == NULL) {
    fclose(fp);
    return 0;
  }

  /* Allocate/initialize the image information data. */
  if ((info_ptr = png_create_info_struct(png_ptr)) == NULL) {
    png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    fclose(fp);
    return 0;
  }

  /* Set error handling. */
  if (setjmp(png_ptr->jmpbuf)) {
    /* If we get here, we had a problem writing the file */
    png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    fclose(fp);
    return 0;
  }

  /* set up the output control. */
  png_init_io(png_ptr, fp);

  /* Set the image information here.  Width and height are up to 2^31,
   * bit_depth is one of 1, 2, 4, 8, or 16, but valid values also depend on
   * the color_type selected. color_type is one of PNG_COLOR_TYPE_GRAY,
   * PNG_COLOR_TYPE_GRAY_ALPHA, PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB,
   * or PNG_COLOR_TYPE_RGB_ALPHA.  interlace is either PNG_INTERLACE_NONE or
   * PNG_INTERLACE_ADAM7, and the compression_type and filter_type MUST
   * currently be PNG_COMPRESSION_TYPE_BASE and PNG_FILTER_TYPE_BASE. REQUIRED
   */

  debug_message("png: " __FUNCTION__ ": %dx%d, type %d\n", p->width, p->height, p->type);

  switch (p->type) {
  case _INDEX:
    debug_message("depth 8 image\n");
    png_set_IHDR(png_ptr, info_ptr, p->width, p->height, 8,
		 PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
		 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    /* set the palette.  REQUIRED for indexed-color images */
    png_set_PLTE(png_ptr, info_ptr, (png_colorp)p->colormap, p->ncolors);
    break;
#if 0
  case _RGB_WITH_BITMASK:
    if (!image_expand(p, _RGB24)) {
      fclose(fp);
      png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
      return 0;
    }
#endif
  case _RGB24:
    png_set_IHDR(png_ptr, info_ptr, p->width, p->height, 8,
		 PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
		 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    break;
  default:
    show_message("png: " __FUNCTION__ ": Unsupported type %d.\n", p->type);
    fclose(fp);
    png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    return 0;
  }

#if (PNG_LIBPNG_VER > 99)
  png_set_compression_buffer_size(png_ptr, 32768L);
#endif
  png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

  /* Write comments into the image */
  comment[0].key = (char *)"Software";
  comment[0].text = (char *)"Created by Enfle " VERSION " / " COPYRIGHT_MESSAGE;
  comment[0].compression = PNG_TEXT_COMPRESSION_zTXt;
  png_set_text(png_ptr, info_ptr, comment, 1);

  /* other optional chunks like cHRM, bKGD, tRNS, tIME, oFFs, pHYs, */
  /* note that if sRGB is present the cHRM chunk must be ignored
   * on read and must be written in accordance with the sRGB profile */

  /* Write the file header information. */
  png_write_info(png_ptr, info_ptr);

  /* Allocate memory */
  if ((row_pointers = png_malloc(png_ptr, sizeof(png_bytep) * p->height)) == NULL) {
    png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    fclose(fp);
    return 0;
  }

  switch (p->type) {
  case _INDEX:
    for (k = 0; k < p->height; k++)
      row_pointers[k] = memory_ptr(p->image) + k * p->width;
    break;
  case _RGB24:
    for (k = 0; k < p->height; k++)
      row_pointers[k] = memory_ptr(p->image) + 3 * k * p->width;
    break;
  default:
    fprintf(stderr, "png_save_image: FATAL: internal error: type %d cannot be processed\n", p->type);
    png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    fclose(fp);
    exit(1);
  }

  /* Write the image */
  png_write_image(png_ptr, row_pointers);
  png_write_end(png_ptr, info_ptr);
  png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
  fclose(fp);

  return 1;
}
