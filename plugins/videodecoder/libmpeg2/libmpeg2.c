/*
 * libmpeg2.c -- libmpeg2 video decoder plugin
 * (C)Copyright 2004-2005 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Jun  5 22:50:48 2006.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include <stdlib.h>
#include <inttypes.h>

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#ifndef USE_PTHREAD
#  error pthread is mandatory for videodecoder_libmpeg2
#endif

#include <pthread.h>

#include "enfle/videodecoder-plugin.h"
#include "mpeg2/mpeg2.h"

DECLARE_VIDEODECODER_PLUGIN_METHODS;

static VideoDecoderPlugin plugin = {
  .type = ENFLE_PLUGIN_VIDEODECODER,
  .name = "LibMPEG2",
  .description = "libmpeg2 Video Decoder plugin version 0.3 with integrated libmpeg2-0.5.1)",
  .author = "Hiroshi Takekawa",

  .query = query,
  .init = init
};

struct videodecoder_libmpeg2 {
  mpeg2dec_t *mpeg2dec;
};

ENFLE_PLUGIN_ENTRY(videodecoder_libmpeg2)
{
  VideoDecoderPlugin *vdp;

  if ((vdp = (VideoDecoderPlugin *)calloc(1, sizeof(VideoDecoderPlugin))) == NULL)
    return NULL;
  memcpy(vdp, &plugin, sizeof(VideoDecoderPlugin));

  return (void *)vdp;
}

ENFLE_PLUGIN_EXIT(videodecoder_libmpeg2, p)
{
  free(p);
}

/* videodecoder plugin methods */

static VideoDecoderStatus
decode(VideoDecoder *vdec, Movie *m, Image *p, DemuxedPacket *dp, unsigned int len, unsigned int *used_r)
{
  struct videodecoder_libmpeg2 *vdm = (struct videodecoder_libmpeg2 *)vdec->opaque;
  mpeg2_state_t state;
  const mpeg2_info_t *mpeg2dec_info;
  const mpeg2_sequence_t *seq;
  unsigned char *buf = dp->data;
  int size;

  state = mpeg2_parse(vdm->mpeg2dec);
  mpeg2dec_info = mpeg2_info(vdm->mpeg2dec);
  seq = mpeg2dec_info->sequence;

  switch (state) {
  case STATE_BUFFER:
    if (len == 0)
      return VD_NEED_MORE_DATA;
    mpeg2_tag_picture(vdm->mpeg2dec, dp->pts, dp->dts);
    mpeg2_buffer(vdm->mpeg2dec, buf, buf + len);
    if (used_r)
      *used_r = len;
    /* buf (== dp->data) should not be freed until all data in dp->data is used. */
    break;
  case STATE_SEQUENCE:
    debug_message_fnc("STATE_SEQUENCE\n");
    size = seq->width * seq->height + seq->chroma_width * seq->chroma_height * 2;
    if (m->width == 0) {
      m->width = seq->width;
      m->height = seq->height;
      m->framerate.num = 27000000;
      m->framerate.den = seq->frame_period;
      show_message("dimension (%d, %d)  %2.5f fps\n", m->width, m->height, rational_to_double(m->framerate));
      if (memory_alloc(image_rendered_image(p), size) == NULL) {
	show_message_fnc("No enough memory (%d bytes) for rendered image.\n", size);
	return VD_ERROR;
      }
    }
    break;
  case STATE_SLICE:
  case STATE_END:
  case STATE_INVALID_END:
    if (mpeg2dec_info->display_fbuf) {
      pthread_mutex_lock(&vdec->update_mutex);
      memcpy(memory_ptr(image_rendered_image(p)), mpeg2dec_info->display_fbuf->buf[0], seq->width * seq->height);
      memcpy(memory_ptr(image_rendered_image(p)) + seq->width * seq->height, mpeg2dec_info->display_fbuf->buf[1], seq->chroma_width * seq->chroma_height);
      memcpy(memory_ptr(image_rendered_image(p)) + seq->width * seq->height + seq->chroma_width * seq->chroma_height, mpeg2dec_info->display_fbuf->buf[2], seq->chroma_width * seq->chroma_height);
      vdec->to_render++;
      m->current_frame++;
      //vdec->ts_base = 1000;
      //vdec->pts = m->current_frame * 1000 / m->framerate;
      {
	const mpeg2_picture_t *mpeg2_pic = mpeg2dec_info->display_picture;

	vdec->ts_base = 0;
	if (mpeg2_pic) {
	  vdec->ts_base = dp->ts_base;
	  vdec->pts = dp->pts;
	  if (mpeg2_pic->tag) {
	    vdec->pts = mpeg2_pic->tag; // override demultiplexer value
#if defined(DEBUG) && 0
	    debug_message_fnc("pts %d, dts %d, type ", mpeg2_pic->tag, mpeg2_pic->tag2);
	    switch (mpeg2_pic->flags & PIC_MASK_CODING_TYPE) {
	    case 1: debug_message("I\n"); break;
	    case 2: debug_message("P\n"); break;
	    case 3: debug_message("B\n"); break;
	    case 4: debug_message("D\n"); break;
	    default: debug_message("?\n"); break;
	    }
#endif
	  }
	}
      }
      while (m->status == _PLAY && vdec->to_render > 0)
	pthread_cond_wait(&vdec->update_cond, &vdec->update_mutex);
      pthread_mutex_unlock(&vdec->update_mutex);
    }
    break;
  case STATE_SEQUENCE_REPEATED:
  case STATE_GOP:
  case STATE_PICTURE:
  case STATE_SLICE_1ST:
  case STATE_PICTURE_2ND:
    break;
  case STATE_INVALID:
    warning_fnc("STATE_INVALID: invalid stream passed.\n");
    break;
  default:
    debug_message_fnc("STATE_***UNKNOWN***\n");
    break;
  }

  return VD_OK;
}

static void
destroy(VideoDecoder *vdec)
{
  struct videodecoder_libmpeg2 *vdm = (struct videodecoder_libmpeg2 *)vdec->opaque;

  if (vdm) {
    mpeg2_close(vdm->mpeg2dec);
    free(vdm);
  }
  _videodecoder_destroy(vdec);
}

static int
setup(VideoDecoder *vdec __attribute__((unused)), Movie *m __attribute__((unused)), Image *p __attribute__((unused)), int w __attribute__((unused)), int h __attribute__((unused)))
{
  return 1;
}

static unsigned int
query(unsigned int fourcc, void *priv __attribute__((unused)))
{
  switch (fourcc) {
  case FCC_mpg1:
  case FCC_mpg2:
  case FCC_PIM1:
  case FCC_VCR2:
    return (IMAGE_I420 |
	    IMAGE_BGRA32 | IMAGE_ARGB32 |
	    IMAGE_RGB24 | IMAGE_BGR24 |
	    IMAGE_BGR565 | IMAGE_RGB565 | IMAGE_BGR555 | IMAGE_RGB555);
  default:
    break;
  }
  return 0;
}

static VideoDecoder *
init(unsigned int fourcc, void *priv __attribute__((unused)))
{
  VideoDecoder *vdec;
  struct videodecoder_libmpeg2 *vdm;

  switch (fourcc) {
  case 0:
  case FCC_mpg1:
  case FCC_mpg2:
  case FCC_PIM1:
  case FCC_VCR2:
    break;
  default:
    debug_message_fnc("Not identified as any libmpeg2 format: %c%c%c%c(%08X)\n",
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
  vdm->mpeg2dec = mpeg2_init();

  return vdec;
}
