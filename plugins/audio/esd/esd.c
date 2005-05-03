/*
 * esd.c -- EsounD Audio plugin
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue May  3 09:36:32 2005.
 * $Id: esd.c,v 1.8 2005/05/03 01:08:30 sian Exp $
 *
 * Note: Audio support is incomplete.
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
#include <fcntl.h>

#include <esd.h>

#define REQUIRE_UNISTD_H
#define REQUIRE_STRING_H
#include "compat.h"

#include "common.h"

#include "enfle/audio-plugin.h"

static AudioDevice *open_device(void *, Config *);
static int set_params(AudioDevice *, AudioFormat *, int *, unsigned int *);
static int write_device(AudioDevice *, unsigned char *, int);
static int bytes_written(AudioDevice *);
static int sync_device(AudioDevice *);
static int close_device(AudioDevice *);

static AudioPlugin plugin = {
  .type = ENFLE_PLUGIN_AUDIO,
  .name = "EsounD",
  .description = "EsounD Audio plugin version 0.2",
  .author = "Hiroshi Takekawa",

  .open_device = open_device,
  .set_params = set_params,
  .write_device = write_device,
  .bytes_written = bytes_written,
  .sync_device = sync_device,
  .close_device = close_device
};

typedef struct _esd_data {
  int sfd;
  float latency_factor;
  char *server_host;
} ESD_data;

ENFLE_PLUGIN_ENTRY(audio_esd)
{
  AudioPlugin *ap;

  if ((ap = (AudioPlugin *)calloc(1, sizeof(AudioPlugin))) == NULL)
    return NULL;
  memcpy(ap, &plugin, sizeof(AudioPlugin));

  return (void *)ap;
}

ENFLE_PLUGIN_EXIT(audio_esd, p)
{
  free(p);
}

/* for internal use */

/* audio plugin methods */

static AudioDevice *
open_device(void *data, Config *c)
{
  AudioDevice *ad;
  ESD_data *esd;

  if ((ad = calloc(1, sizeof(AudioDevice))) == NULL)
    return NULL;
  if ((esd = calloc(1, sizeof(ESD_data))) == NULL) {
    free(ad);
    return NULL;
  }
  ad->private_data = esd;

  if ((esd->server_host = config_get_str(c, "/enfle/plugins/audio/esd/server")) == NULL) {
    esd->server_host = NULL;
  } else if (!strcasecmp(esd->server_host, "NULL")) {
    esd->server_host = NULL;
  }

  ad->c = c;
  ad->bytes_written = 0;
  ad->opened = 1;

  return ad;
}

static int
set_params(AudioDevice *ad, AudioFormat *format_p, int *ch_p, unsigned int *rate_p)
{
  ESD_data *esd = (ESD_data *)ad->private_data;
  int bits, channels;
  AudioFormat format = *format_p;
  int ch = *ch_p;
  int rate = *rate_p;

  if (!ad->opened)
    return 0;

  /* format part */
  switch (format) {
  case _AUDIO_FORMAT_U8:
    bits = ESD_BITS8;
    ad->bytes_per_sample = 1;
    break;
  case _AUDIO_FORMAT_S8:
    bits = ESD_BITS8;
    ad->bytes_per_sample = 1;
    break;
  case _AUDIO_FORMAT_U16_LE:
    bits = ESD_BITS16;
    ad->bytes_per_sample = 2;
    break;
  case _AUDIO_FORMAT_U16_BE:
    bits = ESD_BITS16;
    ad->bytes_per_sample = 2;
    break;
  case _AUDIO_FORMAT_S16_LE:
    bits = ESD_BITS16;
    ad->bytes_per_sample = 2;
    break;
  case _AUDIO_FORMAT_S16_BE:
    bits = ESD_BITS16;
    ad->bytes_per_sample = 2;
    break;
  default:
    show_message_fnc("format %d is invalid or unsupported.\n", format);
    ad->format = _AUDIO_FORMAT_UNSET;
    *format_p = _AUDIO_FORMAT_UNSET;
    return 0;
  }

  channels = (ch == 1) ? ESD_MONO : ESD_STEREO;

  if ((ad->fd = esd_play_stream_fallback(bits | channels | ESD_STREAM | ESD_PLAY, rate, esd->server_host, "enfle")) <= 0)
    return 0;
  if ((esd->sfd = esd_open_sound(esd->server_host)) < 0) {
    esd_close(ad->fd);
    return 0;
  }

  ad->channels = ch;
  ad->speed = rate;
  esd->latency_factor = (4 * 4 * 44100) / (ad->bytes_per_sample * ad->channels * ad->speed);

  debug_message_fnc("%d ch %d Hz %d bytes_per_sample\n", ad->channels, ad->speed, ad->bytes_per_sample);
  ad->opened = 2;

  return 1;
}

static int
write_device(AudioDevice *ad, unsigned char *data, int size)
{
  int written;

  if (ad->opened < 2)
    return 0;

  written = write(ad->fd, data, size);
  ad->bytes_written += written;

  return written;
}

static int
bytes_written(AudioDevice *ad)
{
  ESD_data *esd = (ESD_data *)ad->private_data;
  int latency;

  if (ad->opened < 2)
    return 0;

  latency = esd_get_latency(esd->sfd);

  return ad->bytes_written - (latency * esd->latency_factor);
}

static int
sync_device(AudioDevice *ad)
{
  if (ad->opened < 2)
    return 0;

  fsync(ad->fd);

  return 1;
}

static int
close_device(AudioDevice *ad)
{
  ESD_data *esd = (ESD_data *)ad->private_data;

  if (!ad->opened)
    return 0;
  ad->opened = 0;
  esd_close(esd->sfd);
  esd_close(ad->fd);
  free(esd);
  free(ad);

  return 1;
}
