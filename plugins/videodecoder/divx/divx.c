/*
 * divx.c -- divx video decoder plugin
 * (C)Copyright 2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Wed Jan 28 22:21:26 2004.
 * $Id: divx.c,v 1.1 2004/01/30 12:38:04 sian Exp $
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
int divx_get_input_format(Movie *);
int divx_get_biCompression(Movie *);
short divx_get_biBitCount(Movie *);

static VideoDecoder *init(void);
static VideoDecoderStatus decode(VideoDecoder *, Movie *, Image *, unsigned char *, unsigned int, unsigned int *);
static void destroy(VideoDecoder *);

#define VIDEODECODER_DIVX_PLUGIN_DESCRIPTION "divx Video Decoder plugin version 0.1"

static VideoDecoderPlugin plugin = {
  type: ENFLE_PLUGIN_VIDEODECODER,
  name: "divx",
  description: NULL,
  author: "Hiroshi Takekawa",

  init: init
};

struct videodecoder_divx {
  Image *p;
  unsigned char *buf;
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
    m->has_video = 0;
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

  memset(&dec_init, 0, sizeof(dec_init));
  dec_init.codec_version = divx_get_input_format(m);
  dec_init.smooth_playback = 0;
  decore(&vdm->dec_handle, DEC_OPT_INIT, &dec_init, NULL);

  inst = DEC_ADJ_POSTPROCESSING | DEC_ADJ_SET;
  quality = 0; // 0-60
  decore(vdm->dec_handle, DEC_OPT_ADJUST, &inst, &quality);

  bih.biCompression = divx_get_biCompression(m);
  bih.biBitCount = divx_get_biBitCount(m);
  bih.biSize = sizeof(DivXBitmapInfoHeader);
  bih.biWidth = m->width;
  bih.biHeight = m->height;
  decore(vdm->dec_handle, DEC_OPT_SETOUT, &bih, NULL);

  return 1;
}

static VideoDecoder *
init(void)
{
  VideoDecoder *vdec;
  struct videodecoder_divx *vdm;

  if ((vdec = _videodecoder_init()) == NULL)
    return NULL;
  if ((vdec->opaque = vdm = calloc(1, sizeof(*vdm))) == NULL) {
    free(vdec);
    return NULL;
  }
  vdec->setup = setup;
  vdec->decode = decode;
  vdec->destroy = destroy;

  return vdec;
}
