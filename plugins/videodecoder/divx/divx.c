/*
 * divx.c -- divx video decoder plugin
 * (C)Copyright 2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Feb 21 15:12:29 2004.
 * $Id: divx.c,v 1.3 2004/02/21 07:51:08 sian Exp $
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
#include <inttypes.h>
#define LINUX
#include <decore.h>

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#if DECORE_VERSION < 20021112
#  error DECORE_VERSION should be newer than or equal to 20021112
#endif

#ifndef USE_PTHREAD
#  error pthread is mandatory for videodecoder_avcodec
#endif

#include <pthread.h>

#include "enfle/videodecoder-plugin.h"
#include "utils/libstring.h"

/* XXX: dirty... */
int divx_get_biCompression(Movie *);
short divx_get_biBitCount(Movie *);

DECLARE_VIDEODECODER_PLUGIN_METHODS;

#define VIDEODECODER_DIVX_PLUGIN_DESCRIPTION "divx Video Decoder plugin version 0.1"

static VideoDecoderPlugin plugin = {
  type: ENFLE_PLUGIN_VIDEODECODER,
  name: "DivX",
  description: NULL,
  author: "Hiroshi Takekawa",

  query: query,
  init: init
};

struct videodecoder_divx {
  Image *p;
  unsigned char *buf;
  int input_format;
  int offset, size, to_skip;
  void *dec_handle;
  DEC_FRAME dec_frame;
};

ENFLE_PLUGIN_ENTRY(videodecoder_divx)
{
  VideoDecoderPlugin *vdp;
  String *s;

  if ((vdp = (VideoDecoderPlugin *)calloc(1, sizeof(VideoDecoderPlugin))) == NULL)
    return NULL;
  memcpy(vdp, &plugin, sizeof(VideoDecoderPlugin));

  s = string_create();
  string_set(s, (const char *)VIDEODECODER_DIVX_PLUGIN_DESCRIPTION);
  string_catf(s, (const char *)" with libdivxdecore %d", DECORE_VERSION);
  vdp->description = (const unsigned char *)strdup((const char *)string_get(s));
  string_destroy(s);

  return (void *)vdp;
}

ENFLE_PLUGIN_EXIT(videodecoder_divx, p)
{
  VideoDecoderPlugin *vdp = p;

  if (vdp->description)
    free((void *)vdp->description);
  free(p);
}

/* videodecoder plugin methods */

static VideoDecoderStatus
decode(VideoDecoder *vdec, Movie *m, Image *p, unsigned char *buf, unsigned int len, unsigned int *used_r)
{
  struct videodecoder_divx *vdm = (struct videodecoder_divx *)vdec->opaque;
  int res;

  if (buf == NULL)
    return VD_NEED_MORE_DATA;
  *used_r = len;

  vdm->dec_frame.length = len;
  vdm->dec_frame.bitstream = buf;
  vdm->dec_frame.bmp = memory_ptr(image_rendered_image(p));
  vdm->dec_frame.render_flag = vdm->to_skip > 0 ? 0 : 1;
  vdm->dec_frame.stride = 0;
  pthread_mutex_lock(&vdec->update_mutex);
  if ((res = decore(vdm->dec_handle, DEC_OPT_FRAME, &vdm->dec_frame, NULL)) != DEC_OK) {
    if (res == DEC_BAD_FORMAT) {
      err_message("OPT_FRAME returns DEC_BAD_FORMAT\n");
    } else {
      err_message("OPT_FRAME returns %d\n", res);
    }
    pthread_mutex_unlock(&vdec->update_mutex);
    return VD_ERROR;
  }

  m->current_frame++;
  if (vdm->to_skip > 0) {
    vdm->to_skip--;
    pthread_mutex_unlock(&vdec->update_mutex);
    return VD_OK;
  }

  vdec->to_render++;
  while (m->status == _PLAY && vdec->to_render > 0)
    pthread_cond_wait(&vdec->update_cond, &vdec->update_mutex);
  pthread_mutex_unlock(&vdec->update_mutex);

  return VD_OK;
}

static void
destroy(VideoDecoder *vdec)
{
  struct videodecoder_divx *vdm = (struct videodecoder_divx *)vdec->opaque;

  if (vdm) {
    decore(vdm->dec_handle, DEC_OPT_RELEASE, NULL, NULL);
    free(vdm);
  }
  _videodecoder_destroy(vdec);
}

static int
setup(VideoDecoder *vdec, Movie *m, Image *p, int w, int h)
{
  struct videodecoder_divx *vdm = (struct videodecoder_divx *)vdec->opaque;
  DEC_INIT dec_init;
  DivXBitmapInfoHeader bih;
  int inst, quality;

  if (memory_alloc(image_rendered_image(p), image_bpl(p) * image_height(p)) == NULL) {
    err_message("No enough memory for image body (%d bytes).\n", image_bpl(p) * image_height(p));
    return 0;
  }

  memset(&dec_init, 0, sizeof(dec_init));
  dec_init.codec_version = vdm->input_format;
  dec_init.smooth_playback = 0;
  decore(&vdm->dec_handle, DEC_OPT_INIT, &dec_init, NULL);

  inst = DEC_ADJ_POSTPROCESSING | DEC_ADJ_SET;
  quality = 0; // 0-60
  decore(vdm->dec_handle, DEC_OPT_ADJUST, &inst, &quality);

  bih.biCompression = m->out_fourcc;
  bih.biBitCount = m->out_bitcount;
  bih.biSize = sizeof(DivXBitmapInfoHeader);
  bih.biWidth = m->width;
  bih.biHeight = m->height;
  decore(vdm->dec_handle, DEC_OPT_SETOUT, &bih, NULL);

  return 1;
}

static unsigned int
query(unsigned int fourcc)
{
  switch (fourcc) {
  case FCC_div3:
  case FCC_DIV3:
  case FCC_DIV4:
  case FCC_DIV5:
  case FCC_DIV6:
  case FCC_MP41:
  case FCC_MP43:
  case FCC_DIVX:
  case FCC_divx:
  case FCC_DX50:
    return (IMAGE_I420 |
	    IMAGE_BGRA32 | IMAGE_ARGB32 |
	    IMAGE_RGB24 | IMAGE_BGR24 |
	    IMAGE_BGR_WITH_BITMASK | IMAGE_RGB_WITH_BITMASK);
  default:
    break;
  }
  return 0;
}

static VideoDecoder *
init(unsigned int fourcc)
{
  VideoDecoder *vdec;
  struct videodecoder_divx *vdm;
  int input_format;

  switch (fourcc) {
  case 0:
  case FCC_div3:
  case FCC_DIV3:
  case FCC_DIV4:
  case FCC_DIV5:
  case FCC_DIV6:
  case FCC_MP41:
  case FCC_MP43:
    debug_message_fnc("Identified as DivX ;-) 3.11\n");
    input_format = 311;
    break;
  case FCC_DIVX:
  case FCC_divx:
    debug_message_fnc("Identified as DivX 4\n");
    input_format = 412;
    break;
  case FCC_DX50:
    debug_message_fnc("Identified as DivX 5\n");
    input_format = 500;
    break;
  default:
    debug_message_fnc("Not identified as any DivX format: %c%c%c%c(%08X)\n",
		       fourcc        & 0xff,
		      (fourcc >>  8) & 0xff,
		      (fourcc >> 16) & 0xff,
		      (fourcc >> 24) & 0xff,
		       fourcc);
    return NULL;
  }

  if ((vdec = _videodecoder_init()) == NULL)
    return NULL;
  if ((vdec->opaque = vdm = calloc(1, sizeof(*vdm))) == NULL) {
    free(vdec);
    return NULL;
  }
  vdec->name = plugin.name;
  vdec->setup = setup;
  vdec->decode = decode;
  vdec->destroy = destroy;
  vdm->input_format = input_format;

  return vdec;
}
