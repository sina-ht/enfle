/*
 * demultiplexer_avi.c -- AVI stream demultiplexer
 * (C)Copyright 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Dec 26 12:06:37 2003.
 * $Id: demultiplexer_avi.c,v 1.22 2003/12/27 14:25:25 sian Exp $
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

  switch (whence) {
  case _SEEK_SET:
    return stream_seek(st, pos, _SET);
  case _SEEK_CUR:
    return stream_seek(st, pos, _CUR);
  case _SEEK_END:
    return stream_seek(st, pos, _END);
  }
  return 0;
}

static int
tell_func(void *arg)
{
  Stream *st = (Stream *)arg;

  return stream_tell(st);
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

  //debug_message_fn("()\n");

  info->nastreams = 0;
  info->nvstreams = 0;

  if ((info->rf = riff_file_create()) == NULL)
    return 0;
  riff_file_set_func_input(info->rf, input_func);
  riff_file_set_func_seek(info->rf, seek_func);
  riff_file_set_func_tell(info->rf, tell_func);
  riff_file_set_func_arg(info->rf, (void *)info->st);
  if (!riff_file_open(info->rf)) {
    debug_message_fnc("riff_file_open() failed: %s\n", riff_file_get_errmsg(info->rf));
    goto error_destroy;
  }
  rc_top = rc = riff_chunk_create();

  /*
RIFF( 'AVI' LIST ( 'hdrl'
                   'avih' ( Main AVI Header )
                   LIST ( 'strl'
                          'strh' ( Stream 1 Header )
                          'strf' ( Stream 1 Format )
                         ['strd' ( Stream 1 optional codec data ) ]
                        )
                   LIST ( 'strl'
                          'strh' ( Stream 2 Header )
                          'strf' ( Stream 2 Format )
                        [ 'strd' ( Stream 2 optional codec data )]
                        )
                   ...
                )
        LIST ( 'movi'
                { 
                        '##dc' ( compressed DIB )
                        | 
                        LIST ( 'rec '
                                '##dc' ( compressed DIB )
                                '##db' ( uncompressed DIB )
                                '##wb' ( audio data ) 
                                '##pc' ( palette change )
                                ...
                        )
                }
                ...
        )
        [ 'idx1' ( AVI Index ) ]
)
  */

  for (;;) {
    if (!riff_file_read_chunk_header(info->rf, rc)) {
      if (riff_file_is_eof(info->rf)) {
	debug_message_fnc("EOF\n");
	break;
      }
      show_message_fnc("riff_file_read_chunk_header() failed: %s\n", riff_file_get_errmsg(info->rf));
      goto error_free;
    }
    switch (rc->fourcc) {
    case FCC_JUNK:
      debug_message_fnc("Got chunk 'JUNK', skip.\n");
      riff_file_skip_chunk_data(info->rf, rc);
      break;
    case FCC_idx1:
      debug_message_fnc("Got chunk 'idx1', skip (so far).\n");
      riff_file_skip_chunk_data(info->rf, rc);
      break;
    case FCC_indx:
      debug_message_fnc("Got chunk 'indx', skip (so far).\n");
      riff_file_skip_chunk_data(info->rf, rc);
      break;
    case FCC_LIST:
      switch (rc->list_fourcc) {
      case FCC_movi:
	debug_message_fnc("Got list 'movi'\n");
	info->movi_start = stream_tell(info->st);
	riff_file_skip_chunk_data(info->rf, rc);
	break;
      case FCC_hdrl:
	debug_message_fnc("Got list 'hdrl'\n");
	if (!riff_file_read_chunk_header(info->rf, rc)) {
	  show_message_fnc("riff_file_read_chunk_header() failed: %s\n", riff_file_get_errmsg(info->rf));
	  goto error_free;
	}
	if (rc->fourcc != FCC_avih) {
	  debug_message_fnc("'avih' chunk expected, but got chunk '%s'\n", rc->name);
	  goto error_free;
	}
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

	debug_message_fnc("Streams: %d\n", mah.dwStreams);
	for (i = 0; i < mah.dwStreams; i++) {
	  for (;;) {
	    if (!riff_file_read_chunk_header(info->rf, rc)) {
	      show_message_fnc("riff_file_read_chunk_header() failed: %s\n", riff_file_get_errmsg(info->rf));
	      goto error_free;
	    }
	    if (!riff_chunk_is_list(rc)) {
	      debug_message_fnc("list expected, but got chunk '%s'\n", rc->name);
	      riff_file_skip_chunk_data(info->rf, rc);
	      continue;
	    }
	    if (rc->list_fourcc == FCC_strl)
	      break;
	    debug_message_fnc("'strl' list expected, but got list '%s'\n", rc->list_name);
	    goto error_free;
	  }
	  debug_message_fnc("Got list 'strl'\n");
	  if (!riff_file_read_chunk_header(info->rf, rc)) {
	    show_message_fnc("riff_file_read_chunk_header() failed: %s\n", riff_file_get_errmsg(info->rf));
	    goto error_free;
	  }
	  if (rc->fourcc != FCC_strh) {
	    debug_message_fnc("strh chunk expected, but got chunk '%s'\n", rc->name);
	    goto error_free;
	  }
	  debug_message_fnc("Got chunk 'strh'\n");
	  riff_file_read_data(info->rf, rc);
	  /* XXX: Little endian only */
	  memcpy(&ash, riff_chunk_get_data(rc), sizeof(AVIStreamHeader));
	  riff_chunk_destroy(rc);
	  if (ash.fccType == FCC_vids) {
	    info->vhandler = ash.fccHandler;
	    if (info->vhandler == 0)
	      debug_message_fnc("strh fccHandler == 0\n");
	    info->num_of_frames = ash.dwLength;
	  } else if (ash.fccType == FCC_auds) {
	    /* XXX: First stream only */
	    if (info->nastreams == 0)
	      info->ahandler = ash.fccHandler;
	  } else {
	    show_message_fnc("Unknown fccType %X\n", ash.fccType);
	  }

	  if (!riff_file_read_chunk_header(info->rf, rc)) {
	    show_message_fnc("riff_file_read_chunk_header() failed: %s\n", riff_file_get_errmsg(info->rf));
	    goto error_free;
	  }
	  if (rc->fourcc != FCC_strf) {
	    debug_message_fnc("'strf' chunk expected, but got chunk '%s'\n", rc->name);
	    goto error_free;
	  }
	  debug_message_fnc("Got chunk 'strf'\n");
	  riff_file_read_data(info->rf, rc);
	  /* XXX: Little endian only */
	  if (ash.fccType == FCC_vids) {
	    if (info->nvstreams == 0) {
	      memcpy(&bih, riff_chunk_get_data(rc), sizeof(BITMAPINFOHEADER));
	      if (info->vhandler == 0) {
		debug_message_fnc("using bih.biCompression(%x) for vhandler\n", bih.biCompression);
		info->vhandler = bih.biCompression;
	      }
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
	break;
      default:
	debug_message_fnc("Got list '%s'... not handled\n", riff_chunk_get_list_name(rc));
	riff_file_skip_chunk_data(info->rf, rc);
	break;
      }
    }
  }

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
  RIFF_Chunk *rc;

  if (!info->rf) {
    if (!examine(demux))
      pthread_exit((void *)0);
  }

  if ((rc = riff_chunk_create()) == NULL)
    pthread_exit((void *)0);

  demux->running = 1;
  while (demux->running) {
    AVIPacket *ap;
    char *p;
    int nstream;

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
    nstream = (p[0] - '0') * 10 + (p[1] - '0');
    if (p[2] == 'd' && (p[3] == 'c' || p[3] == 'b')) {
      /* video data */
      if (nstream == info->nvstream) {
	if (!riff_file_read_data(info->rf, rc))
	  break;
	if (riff_chunk_get_size(rc) > 0) {
	  if ((ap = malloc(sizeof(AVIPacket))) == NULL)
	    fatal("%s: No enough memory.\n", __FUNCTION__);
	  ap->size = riff_chunk_get_size(rc);
	  ap->data = riff_chunk_get_data(rc);
	  fifo_put(info->vstream, ap, avi_packet_destructor);
	}
      } else {
	riff_file_skip_chunk_data(info->rf, rc);
      }
    } else if (p[2] == 'w' && p[3] == 'b') {
      /* audio data */
      if (nstream == info->nastream) {
	if (!riff_file_read_data(info->rf, rc))
	  break;
	if (riff_chunk_get_size(rc) > 0) {
	  if ((ap = malloc(sizeof(AVIPacket))) == NULL)
	    fatal("%s: No enough memory.\n", __FUNCTION__);
	  ap->size = riff_chunk_get_size(rc);
	  ap->data = riff_chunk_get_data(rc);
	  fifo_put(info->astream, ap, avi_packet_destructor);
	}
      } else {
	riff_file_skip_chunk_data(info->rf, rc);
      }
    } else {
      show_message_fnc("Got unknown chunk '%s' at %d, skipped\n", riff_chunk_get_name(rc), riff_chunk_get_pos(rc));
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

  return (void *)1;
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
  AVIInfo *info = (AVIInfo *)demux->private_data;
  void *ret;

  if (!demux->running)
    return 0;

  debug_message_fn(" demultiplexer_avi\n");

  demux->running = 0;
  if (info->vstream)
    fifo_emptify(info->vstream);
  if (info->astream)
    fifo_emptify(info->astream);
  pthread_join(demux->thread, &ret);

  debug_message_fn(" demultiplexer_avi OK\n");

  return 1;
}

static int
demux_rewind(Demultiplexer *demux)
{
  AVIInfo *info = (AVIInfo *)demux->private_data;

  if (demux->running)
    return 0;
  return stream_seek(info->st, info->movi_start, _SET);
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
