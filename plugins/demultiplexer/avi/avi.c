/*
 * avi.c -- AVI stream demultiplexer plugin
 * (C)Copyright 2001-2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Apr  6 00:31:20 2004.
 * $Id: avi.c,v 1.6 2004/04/05 15:51:22 sian Exp $
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

#include "enfle/demultiplexer-plugin.h"
#include "enfle/demultiplexer_types.h"
#include "demultiplexer_avi.h"
#include "demultiplexer_avi_private.h"

DECLARE_DEMULTIPLEXER_PLUGIN_METHODS;
DECLARE_DEMULTIPLEXER_METHODS;
PREPARE_DEMULTIPLEXER_TEMPLATE;

static DemultiplexerPlugin plugin = {
  .type = ENFLE_PLUGIN_DEMULTIPLEXER,
  .name = "AVI",
  .description = "AVI Demultiplexer plugin version 0.1",
  .author = "Hiroshi Takekawa",

  .identify = identify,
  .examine = examine
};

ENFLE_PLUGIN_ENTRY(demultiplexer_avi)
{
  DemultiplexerPlugin *dp;

  if ((dp = (DemultiplexerPlugin *)calloc(1, sizeof(DemultiplexerPlugin))) == NULL)
    return NULL;
  memcpy(dp, &plugin, sizeof(DemultiplexerPlugin));

  return (void *)dp;
}

ENFLE_PLUGIN_EXIT(demultiplexer_avi, p)
{
  free(p);
}

/* demultiplexer plugin methods */

static Demultiplexer *
create(void)
{
  Demultiplexer *demux;
  AVIInfo *info;

  if ((demux = _demultiplexer_create()) == NULL)
    return NULL;
  memcpy(demux, &template, sizeof(Demultiplexer));

  if ((info = calloc(1, sizeof(AVIInfo))) == NULL) {
    _demultiplexer_destroy(demux);
    return NULL;
  }

  demux->private_data = (void *)info;

  return demux;
}  


static void
destroy(Demultiplexer *demux)
{
  AVIInfo *info = (AVIInfo *)demux->private_data;

  stop(demux);
  if (info) {
    if (info->rf)
      riff_file_destroy(info->rf);
    if (info->video_extradata)
      free(info->video_extradata);
    if (info->audio_extradata)
      free(info->audio_extradata);
    if (info->idx_offset)
      free(info->idx_offset);
    if (info->idx_length)
      free(info->idx_length);
    free(info);
  }
  _demultiplexer_destroy(demux);
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

static DemultiplexerStatus __examine(Demultiplexer *, int);

DEFINE_DEMULTIPLEXER_PLUGIN_IDENTIFY(st, c, priv)
{
  Demultiplexer *demux = create();
  DemultiplexerStatus ds;

  demux->st = st;
  ds = __examine(demux, 1);

  return ds;
}

DEFINE_DEMULTIPLEXER_PLUGIN_EXAMINE(m, st, c, priv)
{
  Demultiplexer *demux = create();
  DemultiplexerStatus ds;
  AVIInfo *info = (AVIInfo *)demux->private_data;

  demux->st = st;
  if ((ds = __examine(demux, 0)) != DEMULTIPLEX_OK) {
    destroy(demux);
    return NULL;
  }

  m->width = info->width;
  m->height = info->height;
  m->framerate = info->framerate;
  m->channels = info->nchannels;
  m->samplerate = info->samples_per_sec;
  m->num_of_frames = info->num_of_frames;
  m->block_align = info->block_align;
  m->bitrate = info->avg_bytes_per_sec * 8;
  m->v_fourcc = info->vhandler;
  m->a_fourcc = info->ahandler;
  m->video_extradata = info->video_extradata;
  m->video_extradata_size = info->video_extradata_size;
  m->audio_extradata = info->audio_extradata;
  m->audio_extradata_size = info->audio_extradata_size;

  return demux;
}

static DemultiplexerStatus
__examine(Demultiplexer *demux, int identify_only)
{
  static unsigned char form_avi[] = { 'A', 'V', 'I', ' ' };
  AVIInfo *info = (AVIInfo *)demux->private_data;
  DemultiplexerStatus ds;
  MainAVIHeader mah;
  AVIStreamHeader ash;
  BITMAPINFOHEADER bih;
  WAVEFORMATEX wfx;
  RIFF_Chunk *rc;
  unsigned int i;

  //debug_message_fn("()\n");

  demux->vstreams[0] = 0;
  demux->astreams[0] = 1;

  if ((info->rf = riff_file_create()) == NULL)
    return DEMULTIPLEX_ERROR;
  riff_file_set_func_input(info->rf, input_func);
  riff_file_set_func_seek(info->rf, seek_func);
  riff_file_set_func_tell(info->rf, tell_func);
  riff_file_set_func_arg(info->rf, (void *)demux->st);
  if (!riff_file_open(info->rf)) {
    //debug_message_fnc("riff_file_open() failed: %s\n", riff_file_get_errmsg(info->rf));
    ds = DEMULTIPLEX_ERROR;
    goto error_destroy;
  }
  if (memcmp(riff_file_get_form(info->rf), form_avi, 4) != 0) {
    ds = DEMULTIPLEX_NOT;
    goto error_destroy;
  }
  if (identify_only) {
    riff_file_destroy(info->rf);
    info->rf = NULL;
    return DEMULTIPLEX_OK;
  }

  rc = riff_chunk_create();

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
      if (riff_file_get_err(info->rf) == _RIFF_ERR_PREMATURE_CHUNK) {
	warning_fnc("Premature chunk detected.  Could go wrong.\n");
	break;
      } else {
	show_message_fnc("riff_file_read_chunk_header() failed: %s\n", riff_file_get_errmsg(info->rf));
	ds = DEMULTIPLEX_ERROR;
	goto error_free;
      }
    }
    switch (rc->fourcc) {
    case FCC_JUNK:
      debug_message_fnc("Got chunk 'JUNK', skip.\n");
      riff_file_skip_chunk_data(info->rf, rc);
      break;
    case FCC_idx1:
      {
	int nidx;

	riff_file_read_data(info->rf, rc);
	nidx = riff_chunk_get_size(rc) / sizeof(AVIINDEXENTRY);
	debug_message_fnc("Got chunk 'idx1': has %d indexes\n", nidx);
#if 0
	info->idx_offset = calloc(nidx, sizeof(int));
	info->idx_length = calloc(nidx, sizeof(int));
	{
	  AVIINDEXENTRY aie;
	  int i;
	  for (i = 0; i < nidx; i++) {
	    memcpy(&aie, riff_chunk_get_data(rc) + i * sizeof(AVIINDEXENTRY), sizeof(AVIINDEXENTRY));
	    info->idx_offset[i] = aie.dwChunkOffset;
	    info->idx_length[i] = aie.dwChunkLength;
	  }
	}
#endif
      }
      break;
    case FCC_indx:
      debug_message_fnc("Got chunk 'indx', skip (so far).\n");
      riff_file_skip_chunk_data(info->rf, rc);
      break;
    case FCC_LIST:
      switch (rc->list_fourcc) {
      case FCC_movi:
	debug_message_fnc("Got list 'movi'\n");
	info->movi_start = stream_tell(demux->st);
	riff_file_skip_chunk_data(info->rf, rc);
	break;
      case FCC_hdrl:
	debug_message_fnc("Got list 'hdrl'\n");
	if (!riff_file_read_chunk_header(info->rf, rc)) {
	  show_message_fnc("riff_file_read_chunk_header() failed: %s\n", riff_file_get_errmsg(info->rf));
	  ds = DEMULTIPLEX_ERROR;
	  goto error_free;
	}
	if (rc->fourcc != FCC_avih) {
	  debug_message_fnc("'avih' chunk expected, but got chunk '%s'\n", rc->name);
	  ds = DEMULTIPLEX_NOT;
	  goto error_free;
	}
	debug_message_fnc("Got chunk 'avih'\n");
	riff_file_read_data(info->rf, rc);
	/* XXX: little endian only. */
	memcpy(&mah, riff_chunk_get_data(rc), sizeof(MainAVIHeader));
	riff_chunk_free_data(rc);
	info->nframes = mah.dwTotalFrames;
	info->swidth  = mah.dwWidth;
	info->sheight = mah.dwHeight;
	info->rate = mah.dwRate;
	info->length = mah.dwLength;
	info->framerate = 1000000.0 / mah.dwMicroSecPerFrame;
#if defined(DEBUG)
	debug_message_fnc("AVI Flags: ");
	if (mah.dwFlags & AVIF_HASINDEX)
	  debug_message("HASINDEX ");
	if (mah.dwFlags & AVIF_MUSTUSEINDEX)
	  debug_message("MUSTUSEINDEX ");
	if (mah.dwFlags & AVIF_ISINTERLEAVED)
	  debug_message("ISINTERLEAVED ");
	debug_message("\n");
#endif

	debug_message_fnc("Streams: %d\n", mah.dwStreams);
	for (i = 0; i < mah.dwStreams; i++) {
	  for (;;) {
	    if (!riff_file_read_chunk_header(info->rf, rc)) {
	      show_message_fnc("riff_file_read_chunk_header() failed: %s\n", riff_file_get_errmsg(info->rf));
	      ds = DEMULTIPLEX_ERROR;
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
	    ds = DEMULTIPLEX_NOT;
	    goto error_free;
	  }
	  debug_message_fnc("Got list 'strl'\n");
	  if (!riff_file_read_chunk_header(info->rf, rc)) {
	    show_message_fnc("riff_file_read_chunk_header() failed: %s\n", riff_file_get_errmsg(info->rf));
	    ds = DEMULTIPLEX_ERROR;
	    goto error_free;
	  }
	  if (rc->fourcc != FCC_strh) {
	    debug_message_fnc("strh chunk expected, but got chunk '%s'\n", rc->name);
	    ds = DEMULTIPLEX_NOT;
	    goto error_free;
	  }
	  debug_message_fnc("Got chunk 'strh'\n");
	  riff_file_read_data(info->rf, rc);
	  /* XXX: Little endian only */
	  memcpy(&ash, riff_chunk_get_data(rc), sizeof(AVIStreamHeader));
	  riff_chunk_free_data(rc);
	  if (ash.fccType == FCC_vids) {
	    info->vhandler = ash.fccHandler;
	    if (info->vhandler == 0)
	      debug_message_fnc("strh fccHandler == 0\n");
	    if (ash.dwScale != 0)
	      info->framerate = (double)ash.dwRate / (double)ash.dwScale;
	    debug_message_fnc("rate %d / %d scale = %f\n", ash.dwRate, ash.dwScale, info->framerate);
	    info->num_of_frames = ash.dwLength;
	  } else if (ash.fccType == FCC_auds) {
	    /* XXX: First stream only */
	    if (demux->nastreams == 0)
	      info->ahandler = ash.fccHandler;
	  } else {
	    show_message_fnc("Unknown fccType %X\n", ash.fccType);
	  }

	  if (!riff_file_read_chunk_header(info->rf, rc)) {
	    show_message_fnc("riff_file_read_chunk_header() failed: %s\n", riff_file_get_errmsg(info->rf));
	    ds = DEMULTIPLEX_ERROR;
	    goto error_free;
	  }
	  if (rc->fourcc != FCC_strf) {
	    debug_message_fnc("'strf' chunk expected, but got chunk '%s'\n", rc->name);
	    ds = DEMULTIPLEX_NOT;
	    goto error_free;
	  }
	  debug_message_fnc("Got chunk 'strf'\n");
	  riff_file_read_data(info->rf, rc);
	  /* XXX: Little endian only */
	  if (ash.fccType == FCC_vids) {
	    if (demux->nvstreams == 0) {
	      memcpy(&bih, riff_chunk_get_data(rc), sizeof(BITMAPINFOHEADER));
	      if (info->vhandler == 0) {
		debug_message_fnc("using bih.biCompression(%x) for vhandler\n", bih.biCompression);
		info->vhandler = bih.biCompression;
	      }
	      info->width = bih.biWidth;
	      info->height = bih.biHeight;
	      if (info->video_extradata && info->video_extradata_size != riff_chunk_get_size(rc) - sizeof(BITMAPINFOHEADER)) {
		free(info->video_extradata);
		info->video_extradata = NULL;
	      }
	      info->video_extradata_size = riff_chunk_get_size(rc) - sizeof(BITMAPINFOHEADER);
	      if (info->video_extradata_size > 0) {
		debug_message_fnc("video_extradata %d bytes\n", info->video_extradata_size);
		if (info->video_extradata == NULL)
		  info->video_extradata = malloc(info->video_extradata_size);
		memcpy(info->video_extradata, riff_chunk_get_data(rc) + sizeof(BITMAPINFOHEADER), info->video_extradata_size);
	      }
	    }
	    demux->nvstreams++;
	  } else if (ash.fccType == FCC_auds) {
	    /* XXX: First stream only */
	    if (demux->nastreams == 0) {
	      memcpy(&wfx, riff_chunk_get_data(rc), sizeof(WAVEFORMATEX));
	      if (info->ahandler == 0) {
		debug_message_fnc("using wfx.wFormatTag(%x) for ahandler\n", wfx.wFormatTag);
		info->ahandler = wfx.wFormatTag;
	      }
	      info->nchannels = wfx.nChannels;
	      info->samples_per_sec = wfx.nSamplesPerSec;
	      info->block_align = wfx.nBlockAlign;
	      info->avg_bytes_per_sec = wfx.nAvgBytesPerSec;
	      if (info->audio_extradata && info->audio_extradata_size != riff_chunk_get_size(rc) - sizeof(WAVEFORMATEX)) {
		free(info->audio_extradata);
		info->audio_extradata = NULL;
	      }
	      info->audio_extradata_size = riff_chunk_get_size(rc) - sizeof(WAVEFORMATEX);
	      if (info->audio_extradata_size > 0) {
		debug_message_fnc("audio_extradata %d bytes\n", info->audio_extradata_size);
		if (info->audio_extradata == NULL)
		  info->audio_extradata = malloc(info->audio_extradata_size);
		memcpy(info->audio_extradata, riff_chunk_get_data(rc) + sizeof(WAVEFORMATEX), info->audio_extradata_size);
	      }
	    }
	    demux->nastreams++;
	  }
	  riff_chunk_free_data(rc);
	}
	break;
      default:
	debug_message_fnc("Got list '%s'... not handled\n", riff_chunk_get_list_name(rc));
	riff_file_skip_chunk_data(info->rf, rc);
	break;
      }
      break;
    default:
      debug_message_fnc("Got chunk '%s' at %d ... not handled\n", riff_chunk_get_name(rc), riff_chunk_get_pos(rc));
      riff_file_skip_chunk_data(info->rf, rc);
      break;
    }
  }

  if (info->movi_start == 0) {
    show_message_fnc("'movi' chunk not found.\n");
    ds = DEMULTIPLEX_NOT;
    goto error_free;
  }

  riff_chunk_destroy(rc);
  return DEMULTIPLEX_OK;

 error_free:
  riff_chunk_destroy(rc);
 error_destroy:
  riff_file_destroy(info->rf);
  info->rf = NULL;
  return ds;
}

static void *
demux_main(void *arg)
{
  Demultiplexer *demux = (Demultiplexer *)arg;
  AVIInfo *info = (AVIInfo *)demux->private_data;
  RIFF_Chunk *rc;

  if (!info->rf) {
    if (!__examine(demux, 0))
      pthread_exit((void *)0);
  }

  if ((rc = riff_chunk_create()) == NULL)
    pthread_exit((void *)0);

  demux->running = 1;
  while (demux->running) {
    DemuxedPacket *dp;
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
      if (nstream == demux->nvstream && demux->vstream) {
	if (!riff_file_read_data(info->rf, rc))
	  break;
	if (riff_chunk_get_size(rc) > 0) {
	  if ((dp = malloc(sizeof(DemuxedPacket))) == NULL)
	    fatal("%s: No enough memory.\n", __FUNCTION__);
	  dp->size = riff_chunk_get_size(rc);
	  dp->data = riff_chunk_get_data(rc);
	  fifo_put(demux->vstream, dp, demultiplexer_destroy_packet);
	}
      } else {
	riff_file_skip_chunk_data(info->rf, rc);
      }
    } else if (p[2] == 'w' && p[3] == 'b') {
      /* audio data */
      if (nstream == demux->nastream && demux->astream) {
	if (!riff_file_read_data(info->rf, rc))
	  break;
	if (riff_chunk_get_size(rc) > 0) {
	  if ((dp = malloc(sizeof(DemuxedPacket))) == NULL)
	    fatal("%s: No enough memory.\n", __FUNCTION__);
	  dp->size = riff_chunk_get_size(rc);
	  dp->data = riff_chunk_get_data(rc);
	  fifo_put(demux->astream, dp, demultiplexer_destroy_packet);
	}
      } else {
	riff_file_skip_chunk_data(info->rf, rc);
      }
    } else {
      debug_message_fnc("Got chunk '%s' at %d, skipped\n", riff_chunk_get_name(rc), riff_chunk_get_pos(rc));
      riff_file_skip_chunk_data(info->rf, rc);
    }
  }

  riff_chunk_destroy(rc);

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
  void *ret;

  demux->running = 0;
  if (demux->thread) {
    debug_message_fn(" demultiplexer_avi\n");

    pthread_join(demux->thread, &ret);
    demux->thread = 0;

    debug_message_fn(" demultiplexer_avi OK\n");
  }

  return 1;
}

static int
demux_rewind(Demultiplexer *demux)
{
  AVIInfo *info = (AVIInfo *)demux->private_data;

  if (demux->running)
    return 0;
  return stream_seek(demux->st, info->movi_start, _SET);
}
