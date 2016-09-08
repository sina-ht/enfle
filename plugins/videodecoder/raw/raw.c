/*
 * raw.c -- raw video decoder plugin
 * (C)Copyright 2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Jul  3 17:07:37 2005.
 * $Id: raw.c,v 1.6 2005/07/08 18:14:27 sian Exp $
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
#include <inttypes.h>

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#ifndef USE_PTHREAD
#  error pthread is mandatory for videodecoder_avcodec
#endif

#include <pthread.h>

#include "enfle/videodecoder-plugin.h"
#include "utils/libstring.h"

DECLARE_VIDEODECODER_PLUGIN_METHODS;

static VideoDecoderPlugin plugin = {
  .type = ENFLE_PLUGIN_VIDEODECODER,
  .name = "raw",
  .description = "raw Video Decoder plugin version 0.1",
  .author = "Hiroshi Takekawa",

  .query = query,
  .init = init,
};

struct videodecoder_raw {
};

ENFLE_PLUGIN_ENTRY(videodecoder_raw)
{
  VideoDecoderPlugin *vdp;

  if ((vdp = (VideoDecoderPlugin *)calloc(1, sizeof(VideoDecoderPlugin))) == NULL)
    return NULL;
  memcpy(vdp, &plugin, sizeof(VideoDecoderPlugin));

  return (void *)vdp;
}

ENFLE_PLUGIN_EXIT(videodecoder_raw, p)
{
  free(p);
}

/* videodecoder plugin methods */

static VideoDecoderStatus
decode(VideoDecoder *vdec, Movie *m, Image *p, DemuxedPacket *dp, unsigned int len, unsigned int *used_r)
{
  //struct videodecoder_raw *vdm = (struct videodecoder_raw *)vdec->opaque;
  unsigned char *buf = dp->data;
  int i;

  if (len == 0)
    return VD_NEED_MORE_DATA;
  *used_r = len;

  pthread_mutex_lock(&vdec->update_mutex);

  /* decode here */
  switch (m->v_fourcc) {
  case FCC_DIB:
  case FCC_RGB2:
    for (i = 0; i < image_height(p); i++)
      memcpy(memory_ptr(image_image(p)) + image_bpl(p) * (image_height(p) - i - 1), buf + image_bpl(p) * i, image_bpl(p));
    break;
  default:
    err_message_fnc("Unsupported fourcc %X\n", m->v_fourcc);
    pthread_mutex_unlock(&vdec->update_mutex);
    return VD_ERROR;
  }

  m->current_frame++;
  vdec->ts_base = dp->ts_base;
  vdec->pts = m->current_frame * 1000 / rational_to_double(m->framerate);
  vdec->to_render++;
  while (m->status == _PLAY && vdec->to_render > 0)
    pthread_cond_wait(&vdec->update_cond, &vdec->update_mutex);
  pthread_mutex_unlock(&vdec->update_mutex);

  return VD_OK;
}

static void
destroy(VideoDecoder *vdec)
{
  struct videodecoder_raw *vdm = (struct videodecoder_raw *)vdec->opaque;

  if (vdm)
    free(vdm);
  _videodecoder_destroy(vdec);
}

static int
setup(VideoDecoder *vdec, Movie *m, Image *p, int w, int h)
{
  //struct videodecoder_raw *vdm = (struct videodecoder_raw *)vdec->opaque;

  if (memory_alloc(image_image(p), image_bpl(p) * image_height(p)) == NULL) {
    err_message("No enough memory for image body (%d bytes).\n", image_bpl(p) * image_height(p));
    return 0;
  }

  return 1;
}

static unsigned int
query(unsigned int fourcc, void *priv)
{
  switch (fourcc) {
  case FCC_DIB:
  case FCC_RGB2:
    return IMAGE_RGB24 | IMAGE_BGR24;
  default:
    break;
  }
  return 0;
}

static VideoDecoder *
init(unsigned int fourcc, void *priv)
{
  VideoDecoder *vdec;
  struct videodecoder_raw *vdm;

  switch (fourcc) {
  case 0:
  case FCC_DIB:
  case FCC_RGB2:
    debug_message_fnc("Identified as raw RGB\n");
    break;
  default:
    debug_message_fnc("Not identified as any Raw format: %c%c%c%c(%08X)\n",
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

  return vdec;
}
