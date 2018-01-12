/*
 * ogg.c -- OGG stream demultiplexer plugin
 * (C)Copyright 2001-2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Dec 26 01:13:14 2005.
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
#include <ogg/ogg.h>

#define REQUIRE_UNISTD_H
#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#ifndef USE_PTHREAD
#  error pthread is mandatory for demultiplexer_ogg
#endif

#include "enfle/demultiplexer-plugin.h"
#include "demultiplexer_ogg.h"
#define UTILS_NEED_GET_LITTLE_UINT16
#define UTILS_NEED_GET_LITTLE_UINT32
#include "enfle/utils.h"

DECLARE_DEMULTIPLEXER_PLUGIN_METHODS;
DECLARE_DEMULTIPLEXER_METHODS;
PREPARE_DEMULTIPLEXER_TEMPLATE;

static DemultiplexerPlugin plugin = {
  .type = ENFLE_PLUGIN_DEMULTIPLEXER,
  .name = "OGG",
  .description = "OGG Demultiplexer plugin version 0.1",
  .author = "Hiroshi Takekawa",

  .identify = identify,
  .examine = examine
};

ENFLE_PLUGIN_ENTRY(demultiplexer_ogg)
{
  DemultiplexerPlugin *dp;

  if ((dp = (DemultiplexerPlugin *)calloc(1, sizeof(DemultiplexerPlugin))) == NULL)
    return NULL;
  memcpy(dp, &plugin, sizeof(DemultiplexerPlugin));

  return (void *)dp;
}

ENFLE_PLUGIN_EXIT(demultiplexer_ogg, p)
{
  free(p);
}

/* demultiplexer plugin methods */

static Demultiplexer *
create(void)
{
  Demultiplexer *demux;
  OGGInfo *info;

  if ((demux = _demultiplexer_create()) == NULL)
    return NULL;
  memcpy(demux, &template, sizeof(Demultiplexer));

  if ((info = calloc(1, sizeof(OGGInfo))) == NULL) {
    _demultiplexer_destroy(demux);
    return NULL;
  }

  demux->private_data = (void *)info;

  return demux;
}  


static void
destroy(Demultiplexer *demux)
{
  OGGInfo *info = (OGGInfo *)demux->private_data;

  stop(demux);
  if (info)
    free(info);
  _demultiplexer_destroy(demux);
}

static DemultiplexerStatus __examine(Demultiplexer *, int);

DEFINE_DEMULTIPLEXER_PLUGIN_IDENTIFY(st, c __attribute__((unused)), priv __attribute__((unused)))
{
  Demultiplexer *demux = create();
  DemultiplexerStatus ds;

  demux->st = st;
  ds = __examine(demux, 1);

  return ds;
}

DEFINE_DEMULTIPLEXER_PLUGIN_EXAMINE(m, st, c __attribute__((unused)), priv __attribute__((unused)))
{
  Demultiplexer *demux = create();
  DemultiplexerStatus ds;
  OGGInfo *info = (OGGInfo *)demux->private_data;

  demux->st = st;
  if ((ds = __examine(demux, 0)) != DEMULTIPLEX_OK) {
    destroy(demux);
    return NULL;
  }

  m->width = info->width;
  m->height = info->height;
  m->framerate = info->framerate;
  m->num_of_frames = 0;

  m->v_fourcc = info->vhandler;
  m->a_fourcc = WAVEFORMAT_TAG_VORBIS;

  return demux;
}

#define OGG_BLOCK_SIZE 4096
static DemultiplexerStatus
__examine(Demultiplexer *demux, int identify_only)
{
  static unsigned char form_ogg[] = { 'O', 'g', 'g', 'S' };
  unsigned char buffer[4];
  OGGInfo *info = (OGGInfo *)demux->private_data;
  DemultiplexerStatus ds;
  ogg_page og;
  ogg_stream_state os;
  ogg_packet op;
  char *buf;

  demux->nastreams = 0;
  demux->nvstreams = 0;
  demux->vstreams[0] = 0;
  demux->astreams[0] = 1;

  if (stream_read(demux->st, buffer, 4) != 4)
    return DEMULTIPLEX_NOT;
  if (memcmp(buffer, form_ogg, 4) != 0)
    return DEMULTIPLEX_NOT;
  if (identify_only)
    return DEMULTIPLEX_OK;
  stream_rewind(demux->st);

  ogg_sync_init(&info->oy);
  while (1) {
    int res;
    int read_size = 0;

    if ((res = ogg_sync_pageout(&info->oy, &og)) == -1) {
      ds = DEMULTIPLEX_NOT;
      goto error;
    } else if (res == 0) {
      /* Need more data */
      buf = ogg_sync_buffer(&info->oy, OGG_BLOCK_SIZE);
      if ((read_size = stream_read(demux->st, (unsigned char *)buf, OGG_BLOCK_SIZE)) < 0) {
	err_message_fnc("stream_read error.\n");
	ds = DEMULTIPLEX_ERROR;
	goto error;
      }
      if (read_size == 0)
	break;
      ogg_sync_wrote(&info->oy, read_size);
      continue;
    }
    if (!ogg_page_bos(&og))
      break;
    ogg_stream_init(&os, ogg_page_serialno(&og));
    ogg_stream_pagein(&os, &og);
    ogg_stream_packetout(&os, &op);
    if (op.bytes >= 7 && strncmp((char *)&op.packet[1], "vorbis", 6) == 0) {
      demux->nastreams++;
      info->a_serialno = ogg_page_serialno(&og);
      debug_message_fnc("vorbis stream(%d) found.\n", info->a_serialno);
    } else if (op.bytes >= 6 && strncmp((char *)&op.packet[1], "video", 5) == 0) {
      demux->nvstreams++;
      info->v_serialno = ogg_page_serialno(&og);
      debug_message_fnc("video stream(%d) found: ", info->v_serialno);
      info->vhandler = utils_get_little_uint32(op.packet + 9);
      if (utils_get_little_uint32(op.packet + 21) == 0) {
	info->framerate.num = 10000000;
	info->framerate.den = utils_get_little_uint32(op.packet + 17);
      } else {
	err_message_fnc("time_unit beyond 32 bits (%X)...\n", utils_get_little_uint32(op.packet + 21));
	/* XXX: gee... */
	info->framerate.num = 10000000;
	info->framerate.den = utils_get_little_uint32(op.packet + 17);
      }
      info->width = utils_get_little_uint32(op.packet + 45);
      info->height = utils_get_little_uint32(op.packet + 49);
      debug_message_fnc("(%d,%d) %2.5f fps\n", info->width, info->height, rational_to_double(info->framerate));
    } else if (op.bytes >= 6 && strncmp((char *)&op.packet[1], "audio", 5) == 0) {
      demux->nastreams++;
      info->a_serialno = ogg_page_serialno(&og);
      debug_message_fnc("audio stream(%d) found, but not supported yet...\n", info->a_serialno);
#if 0
typedef struct stream_header {
  char streamtype[8];           // 1
  char subtype[4];              // 9
  ogg_int32_t size;             // 13 // size of the structure
  ogg_int64_t time_unit;        // 17 // in reference time
  ogg_int64_t samples_per_unit; // 25
  ogg_int32_t default_len;      // 33 // in media time
  ogg_int32_t buffersize;       // 37
  ogg_int16_t bits_per_sample;  // 41
  stream_header_audio     audio;
} stream_header;
typedef struct stream_header_audio {
  ogg_int16_t channels;         // 43
  ogg_int16_t blockalign;       // 45
  ogg_int32_t avgbytespersec;   // 47
} stream_header_audio;
#endif
    } else {
      debug_message_fnc("UNKNOWN stream found.\n");
    }
  }

  ogg_sync_clear(&info->oy);
  return DEMULTIPLEX_OK;

 error:
  ogg_sync_clear(&info->oy);
  return ds;
}

static void *
demux_main(void *arg)
{
  Demultiplexer *demux = (Demultiplexer *)arg;
  OGGInfo *info = (OGGInfo *)demux->private_data;
  DemuxedPacket *dp;
  DemultiplexerStatus ds;
  char *buf;
  int res, serialno, first, v_or_a;
  int read_size = 0;
  ogg_page og;
  ogg_packet op;
  ogg_stream_state os[2];

  if (demux->running)
    return (void *)0;

  demux->running = 1;

  ogg_sync_init(&info->oy);
  while (demux->running) {
    if ((res = ogg_sync_pageout(&info->oy, &og)) == -1) {
      debug_message_fn("ogg_sync_pageout() failed\n");
      ds = DEMULTIPLEX_NOT;
      goto error;
    } else if (res == 0) {
      /* Need more data */
      buf = ogg_sync_buffer(&info->oy, OGG_BLOCK_SIZE);
      if ((read_size = stream_read(demux->st, (unsigned char *)buf, OGG_BLOCK_SIZE)) < 0) {
	err_message_fnc("stream_read error.\n");
	ds = DEMULTIPLEX_ERROR;
	goto error;
      }
      if (read_size == 0)
	break;
      ogg_sync_wrote(&info->oy, read_size);
      continue;
    }

    serialno = ogg_page_serialno(&og);
    first = ogg_page_bos(&og);
    if (serialno == info->v_serialno)
      v_or_a = 0;
    else if (serialno == info->a_serialno)
      v_or_a = 1;
    else {
      if (first)
	debug_message_fnc("stream(%d) is skipped.\n", serialno);
      continue;
    }

    if (first)
      ogg_stream_init(&os[v_or_a], serialno);
    ogg_stream_pagein(&os[v_or_a], &og);
    for (;;) {
      if ((res = ogg_stream_packetout(&os[v_or_a], &op)) == -1) {
	debug_message_fnc("ogg_stream_packetout() failed\n");
	ds = DEMULTIPLEX_ERROR;
	goto error;
      } else if (res == 0)
	break;
      if (v_or_a == 0) {
	int header_len;

	if ((op.packet[0] & 3) == PACKET_TYPE_HEADER ||
	    (op.packet[0] & 3) == PACKET_TYPE_COMMENT)
	  continue;
	header_len  = (op.packet[0] & PACKET_LEN_BITS01) >> 6;
	header_len |= (op.packet[0] & PACKET_LEN_BITS2)  << 1;
#if 0
	{
	  int i, sample_len;
	  if (header_len > 0 && op.bytes >= header_len + 1) {
	    sample_len = 0;
	    for (i = 0; i < header_len; i++) {
	      sample_len <<= 8;
	      sample_len += op.packet[header_len - i];
	    }
	  }
	}
#endif
	dp = malloc(sizeof(DemuxedPacket));
	dp->pts = dp->dts = -1;
	dp->size = op.bytes - 1 - header_len;
	dp->data = malloc(dp->size);
	memcpy(dp->data, op.packet + 1 + header_len, dp->size);
	fifo_put(demux->vstream, dp, demultiplexer_destroy_packet);
      } else {
	/* XXX: This code is for vorbis only.
	   If mp3 or some like that, you should handle it. */
	dp = malloc(sizeof(DemuxedPacket));
	dp->pts = dp->dts = -1;
	dp->size = op.bytes;
	dp->data = malloc(dp->size);
	memcpy(dp->data, op.packet, dp->size);
	fifo_put(demux->astream, dp, demultiplexer_destroy_packet);
      }
    }
  }

  ds = DEMULTIPLEX_OK;

 error:
  demultiplexer_set_eof(demux, 1);
  demux->running = 0;
  ogg_sync_clear(&info->oy);
  return (void *)ds;
}

static int
start(Demultiplexer *demux)
{
  if (demux->running)
    return 0;

  debug_message_fn(" demultiplexer_ogg\n");

  pthread_create(&demux->thread, NULL, demux_main, demux);

  return 1;
}

static int
stop(Demultiplexer *demux)
{
  void *ret;

  demux->running = 0;
  if (demux->thread) {
    debug_message_fn(" demultiplexer_ogg\n");

    pthread_join(demux->thread, &ret);
    demux->thread = 0;

    debug_message_fn(" demultiplexer_ogg OK\n");
  }

  return 1;
}

static int
demux_rewind(Demultiplexer *demux)
{
  //OGGInfo *info = (OGGInfo *)demux->private_data;

  if (demux->running)
    return 0;
  //return stream_seek(demux->st, info->movi_start, _SET);
  return stream_seek(demux->st, 0, _SET);
}
