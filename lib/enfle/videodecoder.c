/*
 * videodecoder.c -- Video decoder
 * (C)Copyright 2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Wed Jan 28 02:26:16 2004.
 * $Id: videodecoder.c,v 1.1 2004/01/30 12:39:04 sian Exp $
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

#include <stdio.h>
#include <stdlib.h>

#define REQUIRE_STRING_H
#define REQUIRE_UNISTD_H
#include "compat.h"
#include "common.h"

#ifndef USE_PTHREAD
#  error pthread is mandatory for videodecoder
#endif

#include <pthread.h>

#include "videodecoder-plugin.h"

VideoDecoder *
_videodecoder_init(void)
{
  VideoDecoder *vdec;

  if ((vdec = calloc(1, sizeof(*vdec))) == NULL)
    return NULL;
  pthread_mutex_init(&vdec->update_mutex, NULL);
  pthread_cond_init(&vdec->update_cond, NULL);

  return vdec;
}

void
_videodecoder_destroy(VideoDecoder *vdec)
{
  if (vdec) {
    pthread_mutex_destroy(&vdec->update_mutex);
    pthread_cond_destroy(&vdec->update_cond);
    free(vdec);
  }
}

VideoDecoder *
videodecoder_create(EnflePlugins *eps, const char *pluginname)
{
  Plugin *p;
  VideoDecoderPlugin *vdp;

  if ((p = pluginlist_get(eps->pls[ENFLE_PLUGIN_VIDEODECODER], (char *)pluginname)) == NULL)
    return NULL;
  vdp = plugin_get(p);

  return vdp->init();
}
