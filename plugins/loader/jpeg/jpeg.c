/*
 * jpeg.c -- jpeg loader plugin
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun May  1 16:46:17 2005.
 * $Id: jpeg.c,v 1.25 2005/05/01 15:37:55 sian Exp $
 *
 * This software is based in part on the work of the Independent JPEG Group
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

#include <jpeglib.h>
#include <jerror.h>
#include <setjmp.h>

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "enfle/loader-plugin.h"
#include "utils/libstring.h"

#if BITS_IN_JSAMPLE != 8
#  error BITS_IN_JSAMPLE must be 8
#endif

static const unsigned int types = (IMAGE_I420 | IMAGE_RGB24);

DECLARE_LOADER_PLUGIN_METHODS;

#define LOADER_JPEG_PLUGIN_DESCRIPTION "JPEG Loader plugin version 0.2.3"

static LoaderPlugin plugin = {
  .type = ENFLE_PLUGIN_LOADER,
  .name = "JPEG",
  .description = NULL,
  .author = "Hiroshi Takekawa",
  .image_private = NULL,

  .identify = identify,
  .load = load
};

ENFLE_PLUGIN_ENTRY(loader_jpeg)
{
  LoaderPlugin *lp;
  String *s;

  if ((lp = (LoaderPlugin *)calloc(1, sizeof(LoaderPlugin))) == NULL)
    return NULL;
  memcpy(lp, &plugin, sizeof(LoaderPlugin));
  s = string_create();
  string_set(s, LOADER_JPEG_PLUGIN_DESCRIPTION);
  /* 'compiled' means that the version string is compiled in. */
  string_catf(s, " compiled with libjpeg %02d", JPEG_LIB_VERSION);
  lp->description = strdup(string_get(s));
  string_destroy(s);

  return (void *)lp;
}

#undef LOADER_JPEG_PLUGIN_DESCRIPTION

ENFLE_PLUGIN_EXIT(loader_jpeg, p)
{
  LoaderPlugin *lp = (LoaderPlugin *)p;

  if (lp->description)
    free((unsigned char *)lp->description);
  free(lp);
}

/* jpeg data source functions */
typedef struct {
  struct jpeg_source_mgr pub;  /* public fields */
  Stream *st;                  /* source stream */
  JOCTET *buffer;              /* start of buffer */
  boolean start_of_file;       /* have we gotten any data yet? */
} my_source_mgr;
typedef my_source_mgr *my_src_ptr;

#define INPUT_BUF_SIZE 8192

METHODDEF(void)
init_source(j_decompress_ptr cinfo)
{
  my_src_ptr src = (my_src_ptr)cinfo->src;

  src->start_of_file = TRUE;
}

METHODDEF(boolean)
fill_input_buffer(j_decompress_ptr cinfo)
{
  my_src_ptr src = (my_src_ptr)cinfo->src;
  size_t nbytes;

  nbytes = stream_read(src->st, src->buffer, INPUT_BUF_SIZE);

  if (nbytes <= 0) {
    if (src->start_of_file)
      ERREXIT(cinfo, JERR_INPUT_EMPTY);
    WARNMS(cinfo, JWRN_JPEG_EOF);
    /* Insert a fake EOI marker */
    src->buffer[0] = (JOCTET)0xff;
    src->buffer[1] = (JOCTET)JPEG_EOI;
    nbytes = 2;
  }

  src->pub.next_input_byte = src->buffer;
  src->pub.bytes_in_buffer = nbytes;
  src->start_of_file = FALSE;

  return TRUE;
}

METHODDEF(void)
skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
  my_src_ptr src = (my_src_ptr)cinfo->src;

  if (num_bytes > 0) {
    while (num_bytes > (long)src->pub.bytes_in_buffer) {
      num_bytes -= (long)src->pub.bytes_in_buffer;
      (void)fill_input_buffer(cinfo);
    }
    src->pub.next_input_byte += (size_t)num_bytes;
    src->pub.bytes_in_buffer -= (size_t)num_bytes;
  }
}

METHODDEF(void)
term_source(j_decompress_ptr cinfo)
{
}

static void
attach_stream_src(j_decompress_ptr cinfo, Stream *st)
{
  my_src_ptr src;

  if (cinfo->src == NULL) {
    cinfo->src = (struct jpeg_source_mgr *)
      (*cinfo->mem->alloc_small)((j_common_ptr)cinfo, JPOOL_PERMANENT,
                                 sizeof(my_source_mgr));
    src = (my_src_ptr)cinfo->src;
    src->buffer = (JOCTET *)
      (*cinfo->mem->alloc_small)((j_common_ptr)cinfo, JPOOL_PERMANENT,
                                 INPUT_BUF_SIZE * sizeof(JOCTET));
  }

  src = (my_src_ptr)cinfo->src;
  src->pub.init_source = init_source;
  src->pub.fill_input_buffer = fill_input_buffer;
  src->pub.skip_input_data = skip_input_data;
  src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
  src->pub.term_source = term_source;
  src->st = st;
  src->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
  src->pub.next_input_byte = NULL; /* until buffer loaded */
}

/* error handler */

struct my_error_mgr {
  struct jpeg_error_mgr pub;    /* "public" fields */
  jmp_buf setjmp_buffer;        /* for return to caller */
  char *path;
};
typedef struct my_error_mgr *my_error_ptr;

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr)cinfo->err;

  if (cinfo->err->msg_code != JERR_NO_SOI) {
    err_message("jpeg loader: ");
    fflush(stdout);
    (*cinfo->err->output_message)(cinfo);
  }

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}

/* methods */

DEFINE_LOADER_PLUGIN_IDENTIFY(p, st, vw, c, priv)
{
  unsigned char buf[16];
  static unsigned char id[] = { 0xff, 0xd8 };

  if (stream_read(st, buf, 10) != 10)
    return LOAD_NOT;
  if (memcmp(buf, "hsi1", 4) == 0) {
    p->format_detail = (char *)"JPEG(hsi1)";
    return LOAD_OK;
  }
  if (memcmp(buf, id, 2) != 0)
    return LOAD_NOT;
  if (memcmp(buf + 6, "JFIF", 4) == 0) {
    p->format_detail = (char *)"JPEG(JFIF)";
    return LOAD_OK;
  }
  if (memcmp(buf + 6, "Exif", 4) == 0) {
    p->format_detail = (char *)"JPEG(Exif)";
    return LOAD_OK;
  }
  if (memcmp(buf + 6, "Adob", 4) == 0) {
    p->format_detail = (char *)"JPEG(Adobe)";
    return LOAD_OK;
  }
  if (memcmp(buf + 6, "\0" "\0" "\x1\x5", 4) == 0)
    return LOAD_OK;

  debug_message_fnc("Unknown ID (%02X%02X%02X%02X:%c%c%c%c).\n", buf[6], buf[7], buf[8], buf[9], buf[6], buf[7], buf[8], buf[9]);

  return LOAD_OK;
}

//#define ENABLE_YUV

DEFINE_LOADER_PLUGIN_LOAD(p, st, vw, c, priv)
{
  struct jpeg_decompress_struct *cinfo;
  struct my_error_mgr jerr;
  JSAMPROW buffer[1]; /* output row buffer */
  unsigned int i, j;
  unsigned char *d;
#ifdef ENABLE_YUV
  int direct_decode;
  ImageType requested_type;
#endif

  if ((cinfo = calloc(1, sizeof(struct jpeg_decompress_struct))) == NULL)
    return LOAD_ERROR;

  //debug_message("jpeg loader: load() called\n");

#ifdef IDENTIFY_BEFORE_LOAD
  {
    LoaderStatus status;

    if ((status = identify(p, st, vw, c, priv)) != LOAD_OK)
      return status;
    stream_rewind(st);
  }
#endif

  cinfo->err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  jerr.path = st->path;
  /* Establish the setjmp return context for my_error_exit to use. */
  if (setjmp(jerr.setjmp_buffer)) {
  error_destroy_free:
    jpeg_destroy_decompress(cinfo);
    free(cinfo);
    return LOAD_ERROR;
  }

  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress(cinfo);
  attach_stream_src(cinfo, st);
  (void)jpeg_read_header(cinfo, TRUE);

  cinfo->dct_method = JDCT_DEFAULT;
  {
    char *dm;
    if ((dm = config_get(c, "/enfle/plugins/loader/jpeg/dct_method"))) {
      if (strcasecmp(dm, "default") == 0) {
	cinfo->dct_method = JDCT_DEFAULT;
      } else if (strcasecmp(dm, "fast") == 0) {
	cinfo->dct_method = JDCT_IFAST;
      } else if (strcasecmp(dm, "slow") == 0) {
	cinfo->dct_method = JDCT_ISLOW;
      } else if (strcasecmp(dm, "float") == 0) {
	cinfo->dct_method = JDCT_FLOAT;
      } else if (strcasecmp(dm, "fastest")) {
	cinfo->dct_method = JDCT_FASTEST;
      }
    }
  }

#if 0
  debug_message("Using DCT method: ");
  switch (cinfo->dct_method) {
  case JDCT_IFAST: debug_message("IFAST"); break;
  case JDCT_ISLOW: debug_message("ISLOW"); break;
  case JDCT_FLOAT: debug_message("FLOAT"); break;
  default: debug_message("UNKNOWN"); break;
  }
  debug_message("\n");
#endif

#ifdef ENABLE_YUV
  if (vw) {
    requested_type = video_window_request_type(vw, types, &direct_decode);
    if (requested_type == _I420)
      cinfo->out_color_space = JCS_YCbCr;
  }
#endif

  jpeg_calc_output_dimensions(cinfo);

  image_width(p)  = cinfo->output_width;
  image_height(p) = cinfo->output_height;
  image_left(p) = 0;
  image_top(p) = 0;

  /* if (info->if_quantize) cinfo->quantize_colors = TRUE; */
  jpeg_calc_output_dimensions(cinfo);

  if (cinfo->output_components != 1 && cinfo->output_components != 3) {
    err_message("jpeg loader: Can't read %d components-jpeg file\n", cinfo->output_components);
    goto error_destroy_free;
  }

  (void)jpeg_start_decompress(cinfo);

  /* JSAMPLEs per row in output buffer */
  image_bpl(p) = image_width(p) * cinfo->output_components;

  //debug_message("Attempt to allocate memory %d bytes... ", p->bytes_per_line * image_height(p));

  if ((d = memory_alloc(image_image(p), image_bpl(p) * image_height(p))) == NULL) {
#ifdef DEBUG
    debug_message("failed.\n");
#else
    err_message("No enough memory (%d bytes)\n", image_bpl(p) * image_height(p));
#endif
    goto error_destroy_free;
  }

  //debug_message("Ok.\n");

  /* loading... */
  while (cinfo->output_scanline < image_height(p)) {
    buffer[0] = (JSAMPROW)&d[cinfo->output_scanline * image_bpl(p)];
    (void)jpeg_read_scanlines(cinfo, buffer, 1);
  }

  p->bits_per_pixel = cinfo->output_components << 3;
  switch (cinfo->out_color_space) {
  case JCS_GRAYSCALE:
    p->type = _GRAY;
    p->depth = 8;
    p->ncolors = 256;
    break;
  case JCS_RGB:
    /* Copy colormap. Because jpeg_finish_decompress will release colormap memory */
    if (cinfo->quantize_colors == TRUE) {
      if (cinfo->out_color_components == 1)
        for (j = 0; j < (unsigned int)cinfo->actual_number_of_colors; j++)
          p->colormap[j][0] = p->colormap[j][1] = p->colormap[j][2] =
	    cinfo->colormap[0][j];
      else
        for (j = 0; j < (unsigned int)cinfo->actual_number_of_colors; j++)
          for (i = 0; i < 3; i++)
            p->colormap[j][i] = cinfo->colormap[i][j];
      p->type = _INDEX;
      p->depth = 8;
      p->ncolors = cinfo->actual_number_of_colors;
    } else {
      p->type = _RGB24;
      p->depth = 24;
    }
    break;
  case JCS_YCbCr:
    {
      /* Convert to I420 */
      unsigned char *y, *u, *v;
      unsigned int w, h;

      w = ((image_width(p)  + 7) >> 3) << 3;
      h = image_height(p);

      if (!image_rendered_image(p))
	image_rendered_image(p) = memory_create();
      if (vw)
	memory_request_type(image_rendered_image(p), video_window_preferred_memory_type(vw));
      if ((y = memory_alloc(image_rendered_image(p), (w * h * 3) >> 1)) == NULL)
	goto error_destroy_free;
      u = y + w * h;
      v = u + ((w * h) >> 2);

      for (i = 0; i < image_height(p); i += 2) {
	for (j = 0; j < image_width(p); j += 2) {
	  y[ i       *  w +        j      ] = d[( i      * image_width(p) + j    ) * 3    ];
	  y[ i       *  w +        j + 1  ] = d[( i      * image_width(p) + j + 1) * 3    ];
	  y[(i + 1)  *  w +        j      ] = d[((i + 1) * image_width(p) + j    ) * 3    ];
	  y[(i + 1)  *  w +        j + 1  ] = d[((i + 1) * image_width(p) + j + 1) * 3    ];
	  u[(i >> 1) * (w >> 1) + (j >> 1)] = d[( i      * image_width(p) + j    ) * 3 + 1];
	  v[(i >> 1) * (w >> 1) + (j >> 1)] = d[( i      * image_width(p) + j    ) * 3 + 2];
	}
      }

      image_rendered_width(p) = w;
      image_rendered_height(p) = h;
      image_rendered_bpl(p) = (w * 3) >> 1;
      memory_destroy(image_image(p));
      image_image(p) = NULL;
      p->bits_per_pixel = 12;
      p->type = _I420;
    }
    break;
  default:
    break;
  }

  p->next = NULL;

  (void)jpeg_finish_decompress(cinfo);
  jpeg_destroy_decompress(cinfo);
  free(cinfo);

  return LOAD_OK;
}
