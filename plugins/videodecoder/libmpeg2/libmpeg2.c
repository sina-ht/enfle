/*
 * libmpeg2.c -- libmpeg2 video decoder plugin
 * (C)Copyright 2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Feb 21 15:12:21 2004.
 * $Id: libmpeg2.c,v 1.4 2004/02/21 07:51:08 sian Exp $
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
#  error pthread is mandatory for videodecoder_libmpeg2
#endif

#include <pthread.h>

#include "enfle/videodecoder-plugin.h"
#include "mpeg2/mpeg2.h"

DECLARE_VIDEODECODER_PLUGIN_METHODS;

static VideoDecoderPlugin plugin = {
  type: ENFLE_PLUGIN_VIDEODECODER,
  name: "LibMPEG2",
  description: "libmpeg2 Video Decoder plugin version 0.1 with integrated libmpeg2(mpeg2dec-0.4.0)",
  author: "Hiroshi Takekawa",

  query: query,
  init: init
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
decode(VideoDecoder *vdec, Movie *m, Image *p, unsigned char *buf, unsigned int len, unsigned int *used_r)
{
  struct videodecoder_libmpeg2 *vdm = (struct videodecoder_libmpeg2 *)vdec->opaque;
  mpeg2_state_t state;
  const mpeg2_info_t *mpeg2dec_info;
  const mpeg2_sequence_t *seq;
  int size;

  state = mpeg2_parse(vdm->mpeg2dec);
  mpeg2dec_info = mpeg2_info(vdm->mpeg2dec);
  seq = mpeg2dec_info->sequence;

  switch (state) {
  case STATE_BUFFER:
    if (buf == NULL)
      return VD_NEED_MORE_DATA;
    mpeg2_buffer(vdm->mpeg2dec, buf, buf + len);
    if (used_r)
      *used_r = len;
    /* dp->data should not be freed until all data in dp->data is used. */
    break;
  case STATE_SEQUENCE:
    size = seq->width * seq->height + seq->chroma_width * seq->chroma_height * 2;
    if (m->width == 0) {
      m->width = seq->width;
      m->height = seq->height;
      m->framerate = 27000000.0 / seq->frame_period;
      show_message("dimension (%d, %d)  %2.5f fps\n", m->width, m->height, m->framerate);
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
      while (m->status == _PLAY && vdec->to_render > 0)
	pthread_cond_wait(&vdec->update_cond, &vdec->update_mutex);
      pthread_mutex_unlock(&vdec->update_mutex);
    }
    /* XXX: skip */
    //mpeg2_skip(info->mpeg2dec, info->to_skip);
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
setup(VideoDecoder *vdec, Movie *m, Image *p, int w, int h)
{
  return 1;
}

static unsigned int
query(unsigned int fourcc)
{
  switch (fourcc) {
  case FCC_mpg1:
  case FCC_mpg2:
  case FCC_PIM1:
  case FCC_VCR2:
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
  struct videodecoder_libmpeg2 *vdm;

  switch (fourcc) {
  case 0:
  case FCC_mpg1:
  case FCC_mpg2:
  case FCC_PIM1:
  case FCC_VCR2:
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
  vdm->mpeg2dec = mpeg2_init();

  return vdec;
}
