/*
 * vorbis.c -- vorbis audio decoder plugin
 * (C)Copyright 2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Thu Apr  1 23:05:45 2004.
 * $Id: vorbis.c,v 1.4 2004/04/05 15:47:58 sian Exp $
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

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "enfle/audiodecoder-plugin.h"
#include "vorbis/codec.h"

DECLARE_AUDIODECODER_PLUGIN_METHODS;

static AudioDecoderStatus decode(AudioDecoder *, Movie *, AudioDevice *, unsigned char *, unsigned int, unsigned int *);
static void destroy(AudioDecoder *);

static AudioDecoderPlugin plugin = {
  .type = ENFLE_PLUGIN_AUDIODECODER,
  .name = "vorbis",
  .description = "vorbis Audio Decoder plugin version 0.1",
  .author = "Hiroshi Takekawa",

  .query = query,
  .init = init
};

typedef enum vorbis_state {
  _WAIT_HEADER,
  _WAIT_COMMENT,
  _WAIT_CODEBOOK,
  _DECODING
} VorbisState;

struct audiodecoder_vorbis {
  vorbis_info vi;
  vorbis_comment vc;
  vorbis_dsp_state vd;
  vorbis_block vb;
  VorbisState vs;
  int nframe;
};

ENFLE_PLUGIN_ENTRY(audiodecoder_vorbis)
{
  AudioDecoderPlugin *adp;

  if ((adp = (AudioDecoderPlugin *)calloc(1, sizeof(AudioDecoderPlugin))) == NULL)
    return NULL;
  memcpy(adp, &plugin, sizeof(AudioDecoderPlugin));

  return (void *)adp;
}

ENFLE_PLUGIN_EXIT(audiodecoder_vorbis, p)
{
  free(p);
}

/* audiodecoder plugin methods */

#define OUTPUT_BUFFER_SIZE 4096
static AudioDecoderStatus
decode(AudioDecoder *adec, Movie *m, AudioDevice *ad, unsigned char *buf, unsigned int len, unsigned int *used_r)
{
  struct audiodecoder_vorbis *adm = (struct audiodecoder_vorbis *)adec->opaque;
  ogg_packet op;
  int i, convsize, nsamples;
  float **pcm;
  ogg_int16_t output_buffer[OUTPUT_BUFFER_SIZE];

  if (len == 0)
    return AD_NEED_MORE_DATA;

  op.packet = buf;
  op.bytes = len;
  *used_r = len;

  switch (adm->vs) {
  case _WAIT_HEADER:
    if (vorbis_synthesis_headerin(&adm->vi, &adm->vc, &op) < 0) {
      err_message_fnc("vorbis_synthesis_headerin() failed.\n");
      return AD_ERROR;
    }
    adm->vs = _WAIT_COMMENT;
    return AD_OK;
  case _WAIT_COMMENT:
    if (vorbis_synthesis_headerin(&adm->vi, &adm->vc, &op) < 0) {
      err_message_fnc("vorbis_synthesis_headerin() failed.\n");
      return AD_ERROR;
    }
    adm->vs = _WAIT_CODEBOOK;
    return AD_OK;
  case _WAIT_CODEBOOK:
    if (vorbis_synthesis_headerin(&adm->vi, &adm->vc, &op) < 0) {
      err_message_fnc("vorbis_synthesis_headerin() failed.\n");
      return AD_ERROR;
    }
    {
      char **comments = adm->vc.user_comments;

      show_message("Encoded by %s\n", adm->vc.vendor);
      if (*comments) {
	show_message("Comment:\n");
	while (*comments) {
	  show_message(" %s\n", *comments);
	  ++comments;
	}
      }
    }

    vorbis_synthesis_init(&adm->vd, &adm->vi);
    vorbis_block_init(&adm->vd, &adm->vb);

    if (adm->nframe == 0) {
      /* Set up audio device. */
      m->sampleformat = _AUDIO_FORMAT_S16_LE;
      m->channels = adm->vi.channels;
      m->samplerate = adm->vi.rate;
      debug_message_fnc("vorbis: (%d format) %d ch %d Hz %f kbps\n", m->sampleformat, m->channels, m->samplerate, adm->vi.bitrate_nominal / 1000.0f);
      if (adm->vi.bitrate_upper > 0)
	debug_message_fnc("Bitrate upper %ld bps\n", adm->vi.bitrate_upper);
      if (adm->vi.bitrate_lower > 0)
	debug_message_fnc("Bitrate lower %ld bps\n", adm->vi.bitrate_lower);
      m->sampleformat_actual = m->sampleformat;
      m->channels_actual = m->channels;
      m->samplerate_actual = m->samplerate;
      if (!m->ap->set_params(ad, &m->sampleformat_actual, &m->channels_actual, &m->samplerate_actual))
	warning_fnc("set_params went wrong: (%d format) %d ch %d Hz\n", m->sampleformat_actual, m->channels_actual, m->samplerate_actual);
    }

    adm->vs = _DECODING;
    return AD_OK;
  case _DECODING:
    break;
  default:
    err_message_fnc("Unknown state %d\n", adm->vs);
    return AD_ERROR;
  }

  convsize = OUTPUT_BUFFER_SIZE / adm->vi.channels;

  if (vorbis_synthesis(&adm->vb, &op) == 0)
    vorbis_synthesis_blockin(&adm->vd, &adm->vb);

  adm->nframe++;
  while ((nsamples = vorbis_synthesis_pcmout(&adm->vd, &pcm)) > 0) {
    int j;
    int out = (nsamples < convsize ? nsamples : convsize);

    for (i = 0; i < adm->vi.channels; i++) {
      ogg_int16_t *ptr = output_buffer + i;
      float *mono = pcm[i];

      for (j = 0; j < out; j++) {
	int val = mono[j] * 32767.0f;

	if (val > 32767) {
	  val = 32767;
	  warning("Clipping in frame %ld\n", (long)(adm->vd.sequence));
	} else if (val < -32768) {
	  val = -32768;
	  warning("Clipping in frame %ld\n", (long)(adm->vd.sequence));
	}
	*ptr = val;
	ptr += adm->vi.channels;
      }
    }
    m->ap->write_device(ad, (unsigned char *)output_buffer, out * adm->vi.channels * 2);
    /* tell libvorbis how many samples we actually consumed */
    vorbis_synthesis_read(&adm->vd, out);
  }

  return AD_OK;
}

static void
destroy(AudioDecoder *adec)
{
  struct audiodecoder_vorbis *adm = (struct audiodecoder_vorbis *)adec->opaque;

  if (adm) {
    vorbis_block_clear(&adm->vb);
    vorbis_dsp_clear(&adm->vd);
    vorbis_comment_clear(&adm->vc);
    vorbis_info_clear(&adm->vi);
    free(adm);
  }
  free(adec);
}

static int
setup(AudioDecoder *adec, Movie *m)
{
  return 1;
}

static unsigned int
query(unsigned int fourcc)
{
  switch (fourcc) {
  case 0:
  case WAVEFORMAT_TAG_VORBIS:
    return 1;
  default:
    break;
  }
  return 0;
}

static AudioDecoder *
init(unsigned int fourcc)
{
  AudioDecoder *adec;
  struct audiodecoder_vorbis *adm;

  switch (fourcc){
  case 0:
  case WAVEFORMAT_TAG_VORBIS:
    break;
  default:
    return NULL;
  }

  if ((adec = calloc(1, sizeof(*adec))) == NULL)
    return NULL;
  if ((adec->opaque = adm = calloc(1, sizeof(*adm))) == NULL) {
    free(adec);
    return NULL;
  }
  adec->name = plugin.name;
  adec->setup = setup;
  adec->decode = decode;
  adec->destroy = destroy;
  adm->nframe = 0;
  adm->vs = _WAIT_HEADER;

  vorbis_info_init(&adm->vi);
  vorbis_comment_init(&adm->vc);

  return adec;
}
