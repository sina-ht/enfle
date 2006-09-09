/*
 * avcodec.c -- avcodec video decoder
 * (C)Copyright 2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Sep  9 21:48:31 2006.
 * $Id: avcodec.c,v 1.21 2006/09/09 12:54:50 sian Exp $
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
#undef SWAP
#include "avutil/common.h"
#include "avcodec/avcodec.h"
#include "utils/libstring.h"

DECLARE_VIDEODECODER_PLUGIN_METHODS;

#define VIDEODECODER_AVCODEC_PLUGIN_DESCRIPTION "avcodec Video Decoder plugin version 0.2.1"

static VideoDecoderPlugin plugin = {
  .type = ENFLE_PLUGIN_VIDEODECODER,
  .name = "avcodec",
  .description = NULL,
  .author = "Hiroshi Takekawa",

  .query = query,
  .init = init
};

//#if defined(DEBUG)
#define USE_DR1
//#endif

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
  const char *vcodec_name;
  AVCodec *vcodec;
  AVCodecContext *vcodec_ctx;
  AVFrame *vcodec_picture;
  unsigned char *buf;
  int offset, size, to_skip, if_image_alloced;
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
  string_set(s, VIDEODECODER_AVCODEC_PLUGIN_DESCRIPTION);
  string_catf(s, " with " LIBAVCODEC_IDENT);
  vdp->description = strdup((const char *)string_get(s));
  string_destroy(s);

  /* avcodec initialization */
  avcodec_init();
  avcodec_register_all();

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
  VideoDecoder *vdec = (VideoDecoder *)vcodec_ctx->opaque;
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
  VideoDecoder *vdec = (VideoDecoder *)vcodec_ctx->opaque;
  struct videodecoder_avcodec *vdm = (struct videodecoder_avcodec *)vdec->opaque;
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
decode(VideoDecoder *vdec, Movie *m, Image *p, DemuxedPacket *dp, unsigned int len, unsigned int *used_r)
{
  struct videodecoder_avcodec *vdm = (struct videodecoder_avcodec *)vdec->opaque;
  unsigned char *buf = dp->data;
  int l;
  int y;
  int got_picture;

  if (vdm->size <= 0) {
    if (len == 0)
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

  if (!vdm->if_image_alloced && vdm->vcodec_ctx->width > 0) {
    m->width = image_width(p) = vdm->vcodec_ctx->width;
    m->height = image_height(p) = vdm->vcodec_ctx->height;
    m->framerate.num = vdm->vcodec_ctx->time_base.den;
    m->framerate.den = vdm->vcodec_ctx->time_base.num;
    image_bpl(p) = vdm->vcodec_ctx->width * 2; /* XXX: hmm... */
    show_message_fnc("(%d, %d) bpl %d, fps %2.5f\n", m->width, m->height, image_bpl(p), rational_to_double(m->framerate));
    if (memory_alloc(image_renderable_image(p), image_bpl(p) * image_height(p)) == NULL) {
      err_message("No enough memory for image body (%d bytes).\n", image_bpl(p) * image_height(p));
      return VD_ERROR;
    }
    vdm->if_image_alloced = 1;
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
  if (vdm->vcodec_ctx->pix_fmt == PIX_FMT_RGB555) {
    for (y = 0; y < m->height; y++) {
      memcpy(memory_ptr(image_renderable_image(p)) + m->width * 2 * y, vdm->vcodec_picture->data[0] + vdm->vcodec_picture->linesize[0] * y, m->width * 2);
    }
  } else {
    for (y = 0; y < m->height; y++) {
      memcpy(memory_ptr(image_renderable_image(p)) + m->width * y, vdm->vcodec_picture->data[0] + vdm->vcodec_picture->linesize[0] * y, m->width);
    }
    for (y = 0; y < m->height >> 1; y++) {
      memcpy(memory_ptr(image_renderable_image(p)) + (m->width >> 1) * y + m->width * m->height, vdm->vcodec_picture->data[1] + vdm->vcodec_picture->linesize[1] * y, m->width >> 1);
    }
    for (y = 0; y < m->height >> 1; y++) {
      memcpy(memory_ptr(image_renderable_image(p)) + (m->width >> 1) * y + m->width * m->height + (m->width >> 1) * (m->height >> 1), vdm->vcodec_picture->data[2] + vdm->vcodec_picture->linesize[2] * y, m->width >> 1);
    }
  }
  vdec->ts_base = dp->ts_base;
  vdec->pts = m->current_frame * vdec->ts_base / rational_to_double(m->framerate);
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
  vdm->if_image_alloced = 0;
  if (image_width(p) > 0) {
    if (memory_alloc(image_renderable_image(p), image_bpl(p) * image_height(p)) == NULL) {
      err_message("No enough memory for image body (%d bytes).\n", image_bpl(p) * image_height(p));
      return 0;
    }
    vdm->vcodec_ctx->width = w;
    vdm->vcodec_ctx->height = h;
    vdm->if_image_alloced = 1;
  }

  if ((vdm->vcodec = avcodec_find_decoder_by_name(vdm->vcodec_name)) == NULL) {
    warning_fnc("avcodec %s not found\n", vdm->vcodec_name);
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

  vdm->vcodec_ctx->time_base.num = m->framerate.den;
  vdm->vcodec_ctx->time_base.den = m->framerate.num;

  return 1;
}

static unsigned int
query(unsigned int fourcc, void *priv)
{
  /* Should get image color type from avcodec */
  switch (fourcc) {
  case 0:
  case FCC_H263: // h263
  case FCC_I263: // h263i
  case FCC_U263: // h263p
  case FCC_viv1:
  case FCC_DIVX: // mpeg4
  case FCC_divx:
  case FCC_DX50:
  case FCC_XVID:
  case FCC_xvid:
  case FCC_MP4S:
  case FCC_M4S2:
  case FCC_0x04000000:
  case FCC_DIV1:
  case FCC_BLZ0:
  case FCC_mp4v:
  case FCC_UMP4:
  case FCC_FMP4:
  case FCC_DIV3: // msmpeg4
  case FCC_div3:
  case FCC_DIV4:
  case FCC_DIV5:
  case FCC_DIV6:
  case FCC_MP43:
  case FCC_MPG3:
  case FCC_AP41:
  case FCC_COL1:
  case FCC_COL0:
  case FCC_MP42: // msmpeg4v2
  case FCC_mp42:
  case FCC_DIV2:
  case FCC_MP41: // msmpeg4v1
  case FCC_MPG4:
  case FCC_mpg4:
  case FCC_WMV1: // wmv1
  case FCC_WMV2: // wmv2
  case FCC_dvsd: // dvvideo
  case FCC_dvhd:
  case FCC_dvsl:
  case FCC_dv25:
  case FCC_mpg1: // mpeg1video
  case FCC_mpg2:
  case FCC_PIM1:
  case FCC_VCR2:
  case FCC_mjpg: // mjpeg
  case FCC_MJPG:
  case FCC_JPGL: // ljpeg
  case FCC_LJPG:
  case FCC_HFYU: // huffyuv
  case FCC_CYUV: // cyuv
  case FCC_Y422: // rawvideo
  case FCC_I420:
  case FCC_IV31: // indeo3
  case FCC_IV32:
  case FCC_VP31: // vp3
  case FCC_ASV1: // asv1
  case FCC_ASV2: // asv2
  case FCC_VCR1: // vcr1
  case FCC_FFV1: // ffv1
  case FCC_Xxan: // xan_wc4
  case FCC_mrle: // msrle
  case FCC_0x01000000:
  case FCC_cvid: // cinepak
    return (IMAGE_I420 |
	    IMAGE_BGRA32 | IMAGE_ARGB32 |
	    IMAGE_RGB24 | IMAGE_BGR24 |
	    IMAGE_BGR565 | IMAGE_RGB565 | IMAGE_BGR555 | IMAGE_RGB555);
  case FCC_MSVC: // msvideo1
  case FCC_msvc:
  case FCC_CRAM:
  case FCC_cram:
  case FCC_WHAM:
  case FCC_wham:
    return IMAGE_RGB555 | IMAGE_INDEX;
  default:
    break;
  }
  return 0;
}

static VideoDecoder *
init(unsigned int fourcc, void *priv)
{
  VideoDecoder *vdec;
  struct videodecoder_avcodec *vdm;

  if (!query(fourcc, priv)) {
    debug_message_fnc("Video [%c%c%c%c](%08X) was not identified as any avcodec supported format.\n",
		       fourcc        & 0xff,
		      (fourcc >>  8) & 0xff,
		      (fourcc >> 16) & 0xff,
		      (fourcc >> 24) & 0xff,
		       fourcc);
    return NULL;
  }

  if ((vdec = _videodecoder_init()) == NULL)
    return NULL;
  if ((vdec->opaque = vdm = calloc(1, sizeof(*vdm))) == NULL)
    goto error_vdec;
  vdec->name = plugin.name;
  vdec->setup = setup;
  vdec->decode = decode;
  vdec->destroy = destroy;

  vdm->vcodec_name = videodecoder_codec_name(fourcc);
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
