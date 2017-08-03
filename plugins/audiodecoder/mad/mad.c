/*
 * mad.c -- libmad audio decoder plugin
 * (C)Copyright 2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat May  1 21:01:35 2004.
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

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "enfle/audiodecoder-plugin.h"
#include "mad/mad.h"

DECLARE_AUDIODECODER_PLUGIN_METHODS;

static AudioDecoderStatus decode(AudioDecoder *, Movie *, AudioDevice *, unsigned char *, unsigned int, unsigned int *);
static void destroy(AudioDecoder *);

static AudioDecoderPlugin plugin = {
  .type = ENFLE_PLUGIN_AUDIODECODER,
  .name = "mad",
  .description = "mad Audio Decoder plugin version 0.2",
  .author = "Hiroshi Takekawa",

  .query = query,
  .init = init
};

/*
 * highest bit-rate 448 kb/s
 * input buffer must be larger than (448000*(1152/32000))/8 = 2016 bytes
 */
#define INPUT_BUFFER_SIZE 20160*2
#define OUTPUT_BUFFER_SIZE 8192 /* must be multiple of 4. */
struct audiodecoder_mad {
  struct mad_stream stream;
  struct mad_frame frame;
  struct mad_synth synth;
  mad_timer_t timer;
  unsigned char input_buffer[INPUT_BUFFER_SIZE];
  unsigned char output_buffer[OUTPUT_BUFFER_SIZE];
  int nframe;
};

ENFLE_PLUGIN_ENTRY(audiodecoder_mad)
{
  AudioDecoderPlugin *adp;

  if ((adp = (AudioDecoderPlugin *)calloc(1, sizeof(AudioDecoderPlugin))) == NULL)
    return NULL;
  memcpy(adp, &plugin, sizeof(AudioDecoderPlugin));

  return (void *)adp;
}

ENFLE_PLUGIN_EXIT(audiodecoder_mad, p)
{
  free(p);
}

/* audiodecoder plugin methods */

static inline unsigned short
fixed_to_ushort(mad_fixed_t fixed)
{
  fixed += 1L << (MAD_F_FRACBITS - 16);
  if (fixed > MAD_F_ONE - 1)
    fixed = MAD_F_ONE - 1;
  else if (fixed < -MAD_F_ONE)
    fixed = -MAD_F_ONE;
  return (unsigned short)(fixed >> (MAD_F_FRACBITS - 15));
}

static AudioDecoderStatus
decode(AudioDecoder *adec, Movie *m, AudioDevice *ad, unsigned char *buf, unsigned int len, unsigned int *used_r)
{
  struct audiodecoder_mad *adm = (struct audiodecoder_mad *)adec->opaque;
  unsigned char *ptr;
  int i;

  /* need refill? */
  if (adm->stream.buffer == NULL || adm->stream.error == MAD_ERROR_BUFLEN) {
    size_t remain;

    if (len == 0)
      return AD_NEED_MORE_DATA;

    remain = 0;
    if (adm->stream.next_frame != NULL) {
      /* Move to the top if there are any remaining bytes. */
      remain = adm->stream.bufend - adm->stream.next_frame;
      memmove(adm->input_buffer, adm->stream.next_frame, remain);
    }

    /* Fill-in the buffer. */
    *used_r = (len <= INPUT_BUFFER_SIZE - remain) ? len : INPUT_BUFFER_SIZE - remain;
    memmove(adm->input_buffer + remain, buf, *used_r);
    mad_stream_buffer(&adm->stream, adm->input_buffer, remain + *used_r);
    adm->stream.error = 0;
  } else {
    if (used_r)
      *used_r = 0;
  }

  /* Decode the next frame. */
  while (1) {
    if (!mad_frame_decode(&adm->frame, &adm->stream))
      break;

    if (!MAD_RECOVERABLE(adm->stream.error)) {
      if (adm->stream.error == MAD_ERROR_BUFLEN) {
	return AD_NEED_MORE_DATA;
      } else {
	err_message_fnc("unrecoverable error: %s\n", mad_stream_errorstr(&adm->stream));
	return AD_ERROR;
      }
    }
    debug_message_fnc("recoverable error: %s\n", mad_stream_errorstr(&adm->stream));
  }

  if (adm->nframe == 0) {
    /* Set up audio device. */
    m->sampleformat = _AUDIO_FORMAT_S16_LE;
    m->channels = MAD_NCHANNELS(&adm->frame.header);
    m->samplerate = adm->frame.header.samplerate;
    debug_message_fnc("mad: (%d format) %d ch %d Hz %ld kbps\n", m->sampleformat, m->channels, m->samplerate, adm->frame.header.bitrate / 1000);
    m->sampleformat_actual = m->sampleformat;
    m->channels_actual = m->channels;
    m->samplerate_actual = m->samplerate;
    if (!m->ap->set_params(ad, &m->sampleformat_actual, &m->channels_actual, &m->samplerate_actual))
      warning_fnc("set_params went wrong: (%d format) %d ch %d Hz\n", m->sampleformat_actual, m->channels_actual, m->samplerate_actual);
  }

  adm->nframe++;
  mad_timer_add(&adm->timer, adm->frame.header.duration);

  /* Synthesize & Conversion */
  mad_synth_frame(&adm->synth, &adm->frame);
  ptr = adm->output_buffer;
  for (i = 0; i < adm->synth.pcm.length; i++) {
    unsigned short sample;

    /* Left channel */
    sample = fixed_to_ushort(adm->synth.pcm.samples[0][i]);
    *ptr++ = sample & 0xff;
    *ptr++ = sample >> 8;

    /* Right channel */
    if (MAD_NCHANNELS(&adm->frame.header) == 2) {
      sample = fixed_to_ushort(adm->synth.pcm.samples[1][i]);
      *ptr++ = sample & 0xff;
      *ptr++ = sample >> 8;
    }
  }

  m->ap->write_device(ad, adm->output_buffer, ptr - adm->output_buffer);

  return AD_OK;
}

static void
destroy(AudioDecoder *adec)
{
  struct audiodecoder_mad *adm = (struct audiodecoder_mad *)adec->opaque;

  if (adm) {
    mad_synth_finish(&adm->synth);
    mad_frame_finish(&adm->frame);
    mad_stream_finish(&adm->stream);
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
query(unsigned int fourcc, void *priv)
{
  switch (fourcc) {
  case 0:
  case WAVEFORMAT_TAG_MP2:
  case WAVEFORMAT_TAG_MP3:
    return 1;
  default:
    break;
  }
  return 0;
}

static AudioDecoder *
init(unsigned int fourcc, void *priv)
{
  AudioDecoder *adec;
  struct audiodecoder_mad *adm;

  switch (fourcc){
  case 0:
  case WAVEFORMAT_TAG_MP2:
  case WAVEFORMAT_TAG_MP3:
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

  mad_stream_init(&adm->stream);
  mad_frame_init(&adm->frame);
  mad_synth_init(&adm->synth);
  mad_timer_reset(&adm->timer);

  return adec;
}
