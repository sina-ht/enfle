/*
 * alsa.c -- ALSA Audio plugin
 * (C)Copyright 2000, 20001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Oct 14 21:16:49 2001.
 * $Id: alsa.c,v 1.2 2001/10/14 12:33:50 sian Exp $
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
#include <sys/ioctl.h>
#include <errno.h>
#include <alsa/asoundlib.h>

#define REQUIRE_UNISTD_H
#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "enfle/audio-plugin.h"

static AudioDevice *open_device(void *, Config *);
static int set_params(AudioDevice *, AudioFormat *, int *, int *);
static int write_device(AudioDevice *, unsigned char *, int);
static int bytes_written(AudioDevice *);
static int sync_device(AudioDevice *);
static int close_device(AudioDevice *);

static AudioPlugin plugin = {
  type: ENFLE_PLUGIN_AUDIO,
  name: "ALSA",
  description: "ALSA Audio plugin version 0.0.1",
  author: "Hiroshi Takekawa",

  open_device: open_device,
  set_params: set_params,
  write_device: write_device,
  bytes_written: bytes_written,
  sync_device: sync_device,
  close_device: close_device
};

typedef struct _alsa_data {
  snd_pcm_t *fd;
  snd_output_t *log;
} ALSA_data;

void *
plugin_entry(void)
{
  AudioPlugin *ap;

  if ((ap = (AudioPlugin *)calloc(1, sizeof(AudioPlugin))) == NULL)
    return NULL;
  memcpy(ap, &plugin, sizeof(AudioPlugin));

  return (void *)ap;
}

void
plugin_exit(void *p)
{
  free(p);
}

/* audio plugin methods */

static AudioDevice *
open_device(void *data, Config *c)
{
  AudioDevice *ad;
  ALSA_data *alsa;
  char *device = data;
  int err;

  if ((ad = calloc(1, sizeof(AudioDevice))) == NULL)
    return NULL;
  ad->c = c;
  if ((alsa = calloc(1, sizeof(ALSA_data))) == NULL) {
    free(ad);
    return NULL;
  }
  ad->private_data = alsa;

  if ((device == NULL) && ((device = config_get(c, "/enfle/plugins/audio/alsa/device")) == NULL))
    device = (char *)"default";

  err = snd_output_stdio_attach(&alsa->log, stderr, 0);

  if ((err = snd_pcm_open(&alsa->fd, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
    show_message(__FUNCTION__ ": error in opening device %s\n", device);
    perror("ALSA");
    return NULL;
  }

  debug_message(__FUNCTION__ ": opened device %s successfully.\n", device);

  return ad;
}

static int
set_params(AudioDevice *ad, AudioFormat *format_p, int *ch_p, int *rate_p)
{
  ALSA_data *alsa = (ALSA_data *)ad->private_data;
  int r = 0, err;
  snd_pcm_format_t f;
  AudioFormat format = *format_p;
  snd_pcm_hw_params_t *hwparams;
  snd_pcm_sw_params_t *swparams;
  int ch = *ch_p;
  int rate = *rate_p;

  snd_pcm_hw_params_alloca(&hwparams);
  snd_pcm_sw_params_alloca(&swparams);

  ad->format = _AUDIO_FORMAT_UNSET;
  *format_p = _AUDIO_FORMAT_UNSET;
  *ch_p = 0;
  *rate_p = 0;

  /* format part */
  switch (format) {
  case _AUDIO_FORMAT_MU_LAW:
    f = SND_PCM_FORMAT_MU_LAW;
    break;
  case _AUDIO_FORMAT_A_LAW:
    f = SND_PCM_FORMAT_A_LAW;
    break;
  case _AUDIO_FORMAT_ADPCM:
    f = SND_PCM_FORMAT_IMA_ADPCM;
    break;
  case _AUDIO_FORMAT_U8:
    f = SND_PCM_FORMAT_U8;
    break;
  case _AUDIO_FORMAT_S8:
    f = SND_PCM_FORMAT_S8;
    break;
  case _AUDIO_FORMAT_U16_LE:
    f = SND_PCM_FORMAT_U16_LE;
    break;
  case _AUDIO_FORMAT_U16_BE:
    f = SND_PCM_FORMAT_U16_BE;
    break;
  case _AUDIO_FORMAT_S16_LE:
    f = SND_PCM_FORMAT_S16_LE;
    break;
  case _AUDIO_FORMAT_S16_BE:
    f = SND_PCM_FORMAT_S16_BE;
    break;
#if 0
  case _AUDIO_FORMAT_S32_LE:
    f = SND_PCM_FORMAT_U32_LE;
    break;
  case _AUDIO_FORMAT_S32_BE:
    f = SND_PCM_FORMAT_S32_BE;
    break;
#endif
  default:
    show_message(__FUNCTION__ ": format %d is invalid.\n", format);
    return 0;
  }

  if ((err = snd_pcm_hw_params_any(alsa->fd, hwparams)) < 0) {
    show_message(__FUNCTION__ ": snd_pcm_hw_params_any() failed.\n");
    return 0;
  }
  if ((err = snd_pcm_hw_params_set_access(alsa->fd, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    show_message(__FUNCTION__ ": snd_pcm_hw_params_set_access() failed.\n");
    return 0;
  }
  if ((err = snd_pcm_hw_params_set_format(alsa->fd, hwparams, f)) < 0) {
    show_message(__FUNCTION__ ": snd_pcm_hw_params_set_format() failed.\n");
    return 0;
  }
  ad->format = format;
  *format_p = format;

  /* channel part */
  if ((err = snd_pcm_hw_params_set_channels(alsa->fd, hwparams, ch)) < 0) {
    show_message(__FUNCTION__ ": snd_pcm_hw_params_set_channels() failed.\n");
    return 0;
  }
  *ch_p = ch;

  /* rate part */
  if ((err = snd_pcm_hw_params_set_rate_near(alsa->fd, hwparams, rate, 0)) < 0) {
    show_message(__FUNCTION__ ": snd_pcm_hw_params_set_rate_near() failed.\n");
    return 0;
  }
  *rate_p = r;

  /* buffer & period */
  if ((err = snd_pcm_hw_params_set_buffer_time_near(alsa->fd, hwparams, 500000, 0)) < 0) {
    show_message(__FUNCTION__ ": snd_pcm_hw_params_set_buffer_near() failed.\n");
    return 0;
  }
  if ((err = snd_pcm_hw_params_set_period_time_near(alsa->fd, hwparams, 500000 / 4, 0)) < 0) {
    show_message(__FUNCTION__ ": snd_pcm_hw_params_set_period_near() failed.\n");
    return 0;
  }

  if ((err = snd_pcm_hw_params(alsa->fd, hwparams)) < 0) {
    show_message(__FUNCTION__ ": snd_pcm_hw_params() failed.\n");
    return 0;
  }

  ad->channels = snd_pcm_hw_params_get_channels(hwparams);
  ad->speed = snd_pcm_hw_params_get_rate(hwparams, NULL);

#ifdef DEBUG
  debug_message(__FUNCTION__ ": format ");
  switch (snd_pcm_hw_params_get_format(hwparams)) {
  case SND_PCM_FORMAT_MU_LAW:    debug_message("MU_LAW "); break;
  case SND_PCM_FORMAT_A_LAW:     debug_message("A_LAW "); break;
  case SND_PCM_FORMAT_IMA_ADPCM: debug_message("ADPCM "); break;
  case SND_PCM_FORMAT_U8:        debug_message("U8 "); break;
  case SND_PCM_FORMAT_S8:        debug_message("S8 "); break;
  case SND_PCM_FORMAT_U16_LE:    debug_message("U16LE "); break;
  case SND_PCM_FORMAT_U16_BE:    debug_message("U16BE "); break;
  case SND_PCM_FORMAT_S16_LE:    debug_message("S16LE "); break;
  case SND_PCM_FORMAT_S16_BE:    debug_message("S16BE "); break;
#if 0
  case SND_PCM_FORMAT_U32_LE:    debug_message("U32LE "); break;
  case SND_PCM_FORMAT_S32_BE:    debug_message("S32BE "); break;
#endif
  default:                       debug_message("UNKNOWN "); break;
  }

  debug_message("%d ch %d Hz OK\n", ad->channels, ad->speed);
#endif

  return 1;
}

static int
write_device(AudioDevice *ad, unsigned char *data, int size)
{
  ALSA_data *alsa = (ALSA_data *)ad->private_data;
  ssize_t r;
  int unit = 2 * ad->channels;
  ssize_t count = size / unit;

  while (count > 0) {
    if ((r = snd_pcm_writei(alsa->fd, data, count)) == -EAGAIN) {
      snd_pcm_wait(alsa->fd, 1000);
    } else if (r > 0) {
      count -= r;
      data += r * unit;
    }
  }

  return 1;
}

static int
bytes_written(AudioDevice *ad)
{
  return -1;
}

static int
sync_device(AudioDevice *ad)
{
  return 1;
}

static int
close_device(AudioDevice *ad)
{
  ALSA_data *alsa = (ALSA_data *)ad->private_data;

  snd_pcm_close(alsa->fd);
  snd_output_close(alsa->log);
  free(ad);

  return 1;
}
