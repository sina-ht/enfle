/*
 * oss.c -- OSS Audio plugin
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Dec 26 13:41:34 2000.
 * $Id: oss.c,v 1.4 2000/12/27 19:04:40 sian Exp $
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

#include "audio-plugin.h"

static AudioDevice *open_device(void *, Config *);
static int set_params(AudioDevice *, AudioFormat *, int *, int *);
static int write_device(AudioDevice *, unsigned char *, int);
static int sync_device(AudioDevice *);
static int close_device(AudioDevice *);

static AudioPlugin plugin = {
  type: ENFLE_PLUGIN_AUDIO,
  name: "OSS",
  description: "OSS Audio plugin version 0.1.2",
  author: "Hiroshi Takekawa",

  open_device: open_device,
  set_params: set_params,
  write_device: write_device,
  sync_device: sync_device,
  close_device: close_device
};

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

/* for internal use */

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

  if ((ad->fd = open(device, O_WRONLY, 0)) == -1) {
    show_message(__FUNCTION__ ": in opening device %s\n", device);
    perror("OSS");
    free(ad);
    return NULL;
  }

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
    show_message(__FUNCTION__ ": format %d is invalid.\n", format);
    ad->format = _AUDIO_FORMAT_UNSET;
    *format_p = _AUDIO_FORMAT_UNSET;
    return 0;
  }

  if (ioctl(ad->fd, SNDCTL_DSP_SETFMT, &f) == -1) {
    show_message(__FUNCTION__ ": in setting format as %d(to ioctl %d).\n", format, f);
    perror("OSS");
    ad->format = _AUDIO_FORMAT_UNSET;
    return _AUDIO_FORMAT_UNSET;
  }

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
    break;
  case AFMT_S8:
    ad->format = _AUDIO_FORMAT_S8;
    break;
  case AFMT_U16_LE:
    ad->format = _AUDIO_FORMAT_U16_LE;
    break;
  case AFMT_U16_BE:
    ad->format = _AUDIO_FORMAT_U16_BE;
    break;
  case AFMT_S16_LE:
    ad->format = _AUDIO_FORMAT_S16_LE;
    break;
  case AFMT_S16_BE:
    ad->format = _AUDIO_FORMAT_S16_BE;
    break;
#if 0
  case AFMT_S32_LE:
    ad->format = _AUDIO_FORMAT_S32_LE;
    break;
  case AFMT_S32_BE:
    ad->format = _AUDIO_FORMAT_S32_BE;
    break;
#endif
  case AFMT_MPEG:
    ad->format = _AUDIO_FORMAT_UNSET;
    show_message(__FUNCTION__ ": unsupported AFMT_MPEG\n");
    break;
  }

  *format_p = ad->format;

  /* channel part */
  c = ch;
  if (ioctl(ad->fd, SNDCTL_DSP_CHANNELS, &c) == -1) {
    show_message(__FUNCTION__ ": in setting channels to %d.\n", ch);
    perror("OSS");
    *ch_p = 0;
    return 0;
  }

  debug_message(__FUNCTION__ ": set to %d\n", c);

  *ch_p = c;

  /* rate part */
  r = rate;
  if (ioctl(ad->fd, SNDCTL_DSP_SPEED, &r) == -1) {
    show_message(__FUNCTION__ ": in setting speed to %d.\n", rate);
    perror("OSS");
    *rate_p = 0;
    return 0;
  }

  debug_message(__FUNCTION__ ": set to %d\n", r);

  *rate_p = r;

  return 1;
}

static int
write_device(AudioDevice *ad, unsigned char *data, int size)
{
  return write(ad->fd, data, size);
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

  return 1;
}
