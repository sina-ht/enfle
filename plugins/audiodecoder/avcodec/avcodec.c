/*
 * avcodec.c -- avcodec audio decoder
 * (C)Copyright 2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Wed Sep 13 00:50:34 2017.
 *
 * SPDX-License-Identifier: GPL-2.0-only
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
#undef SWAP
#include <libavutil/common.h>
#include <libavcodec/avcodec.h>
#include "utils/libstring.h"

#define MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio

DECLARE_AUDIODECODER_PLUGIN_METHODS;

#define AUDIODECODER_AVCODEC_PLUGIN_DESCRIPTION "avcodec Audio Decoder plugin version 0.3.1"

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
  AVFrame *acodec_sample;
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
  string_set(s, AUDIODECODER_AVCODEC_PLUGIN_DESCRIPTION);
  string_catf(s, " with " LIBAVCODEC_IDENT);
  adp->description = strdup((const char *)string_get(s));
  string_destroy(s);

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
  int l, out_len = 0;

  if (adm->size <= 0) {
    if (buf == NULL)
      return AD_NEED_MORE_DATA;
    adm->buf = buf;
    adm->offset = 0;
    adm->size = len;
    if (used_r)
      *used_r = len;
    //debug_message_fnc("avcodec audio: feed %d bytes\n", adm->size);
  }

  out_len = MAX_AUDIO_FRAME_SIZE;
  {
    AVPacket avp;

    av_init_packet(&avp);
    avp.data = adm->buf + adm->offset;
    avp.size = adm->size;

    l = avcodec_send_packet(adm->acodec_ctx, &avp);
    l = avcodec_receive_frame(adm->acodec_ctx, adm->acodec_sample);

    out_len = av_samples_get_buffer_size(NULL, adm->acodec_ctx->channels,
					 adm->acodec_sample->nb_samples,
					 adm->acodec_ctx->sample_fmt, 1);
  }

  if (l < 0) {
    warning_fnc("avcodec: avcodec_decode_audio return %d\n", l);
    return AD_ERROR;
  }

  adm->size -= l;
  adm->offset += l;
  if (out_len == 0)
    return AD_NEED_MORE_DATA;

  //debug_message_fnc("avcodec audio: consume %d bytes, output %d bytes\n", l, out_len);

  if (adm->nframe == 0) {
    /* Set up audio device. */
    m->sampleformat = _AUDIO_FORMAT_S16_LE;

    debug_message_fnc("avcodec: from acodec_ctx    %d ch %d Hz %" PRId64 " kbps\n", adm->acodec_ctx->channels, adm->acodec_ctx->sample_rate, adm->acodec_ctx->bit_rate / 1024);
    debug_message_fnc("avcodec: from demultiplexer %d ch %d Hz\n", m->channels, m->samplerate);
    m->channels = m->channels == 0 ? adm->acodec_ctx->channels : m->channels;
    m->samplerate = m->samplerate == 0 ? (unsigned int)adm->acodec_ctx->sample_rate : m->samplerate;
    debug_message_fnc("avcodec: (%d format) %d ch %d Hz %" PRId64 " kbps\n", m->sampleformat, m->channels, m->samplerate, adm->acodec_ctx->bit_rate / 1024);
    m->sampleformat_actual = m->sampleformat;
    m->channels_actual = m->channels;
    m->samplerate_actual = m->samplerate;
    if (!m->ap->set_params(ad, &m->sampleformat_actual, &m->channels_actual, &m->samplerate_actual))
      warning_fnc("set_params went wrong: (%d format) %d ch %d Hz\n", m->sampleformat_actual, m->channels_actual, m->samplerate_actual);
  }
  adm->nframe++;

  /* write to sound card */
  m->ap->write_device(ad, (unsigned char *)adm->acodec_sample->data[0], out_len);

  return AD_OK;
}

static void
destroy(AudioDecoder *adec)
{
  struct audiodecoder_avcodec *adm = (struct audiodecoder_avcodec *)adec->opaque;

  if (adm) {
    if (adm->acodec_ctx) {
      if (adm->acodec_ctx->codec)
	avcodec_close(adm->acodec_ctx);
      av_free(adm->acodec_ctx);
    }
    if (adm->acodec_sample)
      av_freep(&adm->acodec_sample);
    av_free(adm);
  }
  _audiodecoder_destroy(adec);
}

static int
setup(AudioDecoder *adec, Movie *m)
{
  struct audiodecoder_avcodec *adm = (struct audiodecoder_avcodec *)adec->opaque;

  if ((adm->acodec = avcodec_find_decoder_by_name(adm->acodec_name)) == NULL) {
    warning_fnc("avcodec %s not found\n", adm->acodec_name);
    return 0;
  }
  debug_message_fnc("avcodec: from demultiplexer %d ch %d Hz %d kbps align %d extra %d bytes\n", m->channels, m->samplerate, m->bitrate / 1024, m->block_align, m->audio_extradata_size);
  adm->acodec_ctx->channels = m->channels;
  adm->acodec_ctx->sample_rate = m->samplerate;
  adm->acodec_ctx->bit_rate = m->bitrate;
  adm->acodec_ctx->block_align = m->block_align;
  adm->acodec_ctx->extradata = m->audio_extradata;
  adm->acodec_ctx->extradata_size = m->audio_extradata_size;
  movie_lock(m);
  if (avcodec_open2(adm->acodec_ctx, adm->acodec, NULL) < 0) {
    warning_fnc("avcodec_open() failed.\n");
    movie_unlock(m);
    return 0;
  }
  movie_unlock(m);

  return 1;
}

static unsigned int
query(unsigned int fourcc, void *priv __attribute__((unused)))
{
  switch (fourcc) {
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
  case WAVEFORMAT_TAG_UNKNOWN:
  case WAVEFORMAT_TAG_NONE:
  default:
    break;
  }
  return 0;
}

static AudioDecoder *
init(unsigned int fourcc, void *priv)
{
  AudioDecoder *adec;
  struct audiodecoder_avcodec *adm;

  if (!query(fourcc, priv)) {
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
  if ((adm->acodec_ctx = avcodec_alloc_context3(NULL)) == NULL)
    goto error_adm;
  adm->acodec_ctx->opaque = adec;
  if ((adm->acodec_sample = av_frame_alloc()) == NULL)
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
