/*
 * avcodec.c -- avcodec video decoder
 * (C)Copyright 2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Wed Jan 28 22:20:58 2004.
 * $Id: avcodec.c,v 1.1 2004/01/30 12:38:04 sian Exp $
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

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#ifndef USE_PTHREAD
#  error pthread is mandatory for videodecoder_avcodec
#endif

#include <pthread.h>

#include "enfle/videodecoder-plugin.h"
#include "avcodec/avcodec.h"
#include "utils/libstring.h"

/* XXX: dirty... */
enum CodecID avcodec_get_vcodec_id(Movie *);

static VideoDecoder *init(void);
static int setup(VideoDecoder *, Movie *, Image *, int, int);
static VideoDecoderStatus decode(VideoDecoder *, Movie *, Image *, unsigned char *, unsigned int, unsigned int *);
static void destroy(VideoDecoder *);

#define VIDEODECODER_AVCODEC_PLUGIN_DESCRIPTION "avcodec Video Decoder plugin version 0.2"

static VideoDecoderPlugin plugin = {
  type: ENFLE_PLUGIN_VIDEODECODER,
  name: "avcodec",
  description: NULL,
  author: "Hiroshi Takekawa",

  init: init
};

#if defined(DEBUG)
//#define USE_DR1
#endif

#if defined(USE_DR1)
struct pic_buf {
  int idx;
  Memory *mem;
  unsigned char data[0];
};

typedef struct __picture_buffer {
  Memory *base;
  struct pic_buf *pb;
  int linesize[3];
} Picture_buffer;

#define N_PICTURE_BUFFER 32
#endif

struct videodecoder_avcodec {
  Image *p;
  AVCodec *vcodec;
  AVCodecContext *vcodec_ctx;
  AVFrame *vcodec_picture;
  unsigned char *buf;
  int offset, size, to_skip;
#if defined(USE_DR1)
  Picture_buffer picture_buffer[N_PICTURE_BUFFER];
  int picture_buffer_count;
#endif
};

ENFLE_PLUGIN_ENTRY(videodecoder_avcodec)
{
  VideoDecoderPlugin *vdp;
  String *s;

  if ((vdp = (VideoDecoderPlugin *)calloc(1, sizeof(VideoDecoderPlugin))) == NULL)
    return NULL;
  memcpy(vdp, &plugin, sizeof(VideoDecoderPlugin));

  s = string_create();
  string_set(s, (const char *)VIDEODECODER_AVCODEC_PLUGIN_DESCRIPTION);
  string_catf(s, (const char *)" with " LIBAVCODEC_IDENT);
  vdp->description = (const unsigned char *)strdup((const char *)string_get(s));
  string_destroy(s);

  /* avcodec initialization */
  avcodec_init();
  avcodec_register_all();
  //av_log_set_level(AV_LOG_INFO);

  return (void *)vdp;
}

ENFLE_PLUGIN_EXIT(videodecoder_avcodec, p)
{
  VideoDecoderPlugin *vdp = p;

  if (vdp->description)
    free((void *)vdp->description);
  free(p);
}

/* videodecoder plugin methods */

#if defined(USE_DR1)
static int
get_buffer(AVCodecContext *vcodec_ctx, AVFrame *vcodec_picture)
{
  struct videodecoder_avcodec *vdm = (struct videodecoder_avcodec *)vdec->opaque;
  int width, height;
  Picture_buffer *buf;

  /* alignment */
  width  = (vcodec_ctx->width  + 15) & ~15;
  height = (vcodec_ctx->height + 15) & ~15;

  if (vcodec_ctx->pix_fmt != PIX_FMT_YUV420P ||
      width != vcodec_ctx->width || height != vcodec_ctx->height) {
    debug_message_fnc("avcodec: unsupported frame format, DR1 disabled.\n");
    vdm->vcodec_ctx->get_buffer = avcodec_default_get_buffer;
    vdm->vcodec_ctx->reget_buffer = avcodec_default_reget_buffer;
    vdm->vcodec_ctx->release_buffer = avcodec_default_release_buffer;
    return avcodec_default_get_buffer(vcodec_ctx, vcodec_picture);
  }

  buf = &vdm->picture_buffer[vdm->picture_buffer_count];
  if (buf->base == NULL) {
    int datasize = image_bpl(vdm->p) * image_height(vdm->p);
    int size = sizeof(struct pic_buf) + datasize;
    struct pic_buf *pb;

    if ((buf->base = memory_create()) == NULL) {
      err_message_fnc("No enough memory for Memory object buf->base.\n");
      return -1;
    }

    if ((pb = memory_alloc(buf->base, size)) == NULL) {
      err_message_fnc("Cannot allocate %d bytes.  No enough memory for pic_buf.\n", size);
      return -1;
    }
    pb->idx = vdm->picture_buffer_count;
    pb->mem = buf->base;
    memset(pb->data, 128, datasize);

    buf->pb = pb;
    buf->linesize[0] = image_width(vdm->p);
    buf->linesize[1] = image_width(vdm->p) >> 1;
    buf->linesize[2] = image_width(vdm->p) >> 1;
  }

  vcodec_picture->base[0] = vcodec_picture->data[0] =
    buf->pb->data;
  vcodec_picture->base[1] = vcodec_picture->data[1] =
    vcodec_picture->data[0] + image_width(vdm->p) * image_height(vdm->p);
  vcodec_picture->base[2] = vcodec_picture->data[2] =
    vcodec_picture->data[1] + (image_width(vdm->p) >> 1) * (image_height(vdm->p) >> 1);
  vcodec_picture->linesize[0] = buf->linesize[0];
  vcodec_picture->linesize[1] = buf->linesize[1];
  vcodec_picture->linesize[2] = buf->linesize[2];

  vcodec_picture->age = 256 * 256 * 256 * 64;
  vcodec_picture->type = FF_BUFFER_TYPE_USER;
  vdm->picture_buffer_count++;

  return 0;
}

static int
reget_buffer(AVCodecContext *vcodec_ctx, AVFrame *vcodec_picture)
{
    if (vcodec_picture->data[0] == NULL) {
        vcodec_picture->buffer_hints |= FF_BUFFER_HINTS_READABLE;
        return vcodec_ctx->get_buffer(vcodec_ctx, vcodec_picture);
    }

    return 0;
}

static void
release_buffer(AVCodecContext *vcodec_ctx, AVFrame *vcodec_picture)
{
  avcodec_info *info = (avcodec_info *)vcodec_ctx->opaque;
  struct pic_buf *pb;
  Picture_buffer *buf, *last, t;

  pb = (struct pic_buf *)(vcodec_picture->data[0] - sizeof(*pb));
  buf = &vdm->picture_buffer[pb->idx];
  last = &vdm->picture_buffer[--vdm->picture_buffer_count];

  t = *buf;
  *buf = *last;
  *last = t;

  vcodec_picture->data[0] = NULL;
  vcodec_picture->data[1] = NULL;
  vcodec_picture->data[2] = NULL;
}
#endif

static VideoDecoderStatus
decode(VideoDecoder *vdec, Movie *m, Image *p, unsigned char *buf, unsigned int len, unsigned int *used_r)
{
  struct videodecoder_avcodec *vdm = (struct videodecoder_avcodec *)vdec->opaque;
  int l;
  int got_picture;

  if (vdm->size <= 0) {
    if (buf == NULL)
      return VD_NEED_MORE_DATA;
    vdm->buf = buf;
    vdm->offset = 0;
    vdm->size = len;
    *used_r = len;
  }

  l = avcodec_decode_video(vdm->vcodec_ctx, vdm->vcodec_picture, &got_picture,
			   vdm->buf + vdm->offset, vdm->size);
  if (l < 0) {
    warning_fnc("avcodec: avcodec_decode_video return %d\n", len);
    return VD_ERROR;
  }
  vdm->size -= l;
  vdm->offset += l;
  if (!got_picture)
    return VD_OK;

  m->current_frame++;
  if (vdm->to_skip > 0) {
    vdm->to_skip--;
    return VD_OK;
  }

  pthread_mutex_lock(&vdec->update_mutex);
#if defined(USE_DR1)
  if (vdm->vcodec_ctx->get_buffer == get_buffer) {
    struct pic_buf *pb;

    pb = (struct pic_buf *)(vdm->vcodec_picture->data[0] - sizeof(*pb));
    image_rendered_set_image(vdm->p, pb->mem);
  } else
#endif
  {
    int y;

    for (y = 0; y < m->height; y++) {
      memcpy(memory_ptr(image_rendered_image(p)) + m->width * y, vdm->vcodec_picture->data[0] + vdm->vcodec_picture->linesize[0] * y, m->width);
    }
    for (y = 0; y < m->height >> 1; y++) {
      memcpy(memory_ptr(image_rendered_image(p)) + (m->width >> 1) * y + m->width * m->height, vdm->vcodec_picture->data[1] + vdm->vcodec_picture->linesize[1] * y, m->width >> 1);
    }
    for (y = 0; y < m->height >> 1; y++) {
      memcpy(memory_ptr(image_rendered_image(p)) + (m->width >> 1) * y + m->width * m->height + (m->width >> 1) * (m->height >> 1), vdm->vcodec_picture->data[2] + vdm->vcodec_picture->linesize[2] * y, m->width >> 1);
    }
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
  struct videodecoder_avcodec *vdm = (struct videodecoder_avcodec *)vdec->opaque;

  if (vdm) {
    if (vdm->vcodec_ctx) {
      avcodec_close(vdm->vcodec_ctx);
      av_free(vdm->vcodec_ctx);
    }
    if (vdm->vcodec_picture)
      av_free(vdm->vcodec_picture);
#if defined(USE_DR1)
    /* free picture_buffer */
    {
      int i;
      for (i = 0; i < vdm->picture_buffer_count; i++) {
	if (vdm->picture_buffer[i].base)
	  memory_destroy(vdm->picture_buffer[i].base);
      }
    }
#endif
    av_free(vdm);
  }
  _videodecoder_destroy(vdec);
}


static int
setup(VideoDecoder *vdec, Movie *m, Image *p, int w, int h)
{
  struct videodecoder_avcodec *vdm = (struct videodecoder_avcodec *)vdec->opaque;

  vdm->p = p;
  vdm->vcodec_ctx->width = w;
  vdm->vcodec_ctx->height = h;

  if ((vdm->vcodec = avcodec_find_decoder(avcodec_get_vcodec_id(m))) == NULL) {
    warning_fnc("avcodec id:%d not found\n", avcodec_get_vcodec_id(m));
    return 0;
  }
  if (vdm->vcodec->capabilities & CODEC_CAP_TRUNCATED)
    vdm->vcodec_ctx->flags |= CODEC_FLAG_TRUNCATED;
#if defined(USE_DR1)
  if (vdm->vcodec->capabilities & CODEC_CAP_DR1)
    vdm->vcodec_ctx->flags |= CODEC_FLAG_EMU_EDGE;
#endif
  if (avcodec_open(vdm->vcodec_ctx, vdm->vcodec) < 0) {
    warning_fnc("avcodec_open() failed.\n");
    return 0;
  }
  if (vdm->vcodec_ctx->pix_fmt == PIX_FMT_YUV420P &&
      vdm->vcodec->capabilities & CODEC_CAP_DR1) {
#if defined(USE_DR1)
    vdm->vcodec_ctx->get_buffer = get_buffer;
    vdm->vcodec_ctx->reget_buffer = reget_buffer;
    vdm->vcodec_ctx->release_buffer = release_buffer;
    show_message("avcodec: DR1 direct rendering enabled.\n");
#else
    // XXX: direct rendering causes dirty video output...
    show_message("avcodec: DR1 direct rendering disabled.\n");
#endif
  }

  return 1;
}

static VideoDecoder *
init(void)
{
  VideoDecoder *vdec;
  struct videodecoder_avcodec *vdm;

  if ((vdec = _videodecoder_init()) == NULL)
    return NULL;
  if ((vdec->opaque = vdm = calloc(1, sizeof(*vdm))) == NULL)
    goto error_vdec;
  vdec->setup = setup;
  vdec->decode = decode;
  vdec->destroy = destroy;

  if ((vdm->vcodec_ctx = avcodec_alloc_context()) == NULL)
    goto error_vdm;
  //vdm->vcodec_ctx->error_resilience = 3;
  vdm->vcodec_ctx->pix_fmt = -1;
  vdm->vcodec_ctx->opaque = vdec;
  if ((vdm->vcodec_picture = avcodec_alloc_frame()) == NULL)
    goto error_ctx;

  return vdec;

 error_ctx:
  av_free(vdm->vcodec_ctx);
 error_vdm:
  av_free(vdm);
 error_vdec:
  av_free(vdec);
  return NULL;
}
