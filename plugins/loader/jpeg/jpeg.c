/*
 * jpeg.c -- jpeg loader plugin
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Thu Jun 21 00:43:51 2001.
 * $Id: jpeg.c,v 1.9 2001/06/20 15:53:19 sian Exp $
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

#if BITS_IN_JSAMPLE != 8
#  error BITS_IN_JSAMPLE must be 8
#endif

static const unsigned int types = (IMAGE_I420 | IMAGE_RGB24);

DECLARE_LOADER_PLUGIN_METHODS;

static LoaderPlugin plugin = {
  type: ENFLE_PLUGIN_LOADER,
  name: "JPEG",
  description: "JPEG Loader plugin version 0.2.1",
  author: "Hiroshi Takekawa",

  identify: identify,
  load: load
};

void *
plugin_entry(void)
{
  LoaderPlugin *lp;

  if ((lp = (LoaderPlugin *)calloc(1, sizeof(LoaderPlugin))) == NULL)
    return NULL;
  memcpy(lp, &plugin, sizeof(LoaderPlugin));

  return (void *)lp;
}

void
plugin_exit(void *p)
{
  free(p);
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
};
typedef struct my_error_mgr *my_error_ptr;

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

/* methods */

DEFINE_LOADER_PLUGIN_IDENTIFY(p, st, vw, c, priv)
{
  unsigned char buf[16];
  static unsigned char id[] = { 0xff, 0xd8 };

  if (stream_read(st, buf, 10) != 10)
    return LOAD_NOT;
  if (memcmp(buf, "hsi1", 4) == 0)
    return LOAD_OK;
  if (memcmp(buf, id, 2) != 0)
    return LOAD_NOT;
  if (memcmp(buf + 6, "JFIF", 4) == 0)
    return LOAD_OK;
  if (memcmp(buf + 6, "Exif", 4) == 0)
    return LOAD_OK;

  show_message("Looks like jpeg, but its ID (%c%c%c%c) is not recognized. I'll try to load. If I can load successfully, please report it.\n", buf[6], buf[7], buf[8], buf[9]);

  return LOAD_OK;
}

//#define ENABLE_YUV

DEFINE_LOADER_PLUGIN_LOAD(p, st, vw, c, priv)
{
  struct jpeg_decompress_struct *cinfo;
  struct my_error_mgr jerr;
  JSAMPROW buffer[1]; /* output row buffer */
  int i, j;
  unsigned char *d;
#ifdef ENABLE_YUV
  int direct_decode;
  ImageType requested_type;
#endif

  if ((cinfo = calloc(1, sizeof(struct jpeg_decompress_struct))) == NULL)
    return LOAD_ERROR;

  debug_message("jpeg loader: load() called\n");

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

#ifdef ENABLE_YUV
  if (vw) {
    requested_type = video_window_request_type(vw, types, &direct_decode);
    if (requested_type == _I420)
      cinfo->out_color_space = JCS_YCbCr;
  }
#endif

  jpeg_calc_output_dimensions(cinfo);

  p->width  = cinfo->output_width;
  p->height = cinfo->output_height;
  p->left = 0;
  p->top = 0;

  /* if (info->if_quantize) cinfo->quantize_colors = TRUE; */
  jpeg_calc_output_dimensions(cinfo);

  if (cinfo->output_components != 1 && cinfo->output_components != 3) {
    fprintf(stderr, "Can't read %d components-jpeg file\n", cinfo->output_components);
    goto error_destroy_free;
  }

  (void)jpeg_start_decompress(cinfo);

  /* JSAMPLEs per row in output buffer */
  p->bytes_per_line = p->width * cinfo->output_components;

  debug_message("Attempt to allocate memory %d bytes... ", p->bytes_per_line * p->height);

  if ((d = memory_alloc(p->image, p->bytes_per_line * p->height)) == NULL) {
#ifdef DEBUG
    debug_message("failed.\n");
#else
    show_message("No enough memory (%d bytes)\n", p->bytes_per_line * p->height);
#endif
    goto error_destroy_free;
  }

  debug_message("Ok.\n");

  /* loading... */
  while (cinfo->output_scanline < p->height) {
    buffer[0] = (JSAMPROW)&d[cinfo->output_scanline * p->bytes_per_line];
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
        for (j = 0; j < cinfo->actual_number_of_colors; j++)
          p->colormap[j][0] = p->colormap[j][1] = p->colormap[j][2] =
	    cinfo->colormap[0][j];
      else
        for (j = 0; j < cinfo->actual_number_of_colors; j++)
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
      char *y, *u, *v;
      unsigned int w, h;

      w = ((p->width  + 7) >> 3) << 3;
      h = ((p->height + 7) >> 3) << 3;

      if ((y = calloc(1, (w * h * 3) >> 1)) == NULL)
	goto error_destroy_free;
      u = y + w * h;
      v = u + ((w * h) >> 2);
      for (i = 0; i < p->height; i += 2)
	for (j = 0; j < p->width; j += 2) {
	  y[ i       *  w +        j      ] = d[( i      * p->width + j    ) * 3    ];
	  y[ i       *  w +        j + 1  ] = d[( i      * p->width + j + 1) * 3    ];
	  y[(i + 1)  *  w +        j      ] = d[((i + 1) * p->width + j    ) * 3    ];
	  y[(i + 1)  *  w +        j + 1  ] = d[((i + 1) * p->width + j + 1) * 3    ];
	  u[(i >> 1) * (w >> 1) + (j >> 1)] = d[( i      * p->width + j    ) * 3 + 1];
	  v[(i >> 1) * (w >> 1) + (j >> 1)] = d[( i      * p->width + j    ) * 3 + 2];
	}
      if (!p->rendered.image)
	p->rendered.image = memory_create();
      memory_request_type(p->rendered.image, video_window_preferred_memory_type(vw));
      if ((d = memory_alloc(p->rendered.image, (w * h * 3) >> 1)) == NULL)
	goto error_destroy_free;
      memcpy(d, y, (w * h * 3) >> 1);
      free(y);
      p->width = w;
      p->height = h;
      p->bytes_per_line = p->width * 1.5;
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
