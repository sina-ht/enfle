/*
 * esd.c -- EsounD Audio plugin
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Jun 24 05:44:43 2001.
 * $Id: esd.c,v 1.3 2001/06/24 15:41:50 sian Exp $
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
static int set_params(AudioDevice *, AudioFormat *, int *, int *);
static int write_device(AudioDevice *, unsigned char *, int);
static int sync_device(AudioDevice *);
static int close_device(AudioDevice *);

static AudioPlugin plugin = {
  type: ENFLE_PLUGIN_AUDIO,
  name: "EsounD",
  description: "EsounD Audio plugin version 0.1",
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

  if ((ad = calloc(1, sizeof(AudioDevice))) == NULL)
    return NULL;

  ad->c = c;

  return ad;
}

static int
set_params(AudioDevice *ad, AudioFormat *format_p, int *ch_p, int *rate_p)
{
  int bits, channels;
  AudioFormat format = *format_p;
  int ch = *ch_p;
  int rate = *rate_p;

  /* format part */
  switch (format) {
  case _AUDIO_FORMAT_U8:
    bits = ESD_BITS8;
    break;
  case _AUDIO_FORMAT_S8:
    bits = ESD_BITS8;
    break;
  case _AUDIO_FORMAT_U16_LE:
    bits = ESD_BITS16;
    break;
  case _AUDIO_FORMAT_U16_BE:
    bits = ESD_BITS16;
    break;
  case _AUDIO_FORMAT_S16_LE:
    bits = ESD_BITS16;
    break;
  case _AUDIO_FORMAT_S16_BE:
    bits = ESD_BITS16;
    break;
  default:
    show_message(__FUNCTION__ ": format %d is invalid or unsupported.\n", format);
    ad->format = _AUDIO_FORMAT_UNSET;
    *format_p = _AUDIO_FORMAT_UNSET;
    return 0;
  }

  channels = (ch == 1) ? ESD_MONO : ESD_STEREO;

  if ((ad->fd = esd_play_stream_fallback(bits | channels | ESD_STREAM | ESD_PLAY, rate, NULL, "enfle")) <= 0)
    return 0;

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
  fsync(ad->fd);

  return 1;
}

static int
close_device(AudioDevice *ad)
{
  close(ad->fd);
  free(ad);

  return 1;
}
