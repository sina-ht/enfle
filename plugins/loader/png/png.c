/*
 * png.c -- png loader plugin
 * (C)Copyright 2000-2016 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Jul 23 12:55:40 2016.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

#ifdef HAVE_LIBPNG_PNG_H
# include <libpng/png.h>
#else
#error Install libpng
#endif

#include "enfle/loader-plugin.h"
#include "utils/libstring.h"

/* png_jmpbuf() macro is not defined prior to libpng-1.0.6. */
#ifndef png_jmpbuf
#  define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif

#define DISPLAY_GAMMA 2.20

//static const unsigned int types = (IMAGE_RGB24 | IMAGE_RGBA32 | IMAGE_GRAY | IMAGE_GRAY_ALPHA | IMAGE_INDEX);

DECLARE_LOADER_PLUGIN_METHODS;

#define LOADER_PNG_PLUGIN_DESCRIPTION "PNG Loader plugin version 0.4"

static LoaderPlugin plugin = {
  .type = ENFLE_PLUGIN_LOADER,
  .name = "PNG",
  .description = NULL,
  .author = "Hiroshi Takekawa",
  .image_private = NULL,

  .identify = identify,
  .load = load
};

ENFLE_PLUGIN_ENTRY(loader_png)
{
  LoaderPlugin *lp;
  String *s;

  if ((lp = (LoaderPlugin *)calloc(1, sizeof(LoaderPlugin))) == NULL)
    return NULL;
  memcpy(lp, &plugin, sizeof(LoaderPlugin));
  s = string_create();
  string_set(s, LOADER_PNG_PLUGIN_DESCRIPTION);
  /* 'compiled' means that the version string is compiled in. */
  string_cat(s, " compiled with libpng " PNG_LIBPNG_VER_STRING);
  lp->description = strdup(string_get(s));
  string_destroy(s);

  return (void *)lp;
}

#undef LOADER_PNG_PLUGIN_DESCRIPTION

ENFLE_PLUGIN_EXIT(loader_png, p)
{
  LoaderPlugin *lp = (LoaderPlugin *)p;

  if (lp->description)
    free((unsigned char *)lp->description);
  free(lp);
}

/* png data source functions */

static void
read_data(png_structp png_ptr, png_bytep data, png_uint_32 len)
{
  Stream *st = png_get_io_ptr(png_ptr);

  stream_read(st, data, len);
}

/* error handler */

static void
error_handler(png_structp png_ptr, png_const_charp error_msg)
{
  int *try_when_error = (int *)png_get_error_ptr(png_ptr);

  if (!strcmp(error_msg, "incorrect data check"))
    *try_when_error = 1;
  else
    err_message("png loader: %s\n", error_msg);

#if 0
  longjmp(png_jmpbuf(png_ptr), 1);
#endif
}

static void
warning_handler(png_structp png_ptr, png_const_charp warning_msg)
{
  Stream *st = png_get_io_ptr(png_ptr);

  warning("png loader: %s: %s\n", st->path, warning_msg);
}

/* methods */

#define PNG_BYTES_TO_CHECK 4

DEFINE_LOADER_PLUGIN_IDENTIFY(p, st, vw, c, priv)
{
  unsigned char buf[PNG_BYTES_TO_CHECK];

  /* Read in the signature bytes */
  if (stream_read(st, buf, PNG_BYTES_TO_CHECK) != PNG_BYTES_TO_CHECK)
    return LOAD_NOT;

  /* Compare the first PNG_BYTES_TO_CHECK bytes of the signature. */
  if (png_sig_cmp(buf, (png_size_t)0, PNG_BYTES_TO_CHECK) != 0)
    return LOAD_NOT;

  return LOAD_OK;
}

DEFINE_LOADER_PLUGIN_LOAD(p, st, vw, c, priv)
{
  png_structp png_ptr;
  png_infop info_ptr;
  png_uint_32 width, height;
  png_bytep *image_array;
  png_textp text_ptr;
  png_color_16 my_background, *image_background;
  int bit_depth, color_type, interlace_type;
  int text_len;
  int try_when_error;
  unsigned int i, num_text;
  int num_trans;
  png_color_16p trans_values;
  png_bytep trans;
  png_colorp palette = NULL;
  int num_palette = 0;

  //debug_message("png loader: load() called\n");

#ifdef IDENTIFY_BEFORE_LOAD
  {
    LoaderStatus status;

    if ((status = identify(p, st, vw, c, priv)) != LOAD_OK)
      return status;
  }
#endif
  try_when_error = 0;

  /* Create and initialize the png_struct */
  if ((png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (void *)&try_when_error,
					error_handler, warning_handler)) == NULL)
    return LOAD_ERROR;

  /* Allocate/initialize the memory for image information. */
  if ((info_ptr = png_create_info_struct(png_ptr)) == NULL) {
    png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
    return LOAD_ERROR;
  }

  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    return try_when_error ? LOAD_OK : LOAD_ERROR;
  }
  
  png_set_read_fn(png_ptr, (png_voidp)st, (png_rw_ptr)read_data);
#ifdef IDENTIFY_BEFORE_LOAD
  png_set_sig_bytes(png_ptr, PNG_BYTES_TO_CHECK);
#endif
  png_read_info(png_ptr, info_ptr);
  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
	       &interlace_type, NULL, NULL);

#ifdef DEBUG
  debug_message("png color type: ");
  switch (color_type) {
  case PNG_COLOR_TYPE_PALETTE:
    debug_message("PALETTE");
    break;
  case PNG_COLOR_TYPE_RGB:
    debug_message("RGB");
    break;
  case PNG_COLOR_TYPE_RGB_ALPHA:
    debug_message("RGB_ALPHA");
    break;
  case PNG_COLOR_TYPE_GRAY:
    debug_message("GRAY");
    break;
  case PNG_COLOR_TYPE_GRAY_ALPHA:
    debug_message("GRAY_ALPHA");
    break;
  default:
    debug_message("UNKNOWN");
    break;
  }
  debug_message("\n");
#endif

  image_width(p) = width;
  image_height(p) = height;
  image_top(p) = 0;
  image_left(p) = 0;

  /* read comment */
  if ((num_text = png_get_text(png_ptr, info_ptr, &text_ptr, 0)) > 0) {
    text_len = 0;
    for (i = 0; i < num_text; i++)
      text_len += strlen(text_ptr[i].key) + 2 + strlen(text_ptr[i].text) + 1;
    if ((p->comment = calloc(1, text_len + 1)) == NULL) {
      png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
      return LOAD_ERROR;
    }
    for (i = 0; i < num_text; i++) {
      strcat((char *)p->comment, text_ptr[i].key);
      strcat((char *)p->comment, ": ");
      strcat((char *)p->comment, text_ptr[i].text);
      strcat((char *)p->comment, "\n");
    }
  }

  if (bit_depth == 16)
    png_set_strip_16(png_ptr);
  else if (bit_depth < 8)
    png_set_packing(png_ptr);

  if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    png_set_expand(png_ptr);

#if 1
  /* XXX: Strip alpha off */
  if (color_type & PNG_COLOR_MASK_ALPHA)
    png_set_strip_alpha(png_ptr);
#endif

  png_read_update_info(png_ptr, info_ptr);
  color_type = png_get_color_type(png_ptr, info_ptr);

  /* Set the transparent color */
  if (png_get_tRNS(png_ptr, info_ptr, &trans, &num_trans, &trans_values)) {
    //p->transparent_disposal = info->transparent_disposal;
    switch (color_type) {
    case PNG_COLOR_TYPE_PALETTE:
      for (i = 0; i < (unsigned int)num_trans; i++) {
	if (trans[i] == 0) {
	  p->transparent_color.index = i;
	  break;
	}
      }
      break;
    case PNG_COLOR_TYPE_RGB:
    case PNG_COLOR_TYPE_RGB_ALPHA:
      p->transparent_color.red = trans_values->red >> 8;
      p->transparent_color.green = trans_values->green >> 8;
      p->transparent_color.blue = trans_values->blue >> 8;
      break;
    case PNG_COLOR_TYPE_GRAY:
    case PNG_COLOR_TYPE_GRAY_ALPHA:
      p->transparent_color.gray = trans_values->gray;
      break;
    default:
      break;
    }
  }

  /* Set the background color */
  if (png_get_bKGD(png_ptr, info_ptr, &image_background)) {
    p->background_color.red   = image_background->red;
    p->background_color.green = image_background->green;
    p->background_color.blue  = image_background->blue;
    p->background_color.index = image_background->index;
    p->background_color.gray  = image_background->gray;
    png_set_background(png_ptr, image_background, PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
  } else {
    if (color_type != PNG_COLOR_TYPE_RGB_ALPHA &&
	color_type != PNG_COLOR_TYPE_GRAY_ALPHA &&
	num_trans > 0) {
      p->background_color.red   = my_background.red   = p->transparent_color.red;
      p->background_color.green = my_background.green = p->transparent_color.green;
      p->background_color.blue  = my_background.blue  = p->transparent_color.blue;
      p->background_color.index = my_background.index = p->transparent_color.index;
      p->background_color.gray  = my_background.gray  = p->transparent_color.gray;
    } else {
      p->background_color.red = p->background_color.green = p->background_color.blue = p->background_color.index = p->background_color.gray =
	my_background.red = my_background.green = my_background.blue = my_background.index = my_background.gray = 0;
    }
    //png_set_background(png_ptr, &my_background, PNG_BACKGROUND_GAMMA_SCREEN, 0, DISPLAY_GAMMA);
  }

  /* prepare image data, store palette if exists */
  switch (color_type) {
  case PNG_COLOR_TYPE_RGB:
    p->type = _RGB24;
    p->depth = 24;
    p->bits_per_pixel = 24;
    image_bpl(p) = width * 3;
    break;
  case PNG_COLOR_TYPE_RGB_ALPHA:
    p->type = _RGBA32;
    p->depth = 24;
    p->bits_per_pixel = 32;
    image_bpl(p) = width * 4;
    break;
  case PNG_COLOR_TYPE_GRAY:
    p->type = _GRAY;
    p->ncolors = 256;
    p->depth = 8;
    p->bits_per_pixel = 8;
    image_bpl(p) = width;
    break;
  case PNG_COLOR_TYPE_GRAY_ALPHA:
    p->type = _GRAY_ALPHA;
    p->ncolors = 256;
    p->depth = 8;
    p->bits_per_pixel = 16;
    image_bpl(p) = width * 2;
    break;
  default:
    p->type = _INDEX;
    p->depth = 8;
    p->bits_per_pixel = 8;
    image_bpl(p) = width;

    if ((png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette) & PNG_INFO_PLTE) && num_palette > 0 && palette != NULL) {
      p->ncolors = num_palette;
      for (i = 0; i < p->ncolors; i++) {
	p->colormap[i][0] = palette[i].red;
	p->colormap[i][1] = palette[i].green;
	p->colormap[i][2] = palette[i].blue;
      }
    }
    break;
  }

  /* allocate memory for returned image */
  if (memory_alloc(image_image(p), image_bpl(p) * image_height(p)) == NULL) {
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    return LOAD_ERROR;
  }

  /* allocate memory for pointer array */
  if ((image_array = calloc(height, sizeof(png_bytep))) == NULL) {
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    return LOAD_ERROR;
  }

  for (i = 0; i < height; i++)
    image_array[i] = memory_ptr(image_image(p)) + png_get_rowbytes(png_ptr, info_ptr) * i;

  /* read image */
  png_read_image(png_ptr, image_array);

  /* read rest of file, and get additional chunks in info_ptr */
  png_read_end(png_ptr, info_ptr);

#if 0
  /* process alpha channel if exists */
  if (color_type == PNG_COLOR_TYPE_RGB_ALPHA) {
    if (!png_process_alpha_rgb(p)) {
      png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
      return LOAD_ERROR;
    } else
      p->transparent_disposal = info->transparent_disposal;
  } else if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
    if (!png_process_alpha_gray(p)) {
      png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
      return LOAD_ERROR;
    } else
      p->transparent_disposal = info->transparent_disposal;
  }
#endif

  /* clean up after the read, and free any memory allocated */
  png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

  /* free temporary memory */
  free(image_array);

  return LOAD_OK;
}
