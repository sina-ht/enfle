/*
 * alsa.c -- ALSA Audio plugin
 * (C)Copyright 2000-2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Nov 18 23:07:25 2011.
 * $Id: alsa.c,v 1.18 2008/04/19 09:28:05 sian Exp $
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
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <alsa/asoundlib.h>

#define REQUIRE_UNISTD_H
#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "enfle/audio-plugin.h"
#include "utils/libstring.h"

static AudioDevice *open_device(void *, Config *);
static int set_params(AudioDevice *, AudioFormat *, int *, unsigned int *);
static int write_device(AudioDevice *, unsigned char *, int);
static int bytes_written(AudioDevice *);
static int sync_device(AudioDevice *);
static int close_device(AudioDevice *);

#define AUDIO_ALSA_PLUGIN_DESCRIPTION "ALSA Audio plugin version 0.2.1"

static AudioPlugin plugin = {
  .type = ENFLE_PLUGIN_AUDIO,
  .name = "ALSA",
  .author = "Hiroshi Takekawa",
  .description = NULL,
  .open_device = open_device,
  .set_params = set_params,
  .write_device = write_device,
  .bytes_written = bytes_written,
  .sync_device = sync_device,
  .close_device = close_device
};

typedef struct _alsa_data {
  snd_pcm_t *fd;
  snd_pcm_uframes_t buffer_size;
  snd_pcm_uframes_t period_size;
  snd_output_t *log;
} ALSA_data;

ENFLE_PLUGIN_ENTRY(audio_alsa)
{
  AudioPlugin *ap;
  String *s;

  if ((ap = (AudioPlugin *)calloc(1, sizeof(AudioPlugin))) == NULL)
    return NULL;
  memcpy(ap, &plugin, sizeof(AudioPlugin));
  s = string_create();
  string_set(s, AUDIO_ALSA_PLUGIN_DESCRIPTION);
  string_catf(s, " with alsa-lib version %s (compiled with " SND_LIB_VERSION_STR ")", snd_asoundlib_version());
  ap->description = strdup(string_get(s));
  string_destroy(s);

  return (void *)ap;
}

ENFLE_PLUGIN_EXIT(audio_alsa, p)
{
  AudioPlugin *ap = (AudioPlugin *)p;

  if (ap->description)
	free((void *)ap->description);
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

  if ((device == NULL) && ((device = config_get_str(c, "/enfle/plugins/audio/alsa/device")) == NULL))
    device = (char *)"default";

  err = snd_output_stdio_attach(&alsa->log, stderr, 0);

  if ((err = snd_pcm_open(&alsa->fd, device, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK)) < 0) {
    show_message_fnc("error in opening device \"%s\"\n", device);
    return NULL;
  }

  ad->bytes_written = 0;
  ad->opened = 1;

  debug_message_fnc("opened device %s successfully.\n", device);

  return ad;
}

static int
set_params(AudioDevice *ad, AudioFormat *format_p, int *ch_p, unsigned int *rate_p)
{
  ALSA_data *alsa = (ALSA_data *)ad->private_data;
  int r, err;
  snd_pcm_format_t f;
  AudioFormat format = *format_p;
  snd_pcm_hw_params_t *hwparams;
  snd_pcm_sw_params_t *swparams;
  int ch = *ch_p;
  unsigned int rate = *rate_p;
  unsigned int buffer, period;

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
    show_message_fnc("format %d is invalid.\n", format);
    return 0;
  }

  if ((err = snd_pcm_hw_params_any(alsa->fd, hwparams)) < 0) {
    show_message_fnc("snd_pcm_hw_params_any() failed.\n");
    return 0;
  }

  if ((err = snd_pcm_hw_params_set_access(alsa->fd, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    show_message_fnc("snd_pcm_hw_params_set_access() failed.\n");
    return 0;
  }

  if ((err = snd_pcm_hw_params_set_format(alsa->fd, hwparams, f)) < 0) {
    show_message_fnc("snd_pcm_hw_params_set_format() failed.\n");
    return 0;
  }
  ad->format = format;
  *format_p = format;

  /* channel part */
  if ((err = snd_pcm_hw_params_set_channels(alsa->fd, hwparams, ch)) < 0) {
    show_message_fnc("snd_pcm_hw_params_set_channels() failed.\n");
    return 0;
  }
  *ch_p = ch;

  /* rate part */
  if ((err = snd_pcm_hw_params_set_rate_near(alsa->fd, hwparams, &rate, 0)) < 0) {
    show_message_fnc("snd_pcm_hw_params_set_rate_near() failed.\n");
    return 0;
  }
  *rate_p = rate;

  /* buffer & period */
  buffer = 500000;
  period = 500000 / 4;
  if ((err = snd_pcm_hw_params_set_buffer_time_near(alsa->fd, hwparams, &buffer, 0)) < 0) {
    show_message_fnc("snd_pcm_hw_params_set_buffer_near() failed.\n");
    return 0;
  }
  r = snd_pcm_hw_params_get_buffer_size(hwparams, &alsa->buffer_size);
  if ((err = snd_pcm_hw_params_set_period_time_near(alsa->fd, hwparams, &period, 0)) < 0) {
    show_message_fnc("snd_pcm_hw_params_set_period_near() failed.\n");
    return 0;
  }
  r = snd_pcm_hw_params_get_period_size(hwparams, &alsa->period_size, 0);

  if ((err = snd_pcm_hw_params(alsa->fd, hwparams)) < 0) {
    perror("snd_pcm_hw_params");
    err_message_fnc("snd_pcm_hw_params() failed.\n");
    snd_pcm_hw_params_dump(hwparams, alsa->log);
    return 0;
  }

  r = snd_pcm_hw_params_get_channels(hwparams, &ad->channels);
  r = snd_pcm_hw_params_get_rate(hwparams, &ad->speed, 0);
  r = r; // dummy

#ifdef DEBUG
  {
    snd_pcm_format_t form;

    debug_message_fnc("format ");
    r = snd_pcm_hw_params_get_format(hwparams, &form);
    switch (form) {
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

    debug_message("%d ch %d Hz buffer %ld period %ld OK\n", ad->channels, ad->speed, alsa->buffer_size, alsa->period_size);
  }
#endif

  /* sw_params */
  if ((err = snd_pcm_sw_params_current(alsa->fd, swparams)) < 0) {
    show_message_fnc("snd_pcm_sw_params_any() failed.\n");
    return 0;
  }
  if ((err = snd_pcm_sw_params(alsa->fd, swparams)) < 0) {
    show_message_fnc("snd_pcm_sw_params() failed.\n");
    return 0;
  }

  debug_message_fnc("1 sample -> %ld bytes\n", snd_pcm_samples_to_bytes(alsa->fd, 1));

  return 1;
}

static int
write_device(AudioDevice *ad, unsigned char *data, int size)
{
  ALSA_data *alsa = (ALSA_data *)ad->private_data;
  snd_pcm_sframes_t r;
  ssize_t unit = snd_pcm_samples_to_bytes(alsa->fd, 1) * ad->channels;
  snd_pcm_uframes_t count = size / unit;

  while (count > 0) {
    if ((r = snd_pcm_writei(alsa->fd, data, count)) == -EAGAIN) {
      //debug_message_fnc(" EAGAIN\n");
      snd_pcm_wait(alsa->fd, 1000);
    } else if (r > 0) {
      //debug_message_fnc(" wrote %d bytes\n", (int)(r * unit));
      ad->bytes_written += r * unit;
      count -= r;
      data += r * unit;
    } else if (r == -EPIPE) {
      debug_message_fnc("EPIPE: ");
      if (snd_pcm_state(alsa->fd) == SND_PCM_STATE_XRUN) {
	if ((r = snd_pcm_prepare(alsa->fd)) < 0) {
	  debug_message("failed\n");
	  warning_fnc("snd_pcm_prepare() failed.");
	} else {
	  debug_message("OK\n");
	}
      }
    } else {
      warning_fnc(" r = %d < 0...\n", (int)r);
    }
  }

  return 1;
}

static int
bytes_written(AudioDevice *ad)
{
  ALSA_data *alsa = (ALSA_data *)ad->private_data;
  snd_pcm_sframes_t delay;
  snd_pcm_state_t state;

  if (!alsa)
    return -1;

  state = snd_pcm_state(alsa->fd);
  if (state == SND_PCM_STATE_OPEN || state == SND_PCM_STATE_SETUP)
    return -1;

  if (snd_pcm_delay(alsa->fd, &delay) < 0)
    warning_fnc("snd_pcm_delay() failed.\n");

  return ad->bytes_written - snd_pcm_samples_to_bytes(alsa->fd, delay) * ad->channels;
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
  free(alsa);
  free(ad);

  return 1;
}
