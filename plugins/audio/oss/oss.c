/*
 * oss.c -- OSS Audio plugin
 * (C)Copyright 2000, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Nov 30 14:50:42 2003.
 * $Id: oss.c,v 1.13 2003/11/30 05:51:38 sian Exp $
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

#define REQUIRE_UNISTD_H
#define REQUIRE_STRING_H
#include "compat.h"

#ifdef HAVE_SYS_SOUNDCARD_H
#  include <sys/soundcard.h>
#else
#  ifdef HAVE_SOUNDCARD_H
#    include <soundcard.h>
#  endif
#  ifdef HAVE_MACHINE_SOUNDCARD_H
#    include <machine/soundcard.h>
#  endif
#endif
#ifndef SNDCTL_DSP_CHANNELS
#  define SNDCTL_DSP_CHANNELS SNDCTL_DSP_STEREO
#endif

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
  name: "OSS",
  description: "OSS Audio plugin version 0.1.3",
  author: "Hiroshi Takekawa",

  open_device: open_device,
  set_params: set_params,
  write_device: write_device,
  bytes_written: bytes_written,
  sync_device: sync_device,
  close_device: close_device
};

ENFLE_PLUGIN_ENTRY(audio_oss)
{
  AudioPlugin *ap;

  if ((ap = (AudioPlugin *)calloc(1, sizeof(AudioPlugin))) == NULL)
    return NULL;
  memcpy(ap, &plugin, sizeof(AudioPlugin));

  return (void *)ap;
}

ENFLE_PLUGIN_EXIT(audio_oss, p)
{
  free(p);
}

/* audio plugin methods */

static AudioDevice *
open_device(void *data, Config *c)
{
  AudioDevice *ad;
  char *device = data;

  if ((ad = calloc(1, sizeof(AudioDevice))) == NULL)
    return NULL;

  ad->c = c;
  
  if ((device == NULL) && ((device = config_get(c, "/enfle/plugins/audio/oss/device_path")) == NULL))
    /* last resort */
    device = (char *)"/dev/dsp";

  if ((ad->fd = open(device, O_WRONLY | O_NONBLOCK, 0)) == -1) {
    show_message_fnc("in opening device \"%s\"\n", device);
    perror("OSS");
    free(ad);
    return NULL;
  }

  ioctl(ad->fd, SNDCTL_DSP_RESET);
  ad->opened = 1;

  return ad;
}

static int
set_params(AudioDevice *ad, AudioFormat *format_p, int *ch_p, int *rate_p)
{
  int c, f, r;
  AudioFormat format = *format_p;
  int ch = *ch_p;
  int rate = *rate_p;

  /* format part */
  switch (format) {
  case _AUDIO_FORMAT_MU_LAW:
    f = AFMT_MU_LAW;
    break;
  case _AUDIO_FORMAT_A_LAW:
    f = AFMT_A_LAW;
    break;
  case _AUDIO_FORMAT_ADPCM:
    f = AFMT_IMA_ADPCM;
    break;
  case _AUDIO_FORMAT_U8:
    f = AFMT_U8;
    break;
  case _AUDIO_FORMAT_S8:
    f = AFMT_S8;
    break;
  case _AUDIO_FORMAT_U16_LE:
    f = AFMT_U16_LE;
    break;
  case _AUDIO_FORMAT_U16_BE:
    f = AFMT_U16_BE;
    break;
  case _AUDIO_FORMAT_S16_LE:
    f = AFMT_S16_LE;
    break;
  case _AUDIO_FORMAT_S16_BE:
    f = AFMT_S16_BE;
    break;
#if 0
  case _AUDIO_FORMAT_S32_LE:
    f = AFMT_S32_LE;
    break;
  case _AUDIO_FORMAT_S32_BE:
    f = AFMT_S32_BE;
    break;
#endif
  default:
    show_message_fnc("format %d is invalid.\n", format);
    ad->format = _AUDIO_FORMAT_UNSET;
    *format_p = _AUDIO_FORMAT_UNSET;
    return 0;
  }

  if (ioctl(ad->fd, SNDCTL_DSP_SETFMT, &f) == -1) {
    show_message_fnc("in setting format as %d(to ioctl %d).\n", format, f);
    perror("OSS");
    ad->format = _AUDIO_FORMAT_UNSET;
    return 0;
  }

  ad->bytes_per_sample = 2;
  switch (f) {
  case AFMT_MU_LAW:
    ad->format = _AUDIO_FORMAT_MU_LAW;
    break;
  case AFMT_A_LAW:
    ad->format = _AUDIO_FORMAT_A_LAW;
    break;
  case AFMT_IMA_ADPCM:
    ad->format = _AUDIO_FORMAT_ADPCM;
    break;
  case AFMT_U8:
    ad->format = _AUDIO_FORMAT_U8;
    ad->bytes_per_sample = 1;
    break;
  case AFMT_S8:
    ad->format = _AUDIO_FORMAT_S8;
    ad->bytes_per_sample = 1;
    break;
  case AFMT_U16_LE:
    ad->format = _AUDIO_FORMAT_U16_LE;
    ad->bytes_per_sample = 2;
    break;
  case AFMT_U16_BE:
    ad->format = _AUDIO_FORMAT_U16_BE;
    ad->bytes_per_sample = 2;
    break;
  case AFMT_S16_LE:
    ad->format = _AUDIO_FORMAT_S16_LE;
    ad->bytes_per_sample = 2;
    break;
  case AFMT_S16_BE:
    ad->format = _AUDIO_FORMAT_S16_BE;
    ad->bytes_per_sample = 2;
    break;
#if 0
  case AFMT_S32_LE:
    ad->format = _AUDIO_FORMAT_S32_LE;
    ad->bytes_per_sample = 4;
    break;
  case AFMT_S32_BE:
    ad->format = _AUDIO_FORMAT_S32_BE;
    ad->bytes_per_sample = 4;
    break;
#endif
  case AFMT_MPEG:
    ad->format = _AUDIO_FORMAT_UNSET;
    show_message_fnc("unsupported AFMT_MPEG\n");
    break;
  }

  *format_p = ad->format;

  /* channel part */
  c = ch;
  if (ioctl(ad->fd, SNDCTL_DSP_CHANNELS, &c) == -1) {
    show_message_fnc("in setting channels to %d.\n", ch);
    perror("OSS");
    *ch_p = 0;
    return 0;
  }

  debug_message_fnc("set to %d ch\n", c);

  *ch_p = c;

  /* rate part */
  r = rate;
  if (ioctl(ad->fd, SNDCTL_DSP_SPEED, &r) == -1) {
    show_message_fnc("in setting speed to %d.\n", rate);
    perror("OSS");
    *rate_p = 0;
    return 0;
  }

  debug_message_fnc("set to %d Hz\n", r);

  *rate_p = r;

  ad->channels = ch;
  ad->speed = rate;

  return 1;
}

static int
write_device(AudioDevice *ad, unsigned char *data, int size)
{
  return write(ad->fd, data, size);
}

static int
bytes_written(AudioDevice *ad)
{
  count_info info;

  if (!ioctl(ad->fd, SNDCTL_DSP_GETOPTR, &info))
    return info.bytes;
  return -1;
}

static int
sync_device(AudioDevice *ad)
{
  ioctl(ad->fd, SNDCTL_DSP_SYNC, 0);

  return 1;
}

static int
close_device(AudioDevice *ad)
{
  close(ad->fd);
  free(ad);
  ad->opened = 0;

  return 1;
}
