/*
 * avcodec.c -- avcodec audio decoder
 * (C)Copyright 2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Wed Mar 31 23:13:09 2004.
 * $Id: avcodec.c,v 1.1 2004/03/31 14:29:31 sian Exp $
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
#  error pthread is mandatory for audiodecoder_avcodec
#endif

#include <pthread.h>

#include "enfle/audiodecoder-plugin.h"
#include "avcodec/avcodec.h"
#include "utils/libstring.h"

DECLARE_AUDIODECODER_PLUGIN_METHODS;

#define AUDIODECODER_AVCODEC_PLUGIN_DESCRIPTION "avcodec Audio Decoder plugin version 0.2"

static AudioDecoderPlugin plugin = {
  .type = ENFLE_PLUGIN_AUDIODECODER,
  .name = "avcodec",
  .description = NULL,
  .author = "Hiroshi Takekawa",

  .query = query,
  .init = init
};

struct audiodecoder_avcodec {
  const char *acodec_name;
  AVCodec *acodec;
  AVCodecContext *acodec_ctx;
  unsigned char *buf;
  unsigned short int *outbuf;
  int offset, size;
  int nframe;
};

ENFLE_PLUGIN_ENTRY(audiodecoder_avcodec)
{
  AudioDecoderPlugin *adp;
  String *s;

  if ((adp = (AudioDecoderPlugin *)calloc(1, sizeof(AudioDecoderPlugin))) == NULL)
    return NULL;
  memcpy(adp, &plugin, sizeof(AudioDecoderPlugin));

  s = string_create();
  string_set(s, (const char *)AUDIODECODER_AVCODEC_PLUGIN_DESCRIPTION);
  string_catf(s, (const char *)" with " LIBAVCODEC_IDENT);
  adp->description = (const unsigned char *)strdup((const char *)string_get(s));
  string_destroy(s);

  /* avcodec initialization */
  avcodec_init();
  avcodec_register_all();

  return (void *)adp;
}

ENFLE_PLUGIN_EXIT(audiodecoder_avcodec, p)
{
  AudioDecoderPlugin *adp = p;

  if (adp->description)
    free((void *)adp->description);
  free(p);
}

/* audiodecoder plugin methods */

static AudioDecoderStatus
decode(AudioDecoder *adec, Movie *m, AudioDevice *ad, unsigned char *buf, unsigned int len, unsigned int *used_r)
{
  struct audiodecoder_avcodec *adm = (struct audiodecoder_avcodec *)adec->opaque;
  int l, out_len;

  if (adm->size <= 0) {
    if (buf == NULL)
      return AD_NEED_MORE_DATA;
    adm->buf = buf;
    adm->offset = 0;
    adm->size = len;
    *used_r = len;
  }

  l = avcodec_decode_audio(adm->acodec_ctx, adm->outbuf, &out_len,
			   adm->buf + adm->offset, adm->size);
  if (l < 0) {
    warning_fnc("avcodec: avcodec_decode_audio return %d\n", len);
    return AD_ERROR;
  }

  adm->size -= l;
  adm->offset += l;
  if (out_len == 0)
    return AD_OK;

  //debug_message_fnc("avcodec audio: consume %d bytes, output %d bytes\n", l, out_len);

  if (adm->nframe == 0) {
    /* Set up audio device. */
    m->sampleformat = _AUDIO_FORMAT_S16_LE;

    debug_message_fnc("avcodec: from acodec_ctx    %d ch %d Hz %d kbps\n", adm->acodec_ctx->channels, adm->acodec_ctx->sample_rate, adm->acodec_ctx->bit_rate);
    debug_message_fnc("avcodec: from demultiplexer %d ch %d Hz\n", m->channels, m->samplerate);
    m->channels = m->channels == 0 ? adm->acodec_ctx->channels : m->channels;
    m->samplerate = m->samplerate == 0 ? adm->acodec_ctx->sample_rate : m->samplerate;
    debug_message_fnc("avcodec: (%d format) %d ch %d Hz %d kbps\n", m->sampleformat, m->channels, m->samplerate, adm->acodec_ctx->bit_rate);
    m->sampleformat_actual = m->sampleformat;
    m->channels_actual = m->channels;
    m->samplerate_actual = m->samplerate;
    if (!m->ap->set_params(ad, &m->sampleformat_actual, &m->channels_actual, &m->samplerate_actual))
      warning_fnc("set_params went wrong: (%d format) %d ch %d Hz\n", m->sampleformat_actual, m->channels_actual, m->samplerate_actual);
  }
  adm->nframe++;

  /* write to sound card */
  m->ap->write_device(ad, (unsigned char *)adm->outbuf, out_len);

  return AD_OK;
}

static void
destroy(AudioDecoder *adec)
{
  struct audiodecoder_avcodec *adm = (struct audiodecoder_avcodec *)adec->opaque;

  if (adm) {
    if (adm->acodec_ctx) {
      avcodec_close(adm->acodec_ctx);
      av_free(adm->acodec_ctx);
    }
    if (adm->outbuf)
      av_free(adm->outbuf);
    av_free(adm);
  }
  _audiodecoder_destroy(adec);
}

static int
setup(AudioDecoder *adec)
{
  struct audiodecoder_avcodec *adm = (struct audiodecoder_avcodec *)adec->opaque;

  if ((adm->acodec = avcodec_find_decoder_by_name(adm->acodec_name)) == NULL) {
    warning_fnc("avcodec %s not found\n", adm->acodec_name);
    return 0;
  }
  debug_message_fnc("avcodec audio: %s found: ctx %p codec %p\n", adm->acodec_name, adm->acodec_ctx, adm->acodec);
  if (avcodec_open(adm->acodec_ctx, adm->acodec) < 0) {
    warning_fnc("avcodec_open() failed.\n");
    return 0;
  }
  debug_message_fnc("avcodec audio: %s opened\n", adm->acodec_name);

  return 1;
}

static unsigned int
query(unsigned int fourcc)
{
  switch (fourcc) {
  case 0:
  case WAVEFORMAT_TAG_PCM:
  case WAVEFORMAT_TAG_MS_ADPCM:
  case WAVEFORMAT_TAG_IMA_ADPCM:
  case WAVEFORMAT_TAG_MS_GSM_6_10:
  case WAVEFORMAT_TAG_MSN_Audio:
  case WAVEFORMAT_TAG_MP2:
  case WAVEFORMAT_TAG_MP3:
  case WAVEFORMAT_TAG_Voxware:
  case WAVEFORMAT_TAG_Acelp:
  case WAVEFORMAT_TAG_DivX_WMAv1:
  case WAVEFORMAT_TAG_DivX_WMAv2:
  case WAVEFORMAT_TAG_IMC:
  case WAVEFORMAT_TAG_AC3:
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
  struct audiodecoder_avcodec *adm;

  switch (fourcc) {
  case 0:
  case WAVEFORMAT_TAG_PCM:
  case WAVEFORMAT_TAG_MS_ADPCM:
  case WAVEFORMAT_TAG_IMA_ADPCM:
  case WAVEFORMAT_TAG_MS_GSM_6_10:
  case WAVEFORMAT_TAG_MSN_Audio:
  case WAVEFORMAT_TAG_MP2:
  case WAVEFORMAT_TAG_MP3:
  case WAVEFORMAT_TAG_Voxware:
  case WAVEFORMAT_TAG_Acelp:
  case WAVEFORMAT_TAG_DivX_WMAv1:
  case WAVEFORMAT_TAG_DivX_WMAv2:
  case WAVEFORMAT_TAG_IMC:
  case WAVEFORMAT_TAG_AC3:
  case WAVEFORMAT_TAG_VORBIS:
    break;
  default:
    debug_message_fnc("Audio (%08X) was not identified as any avcodec supported format.\n", fourcc);
    return NULL;
  }

  if ((adec = _audiodecoder_init()) == NULL)
    return NULL;
  if ((adec->opaque = adm = calloc(1, sizeof(*adm))) == NULL)
    goto error_adec;
  adec->name = plugin.name;
  adec->setup = setup;
  adec->decode = decode;
  adec->destroy = destroy;

  adm->acodec_name = audiodecoder_codec_name(fourcc);
  if ((adm->acodec_ctx = avcodec_alloc_context()) == NULL)
    goto error_adm;
  adm->acodec_ctx->opaque = adec;
  if ((adm->outbuf = malloc(AVCODEC_MAX_AUDIO_FRAME_SIZE)) == NULL)
    goto error_ctx;
  adm->nframe = 0;

  return adec;

 error_ctx:
  av_free(adm->acodec_ctx);
 error_adm:
  av_free(adm);
 error_adec:
  av_free(adec);
  return NULL;
}
