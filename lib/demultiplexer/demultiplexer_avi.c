/*
 * demultiplexer_avi.c -- AVI stream demultiplexer
 * (C)Copyright 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat May  4 23:26:32 2002.
 * $Id: demultiplexer_avi.c,v 1.12 2002/05/04 14:34:54 sian Exp $
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

#define REQUIRE_UNISTD_H
#define REQUIRE_STRING_H
#include "compat.h"
#define REQUIRE_FATAL
#include "common.h"

#ifndef USE_PTHREAD
#  error pthread is mandatory for demultiplexer_avi
#endif

#include "demultiplexer_avi.h"
#include "demultiplexer_avi_private.h"

DECLARE_DEMULTIPLEXER_METHODS;
PREPARE_DEMULTIPLEXER_TEMPLATE;

Demultiplexer *
demultiplexer_avi_create(void)
{
  Demultiplexer *demux;
  AVIInfo *info;

  if ((demux = _demultiplexer_create()) == NULL)
    return NULL;
  memcpy(demux, &template, sizeof(Demultiplexer));

  if ((info = calloc(1, sizeof(AVIInfo))) == NULL) {
    free(demux);
    return NULL;
  }

  demux->private_data = (void *)info;

  return demux;
}  

static int
input_func(void *arg, void *ptr, int size)
{
  Stream *st = (Stream *)arg;

  return stream_read(st, ptr, size);
}

static int
seek_func(void *arg, unsigned int pos, RIFF_SeekWhence whence)
{
  Stream *st = (Stream *)arg;

  return stream_seek(st, pos, whence);
}

static int
tell_func(void *arg)
{
  Stream *st = (Stream *)arg;

  return stream_tell(st);
}

#define FCC_vids 0x73646976
#define FCC_auds 0x73647561

#define _READ_CHUNK_HEADER(f, c) \
 for (;;) { \
  if (!riff_file_read_chunk_header((f), (c))) { \
    show_message_fnc("riff_file_read_chunk_header() failed: %s\n", riff_file_get_errmsg((f))); \
    goto error_free; \
  } \
  if (strcmp(riff_chunk_get_name((c)), "JUNK")) \
    break; \
  riff_file_skip_chunk_data((f), (c)); \
 }
#define _CHECK_CHUNK_NAME(c, n) \
 if (riff_chunk_is_list((c))) { \
  show_message_fnc("Chunk '%s' expected, but got list '%s'\n", n, riff_chunk_get_list_name((c))); \
  goto error_free; \
 } \
 if (strcmp(riff_chunk_get_name((c)), n) != 0) { \
  show_message("Chunk name is not '" n "' but '%s'.\n", riff_chunk_get_name(rc)); \
  goto error_free; \
 }
#define _CHECK_LIST_NAME(c, n) \
 if (!riff_chunk_is_list((c))) { \
  show_message_fnc("List '%s' expected, but got chunk '%s'\n", n, riff_chunk_get_name((c))); \
  goto error_free; \
 } \
 if (strcmp(riff_chunk_get_list_name((c)), n) != 0) { \
  show_message("List name is not '" n "' but '%s'.\n", riff_chunk_get_list_name(rc)); \
  goto error_free; \
 }

static int
examine(Demultiplexer *demux)
{
  AVIInfo *info = (AVIInfo *)demux->private_data;
  MainAVIHeader mah;
  AVIStreamHeader ash;
  BITMAPINFOHEADER bih;
  WAVEFORMATEX wfx;
  RIFF_Chunk *rc_top, *rc;
  unsigned int i;

  debug_message_fn("()\n");

  info->nastreams = 0;
  info->nvstreams = 0;

  if ((info->rf = riff_file_create()) == NULL)
    return 0;
  riff_file_set_func_input(info->rf, input_func);
  riff_file_set_func_seek(info->rf, seek_func);
  riff_file_set_func_tell(info->rf, tell_func);
  riff_file_set_func_arg(info->rf, (void *)info->st);
  if (!riff_file_open(info->rf)) {
    show_message_fnc("riff_file_open() failed: %s\n", riff_file_get_errmsg(info->rf));
    goto error_destroy;
  }
  rc_top = rc = riff_chunk_create();

  _READ_CHUNK_HEADER(info->rf, rc);
  _CHECK_LIST_NAME(rc, "hdrl");
  debug_message_fnc("Got list 'hdrl'\n");

  _READ_CHUNK_HEADER(info->rf, rc);
  _CHECK_CHUNK_NAME(rc, "avih");
  debug_message_fnc("Got chunk 'avih'\n");

  riff_file_read_data(info->rf, rc);
  /* XXX: little endian only. */
  memcpy(&mah, riff_chunk_get_data(rc), sizeof(MainAVIHeader));
  riff_chunk_destroy(rc);
  info->nframes = mah.dwTotalFrames;
  info->swidth  = mah.dwWidth;
  info->sheight = mah.dwHeight;
  info->rate = mah.dwRate;
  info->length = mah.dwLength;
  info->framerate = 1000000.0 / mah.dwMicroSecPerFrame;

  for (i = 0; i < mah.dwStreams; i++) {
    _READ_CHUNK_HEADER(info->rf, rc);
    _CHECK_LIST_NAME(rc, "strl");
    debug_message_fnc("Got list 'strl'\n");

    _READ_CHUNK_HEADER(info->rf, rc);
    _CHECK_CHUNK_NAME(rc, "strh");
    debug_message_fnc("Got chunk 'strh'\n");
    riff_file_read_data(info->rf, rc);
    /* XXX: Little endian only */
    memcpy(&ash, riff_chunk_get_data(rc), sizeof(AVIStreamHeader));
    riff_chunk_destroy(rc);
    if (ash.fccType == FCC_vids) {
      /* vids */
      info->vhandler = ash.fccHandler;
      info->num_of_frames = ash.dwLength;
    } else if (ash.fccType == FCC_auds) {
      /* auds */
      /* XXX: First stream only */
      if (info->nastreams == 0)
	info->ahandler = ash.fccHandler;
    } else {
      show_message_fnc("Unknown fccType %X\n", ash.fccType);
    }

    _READ_CHUNK_HEADER(info->rf, rc);
    _CHECK_CHUNK_NAME(rc, "strf");
    debug_message_fnc("Got chunk 'strf'\n");
    riff_file_read_data(info->rf, rc);
    /* XXX: Little endian only */
    if (ash.fccType == FCC_vids) {
      if (info->nvstreams == 0) {
	memcpy(&bih, riff_chunk_get_data(rc), sizeof(BITMAPINFOHEADER));
	info->width = bih.biWidth;
	info->height = bih.biHeight;
      }
      info->nvstreams++;
    } else if (ash.fccType == FCC_auds) {
      /* XXX: First stream only */
      if (info->nastreams == 0) {
	memcpy(&wfx, riff_chunk_get_data(rc), sizeof(WAVEFORMATEX));
	info->nchannels = wfx.nChannels;
	info->samples_per_sec = wfx.nSamplesPerSec;
	info->num_of_samples = wfx.cbSize;
      }
      info->nastreams++;
    }
    riff_chunk_destroy(rc);
  }

  for (;;) {
    _READ_CHUNK_HEADER(info->rf, rc);
    if (!riff_chunk_is_list(rc)) {
      debug_message_fnc("Skipped chunk '%s'.\n", riff_chunk_get_name(rc));
      riff_file_skip_chunk_data(info->rf, rc);
    } else if (strcmp(riff_chunk_get_list_name(rc), "movi") == 0)
      break;
    else {
      debug_message("List name is not 'movi' but '%s', skipped.\n", riff_chunk_get_list_name(rc));
      riff_file_skip_chunk_data(info->rf, rc);
    }
  }

  debug_message_fnc("Got list 'movi'\n");
  info->movi_start = stream_tell(info->st);

  /* XXX: should check idx1 */

  free(rc);
  return 1;

 error_free:
  free(rc);
 error_destroy:
  riff_file_destroy(info->rf);
  info->rf = NULL;
  return 0;
}

static void
avi_packet_destructor(void *d)
{
  AVIPacket *ap = (AVIPacket *)d;

  if (ap) {
    if (ap->data)
      free(ap->data);
    free(ap);
  }
}

static void *
demux_main(void *arg)
{
  Demultiplexer *demux = (Demultiplexer *)arg;
  AVIInfo *info = (AVIInfo *)demux->private_data;
  AVIPacket *ap;
  RIFF_Chunk *rc;

  if (!info->rf) {
    if (!examine(demux))
      pthread_exit((void *)0);
  }

  if ((rc = riff_chunk_create()) == NULL)
    pthread_exit((void *)0);

  demux->running = 1;
  while (demux->running) {
    char *p;

    if (!riff_file_read_chunk_header(info->rf, rc))
      break;
    if (riff_chunk_is_list(rc)) {
      if (strcmp(riff_chunk_get_list_name(rc), "rec ") != 0) {
	debug_message_fnc("Got list '%s', skipped.\n", riff_chunk_get_list_name(rc));
	riff_file_skip_chunk_data(info->rf, rc);
	continue;
      }
      if (!riff_file_read_chunk_header(info->rf, rc))
	break;
      if (riff_chunk_is_list(rc)) {
	show_message_fnc("List '%s' within 'rec ' list... skipped.\n", riff_chunk_get_list_name(rc));
	riff_file_skip_chunk_data(info->rf, rc);
	continue;
      }
    }
    p = riff_chunk_get_name(rc);
    if (p[2] == 'd' && (p[3] == 'c' || p[3] == 'b')) {
      /* video data */
      if (!riff_file_read_data(info->rf, rc))
	break;
      if ((ap = malloc(sizeof(AVIPacket))) == NULL)
	fatal(2, "%s: No enough memory.\n", __FUNCTION__);
      ap->size = riff_chunk_get_size(rc);
      ap->data = riff_chunk_get_data(rc);
      fifo_put(info->vstream, ap, avi_packet_destructor);
    } else if (p[2] == 'w' && p[3] == 'b') {
      int nstream;

      /* audio data */
      nstream = (p[0] - '0') * 10 + (p[1] - '0');
      if (nstream == info->nastream) {
	if (!riff_file_read_data(info->rf, rc))
	  break;
	if (riff_chunk_get_size(rc) > 0) {
	  if ((ap = malloc(sizeof(AVIPacket))) == NULL)
	    fatal(2, "%s: No enough memory.\n", __FUNCTION__);
	  ap->size = riff_chunk_get_size(rc);
	  ap->data = riff_chunk_get_data(rc);
	  fifo_put(info->astream, ap, avi_packet_destructor);
	}
      } else {
	riff_file_skip_chunk_data(info->rf, rc);
      }
    } else {
      show_message_fnc("Got unknown chunk '%s', skipped\n", riff_chunk_get_name(rc));
      riff_file_skip_chunk_data(info->rf, rc);
    }
  }

  free(rc);

  if (riff_file_get_err(info->rf) != _RIFF_ERR_SUCCESS) {
    show_message_fnc("Abort: %s.\n", riff_file_get_errmsg(info->rf));
    pthread_exit((void *)0);
  }

  debug_message_fnc("EOF\n");

  demultiplexer_set_eof(demux, 1);
  demux->running = 0;

  pthread_exit((void *)1);
}

static int
start(Demultiplexer *demux)
{
  if (demux->running)
    return 0;

  debug_message_fn(" demultiplexer_avi\n");

  pthread_create(&demux->thread, NULL, demux_main, demux);

  return 1;
}

static int
stop(Demultiplexer *demux)
{
  void *ret;

  if (!demux->running)
    return 0;

  debug_message_fn(" demultiplexer_avi\n");

  demux->running = 0;
  //pthread_cancel(demux->thread);
  pthread_join(demux->thread, &ret);

  debug_message_fnc("joined\n");

  return 1;
}

static int
demux_rewind(Demultiplexer *demux)
{
  AVIInfo *info = (AVIInfo *)demux->private_data;

  if (demux->running)
    return 0;
  return stream_seek(info->st, info->movi_start, _SEEK_SET);
}

static void
destroy(Demultiplexer *demux)
{
  AVIInfo *info = (AVIInfo *)demux->private_data;

  stop(demux);
  if (info->rf)
    riff_file_destroy(info->rf);
  if (demux->private_data)
    free(demux->private_data);
  _demultiplexer_destroy(demux);
}
