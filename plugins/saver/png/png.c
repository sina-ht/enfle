/*
 * png.c -- PNG Saver plugin
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Sep 18 13:39:46 2001.
 * $Id: png.c,v 1.8 2001/09/18 05:22:23 sian Exp $
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
  description: "PNG Saver plugin version 0.1.2",
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

DEFINE_SAVER_PLUGIN_SAVE(p, fp, c, params)
{
  png_structp png_ptr;
  png_infop info_ptr;
  png_text comment[2];
  png_uint_32 k;
  png_bytep *row_pointers;
  char *tmp;
  int compression_level;
  int filter_flag_set;
  int filter_flag;
  int interlace_flag;
  int result, b;

  debug_message("png: save (%s) (%d, %d) called.\n", image_type_to_string(p->type), p->width, p->height);

  if ((png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)) == NULL) {
    fclose(fp);
    return 0;
  }

  /* Allocate and initialize the image information data. */
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

  compression_level = config_get_int(c, "/enfle/plugins/saver/png/compression_level", &result);
  if (!result)
    compression_level = 9;
  else if (compression_level < -1 || compression_level > 9) {
    show_message("Invalid compression level %d: defaults to 9.\n", compression_level);
    compression_level = 9;
  }

  filter_flag = 0;
  if ((tmp = config_get_str(c, "/enfle/plugins/saver/png/filter")) == NULL)
    filter_flag_set = 0;
  else {
    filter_flag_set = 1;
    if (strcasecmp(tmp, "all") == 0)
      filter_flag = PNG_ALL_FILTERS;
    else if (strcasecmp(tmp, "default") == 0)
      filter_flag_set = 0;
    else if (strcasecmp(tmp, "paeth") == 0)
      filter_flag = PNG_FILTER_PAETH;
    else if (strcasecmp(tmp, "avg") == 0)
      filter_flag = PNG_FILTER_AVG;
    else if (strcasecmp(tmp, "sub") == 0)
      filter_flag = PNG_FILTER_SUB;
    else if (strcasecmp(tmp, "up") == 0)
      filter_flag = PNG_FILTER_UP;
    else if (strcasecmp(tmp, "none") == 0)
      filter_flag = PNG_NO_FILTERS;
    else {
      show_message("Invalid filter: %s\n", tmp);
      filter_flag_set = 0;
    }
  }

  b = config_get_boolean(c, "/enfle/plugins/saver/png/filter", &result);
  if (result > 0)
    interlace_flag = b ? PNG_INTERLACE_ADAM7 : PNG_INTERLACE_NONE;
  else {
    if (result < 0)
      warning("Invalid string in png/filter.\n");
    interlace_flag = PNG_INTERLACE_NONE;
  }

  /* set up the output control. */
  png_init_io(png_ptr, fp);

  /*
   * bit_depth: 1, 2, 4, 8, 16.
   * but valid values also depend on the color_type selected.
   * color_type: PNG_COLOR_TYPE_GRAY, PNG_COLOR_TYPE_GRAY_ALPHA,
   * PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB, PNG_COLOR_TYPE_RGB_ALPHA.
   * interlace: PNG_INTERLACE_NONE, PNG_INTERLACE_ADAM7.
   */

  debug_message("png: " __FUNCTION__ ": %dx%d, type %s\n", p->width, p->height, image_type_to_string(p->type));

  switch (p->type) {
  case _GRAY:
    png_set_IHDR(png_ptr, info_ptr, p->width, p->height, 8,
		 PNG_COLOR_TYPE_GRAY, interlace_flag,
		 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    break;
  case _INDEX:
    png_set_IHDR(png_ptr, info_ptr, p->width, p->height, 8,
		 PNG_COLOR_TYPE_PALETTE, interlace_flag,
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
		 PNG_COLOR_TYPE_RGB, interlace_flag,
		 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    break;
  default:
    show_message("png: " __FUNCTION__ ": Unsupported type %d.\n", p->type);
    fclose(fp);
    png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    return 0;
  }

  /* Compression parameter setup */
  debug_message("Compression level %d, filter %d(is_set %d), interlace %d\n", compression_level, filter_flag, filter_flag_set, interlace_flag);

#if (PNG_LIBPNG_VER > 99)
  png_set_compression_buffer_size(png_ptr, 32768L);
#endif
  png_set_compression_level(png_ptr, compression_level);
  if (filter_flag_set)
    png_set_filter(png_ptr, 0, filter_flag);

  /* Write comments into the image */
  comment[0].key = (char *)"Software";
  comment[0].text = (char *)"Enfle " VERSION " / " COPYRIGHT_MESSAGE;
  comment[0].compression = PNG_TEXT_COMPRESSION_NONE;
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
  case _GRAY:
  case _INDEX:
    for (k = 0; k < p->height; k++)
      row_pointers[k] = memory_ptr(p->image) + k * p->width;
    break;
  case _RGB24:
    for (k = 0; k < p->height; k++)
      row_pointers[k] = memory_ptr(p->image) + 3 * k * p->width;
    break;
  default:
    fprintf(stderr, "png_save_image: FATAL: internal error: type %s cannot be processed\n", image_type_to_string(p->type));
    png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    fclose(fp);
    exit(1);
  }

  /* Write the image */
  png_write_image(png_ptr, row_pointers);
  png_write_end(png_ptr, info_ptr);
  png_destroy_write_struct(&png_ptr, (png_infopp)NULL);

  return 1;
}
