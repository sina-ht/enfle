/*
 * png.c -- PNG Saver plugin
 * (C)Copyright 2000-2016 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Jul 23 12:29:52 2016.
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

#include <stdlib.h>

#include <sys/stat.h>
#include <errno.h>

#define REQUIRE_STRING_H
#define REQUIRE_UNISTD_H
#include "compat.h"
#include "common.h"

#ifdef HAVE_LIBPNG_PNG_H
# include <libpng/png.h>
#else
#error Install libpng
#endif

#include "utils/libstring.h"
#include "utils/libconfig.h"
#include "enfle/saver-plugin.h"

DECLARE_SAVER_PLUGIN_METHODS;

static SaverPlugin plugin = {
  .type = ENFLE_PLUGIN_SAVER,
  .name = "PNG",
  .description = "PNG Saver plugin version 0.2.1",
  .author = "Hiroshi Takekawa",

  .save = save,
  .get_ext = get_ext
};

ENFLE_PLUGIN_ENTRY(saver_png)
{
  SaverPlugin *sp;

  if ((sp = (SaverPlugin *)calloc(1, sizeof(SaverPlugin))) == NULL)
    return NULL;
  memcpy(sp, &plugin, sizeof(SaverPlugin));

  return (void *)sp;
}

ENFLE_PLUGIN_EXIT(saver_png, p)
{
  free(p);
}

/* methods */

DEFINE_SAVER_PLUGIN_GET_EXT(c __attribute__((unused)))
{
  return strdup("png");
}

DEFINE_SAVER_PLUGIN_SAVE(p, fp, c, params __attribute__((unused)))
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

  debug_message("png: save (%s) (%d, %d) called.\n", image_type_to_string(p->type), image_width(p), image_height(p));

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
  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_write_struct(&png_ptr, &info_ptr);
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

  b = config_get_boolean(c, "/enfle/plugins/saver/png/interlace", &result);
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

  debug_message("png: %s: %dx%d, type %s\n", __FUNCTION__, image_width(p), image_height(p), image_type_to_string(p->type));

  switch (p->type) {
  case _GRAY:
    png_set_IHDR(png_ptr, info_ptr, image_width(p), image_height(p), 8,
		 PNG_COLOR_TYPE_GRAY, interlace_flag,
		 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    break;
  case _INDEX:
    png_set_IHDR(png_ptr, info_ptr, image_width(p), image_height(p), 8,
		 PNG_COLOR_TYPE_PALETTE, interlace_flag,
		 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    /* set the palette.  REQUIRED for indexed-color images */
    png_set_PLTE(png_ptr, info_ptr, (png_colorp)p->colormap, p->ncolors);
    break;
#if 0
  case _RGB_WITH_BITMASK:
    if (!image_expand(p, _RGB24)) {
      fclose(fp);
      png_destroy_write_struct(&png_ptr, &info_ptr);
      return 0;
    }
#endif
  case _BGR24:
    png_set_bgr(png_ptr);
    /* FALL THROUGH */
  case _RGB24:
    png_set_IHDR(png_ptr, info_ptr, image_width(p), image_height(p), 8,
		 PNG_COLOR_TYPE_RGB, interlace_flag,
		 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    break;
  default:
    show_message("png: %s: Unsupported type %s.\n", __FUNCTION__, image_type_to_string(p->type));
    fclose(fp);
    png_destroy_write_struct(&png_ptr, &info_ptr);
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
  if ((row_pointers = png_malloc(png_ptr, sizeof(png_bytep) * image_height(p))) == NULL) {
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
    return 0;
  }

  /* Set up row pointers */
  for (k = 0; k < image_height(p); k++)
    row_pointers[k] = memory_ptr(image_image(p)) + k * image_bpl(p);

  /* Write the image */
  png_write_image(png_ptr, row_pointers);
  png_write_end(png_ptr, info_ptr);
  png_destroy_write_struct(&png_ptr, &info_ptr);

  return 1;
}
