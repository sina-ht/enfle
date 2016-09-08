/*
 * mpglib.c -- mpglib audio decoder plugin
 * (C)Copyright 2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue May  3 09:38:01 2005.
 * $Id: mpglib.c,v 1.7 2005/05/03 01:08:30 sian Exp $
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

#include "common.h"

#include "enfle/audiodecoder-plugin.h"
#include "mpglib/mpg123.h"
#include "mpglib/mpglib.h"

DECLARE_AUDIODECODER_PLUGIN_METHODS;

static AudioDecoderStatus decode(AudioDecoder *, Movie *, AudioDevice *, unsigned char *, unsigned int, unsigned int *);
static void destroy(AudioDecoder *);

static AudioDecoderPlugin plugin = {
  .type = ENFLE_PLUGIN_AUDIODECODER,
  .name = "mpglib",
  .description = "mpglib Audio Decoder plugin version 0.1",
  .author = "Hiroshi Takekawa",

  .query = query,
  .init = init
};

#define MP3_DECODE_BUFFER_SIZE 16384
struct audiodecoder_mpglib {
  struct mpstr mp;
  int param_is_set;
  unsigned char output_buffer[MP3_DECODE_BUFFER_SIZE];
};

ENFLE_PLUGIN_ENTRY(audiodecoder_mpglib)
{
  AudioDecoderPlugin *adp;

  if ((adp = (AudioDecoderPlugin *)calloc(1, sizeof(AudioDecoderPlugin))) == NULL)
    return NULL;
  memcpy(adp, &plugin, sizeof(AudioDecoderPlugin));

  return (void *)adp;
}

ENFLE_PLUGIN_EXIT(audiodecoder_mpglib, p)
{
  free(p);
}

/* audiodecoder plugin methods */

static AudioDecoderStatus
decode(AudioDecoder *adec, Movie *m, AudioDevice *ad, unsigned char *buf, unsigned int len, unsigned int *used_r)
{
  struct audiodecoder_mpglib *adm = (struct audiodecoder_mpglib *)adec->opaque;
  int ret, write_size;

  ret = decodeMP3(&adm->mp, (char *)buf, len, (char *)adm->output_buffer, MP3_DECODE_BUFFER_SIZE, &write_size);
  if (!adm->param_is_set) {
    m->sampleformat = _AUDIO_FORMAT_S16_LE;
    m->channels = adm->mp.fr.stereo;
    m->samplerate = freqs[adm->mp.fr.sampling_frequency];
    debug_message_fnc("mpglib: (%d format) %d ch %d Hz\n", m->sampleformat, m->channels, m->samplerate);
    m->sampleformat_actual = m->sampleformat;
    m->channels_actual = m->channels;
    m->samplerate_actual = m->samplerate;
    if (!m->ap->set_params(ad, &m->sampleformat_actual, &m->channels_actual, &m->samplerate_actual))
      warning_fnc("set_params went wrong: (%d format) %d ch %d Hz\n", m->sampleformat_actual, m->channels_actual, m->samplerate_actual);
    adm->param_is_set++;
  }
  while (ret == MP3_OK) {
    m->ap->write_device(ad, adm->output_buffer, write_size);
    ret = decodeMP3(&adm->mp, NULL, 0,
		    (char *)adm->output_buffer, MP3_DECODE_BUFFER_SIZE, &write_size);
  }
  if (used_r)
    *used_r = len;

  return AD_NEED_MORE_DATA;
}

static void
destroy(AudioDecoder *adec)
{
  struct audiodecoder_mpglib *adm = (struct audiodecoder_mpglib *)adec->opaque;

  if (adm) {
    ExitMP3(&adm->mp);
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
  struct audiodecoder_mpglib *adm;

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
  adm->param_is_set = 0;

  InitMP3(&adm->mp);

  return adec;
}
