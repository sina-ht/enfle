/*
 * mpeg.c -- MPEG stream demultiplexer plugin
 * (C)Copyright 2001-2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Jun 21 22:43:13 2004.
 * $Id: mpeg.c,v 1.9 2004/06/21 13:47:26 sian Exp $
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
#  error pthread is mandatory for demultiplexer_mpeg
#endif

#include "enfle/demultiplexer-plugin.h"
#include "demultiplexer_mpeg.h"
#define UTILS_NEED_GET_BIG_UINT16
#include "enfle/utils.h"
#include "enfle/fourcc.h"

DECLARE_DEMULTIPLEXER_PLUGIN_METHODS;
DECLARE_DEMULTIPLEXER_METHODS;
PREPARE_DEMULTIPLEXER_TEMPLATE;

static DemultiplexerPlugin plugin = {
  .type = ENFLE_PLUGIN_DEMULTIPLEXER,
  .name = "MPEG",
  .description = "MPEG Demultiplexer plugin version 0.2",
  .author = "Hiroshi Takekawa",

  .identify = identify,
  .examine = examine
};

ENFLE_PLUGIN_ENTRY(demultiplexer_mpeg)
{
  DemultiplexerPlugin *dp;

  if ((dp = (DemultiplexerPlugin *)calloc(1, sizeof(DemultiplexerPlugin))) == NULL)
    return NULL;
  memcpy(dp, &plugin, sizeof(DemultiplexerPlugin));

  return (void *)dp;
}

ENFLE_PLUGIN_EXIT(demultiplexer_mpeg, p)
{
  free(p);
}

/* demultiplexer plugin methods */

#define DEMULTIPLEXER_MPEG_BUFFER_SIZE 65536*4
#define DEMULTIPLEXER_MPEG_IDENTIFY_SIZE 4096
#define DEMULTIPLEXER_MPEG_DETERMINE_SIZE 65536*8

/*
#define MPEG_USER_DATA 0xb2
#define MPEG_SEQUENCE_START 0xb3
#define MPEG_SEQUENCE_ERROR 0xb4
#define MPEG_EXTENSION_START 0xb5
#define MPEG_SEQUENCE_END 0xb7
#define MPEG_GROUP_START 0xb8
*/
#define MPEG_PROGRAM_END 0xb9
#define MPEG_PACK_HEADER 0xba
#define MPEG_SYSTEM_START 0xbb
#define MPEG_RESERVED_STREAM1 0xbc
#define MPEG_PRIVATE_STREAM1 0xbd
#define MPEG_PADDING 0xbe
#define MPEG_PRIVATE_STREAM2 0xbf

static Demultiplexer *
create(void)
{
  Demultiplexer *demux;
  MpegInfo *info;

  if ((demux = _demultiplexer_create()) == NULL)
    return NULL;
  memcpy(demux, &template, sizeof(Demultiplexer));

  if ((info = calloc(1, sizeof(MpegInfo))) == NULL) {
    _demultiplexer_destroy(demux);
    return NULL;
  }

  demux->private_data = (void *)info;

  return demux;
}

static void
destroy(Demultiplexer *demux)
{
  stop(demux);
  if (demux->private_data)
    free(demux->private_data);
  _demultiplexer_destroy(demux);
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
  MpegInfo *info = (MpegInfo *)demux->private_data;
  DemultiplexerStatus ds;

  demux->st = st;
  stream_rewind(st);
  if ((ds = __examine(demux, 0)) != DEMULTIPLEX_OK) {
    destroy(demux);
    return NULL;
  }

  /* These will be set after reading SEQUENCE_START. */
  m->width = 0;
  m->height = 0;
  m->framerate = 0;
  m->num_of_frames = 0;

  m->v_fourcc = info->ver == 1 ? FCC_mpg1 : FCC_mpg2;
  m->a_fourcc = WAVEFORMAT_TAG_MP3; /* hmm... */

  return demux;
}

static DemultiplexerStatus
__examine(Demultiplexer *demux, int identify_only)
{
  MpegInfo *info = (MpegInfo *)demux->private_data;
  DemultiplexerStatus ds;
  unsigned char *buf, id;
  int read_total, read_size, used_size, used_size_prev, skip;
  int nvstream, nastream;
  int vstream, astream;
  int maximum_size = identify_only ? DEMULTIPLEXER_MPEG_IDENTIFY_SIZE : DEMULTIPLEXER_MPEG_DETERMINE_SIZE;

  debug_message("%s...\n", identify_only ? "identify" : "examine");

  vstream = 0;
  astream = 0;
  demux->vstreams[0] = 0;
  demux->astreams[0] = 0;

  if ((buf = malloc(DEMULTIPLEXER_MPEG_BUFFER_SIZE)) == NULL)
    return DEMULTIPLEX_ERROR;
  used_size = used_size_prev = 0;
  read_total = 0;
  skip = 1;

  do {
    if (used_size < skip && read_total < maximum_size) {
      if ((read_size = stream_read(demux->st, buf + used_size,
				   DEMULTIPLEXER_MPEG_BUFFER_SIZE - used_size)) < 0) {
	err_message_fnc("stream_read error.\n");
	ds = DEMULTIPLEX_ERROR;
	goto error;
      }
      if (read_size == 0)
	break;
      used_size += read_size;
      read_total += read_size;
    }

    if (read_total >= maximum_size) {
      if (used_size <= 12 || used_size_prev == used_size) {
	if (info->ver == 0) {
	  debug_message_fnc("Not determined as MPEG.\n");
	  ds = DEMULTIPLEX_NOT;
	  goto error;
	} else {
	  break;
	}
      }
      used_size_prev = used_size;
    }

    if (buf[0]) {
      memmove(buf, buf + 1, used_size - 1);
      used_size--;
      continue;
    }
    if (buf[1]) {
      memmove(buf, buf + 2, used_size - 2);
      used_size -= 2;
      continue;
    }
    if (buf[2] != 1) {
      if (buf[2]) {
	memmove(buf, buf + 3, used_size - 3);
	used_size -= 3;
	continue;
      } else {
	memmove(buf, buf + 1, used_size - 1);
	used_size--;
	continue;
      }
    }

    id = buf[3];
    switch (id) {
    case MPEG_PROGRAM_END:
      debug_message_fnc("MPEG_PROGRAM_END\n");
      goto end;
    case MPEG_PACK_HEADER:
      if ((buf[4] & 0xc0) == 0x40) {
	skip = 14 + (buf[13] & 7);
	if (used_size < skip)
	  continue;
	info->ver = 2;
      } else if ((buf[4] & 0xf0) == 0x20) {
	skip = 12;
	if (used_size < skip)
	  continue;
	info->ver = 1;
      } else {
	show_message_fnc("MPEG neither I nor II.\n");
	ds = DEMULTIPLEX_NOT;
	goto error;
      }
      //debug_message_fnc("MPEG_PACK_HEADER: version %d skip %d\n", info->ver, skip);
      if (identify_only) {
	debug_message_fnc("MPEG %d stream detected\n", info->ver);
	goto end;
      }
      break;
    case MPEG_SYSTEM_START:
      debug_message_fnc("MPEG_SYSTEM_START\n");
      skip = 6 + utils_get_big_uint16(buf + 4);
      if (used_size < skip)
	continue;
      break;
    case MPEG_RESERVED_STREAM1:
      skip = 6 + utils_get_big_uint16(buf + 4);
      if (used_size < skip)
	continue;
      debug_message_fnc("MPEG_RESERVED_STREAM1\n");
      break;
    case MPEG_PRIVATE_STREAM1: /* AC3 */
      skip = 6 + utils_get_big_uint16(buf + 4);
      if (used_size < skip)
	continue;
      debug_message_fnc("MPEG_PRIVATE_STREAM1 (AC3)\n");
      break;
    case MPEG_PADDING:
      skip = 6 + utils_get_big_uint16(buf + 4);
      //debug_message_fnc("MPEG_PADDING, skip %d bytes\n", skip);
      if (used_size < skip)
	continue;
      break;
    case MPEG_PRIVATE_STREAM2:
      skip = 6 + utils_get_big_uint16(buf + 4);
      if (used_size < skip)
	continue;
      debug_message_fnc("MPEG_PRIVATE_STREAM2\n");
      break;
    default:
      skip = 6 + utils_get_big_uint16(buf + 4);
      if (used_size < skip)
	continue;
      if ((id & 0xe0) == 0xc0) {
	nastream = id & 0x1f;
	//debug_message_fnc("MPEG_AUDIO %d\n", nastream);
	if (!(astream & (1 << nastream))) {
	  astream |= 1 << nastream;
	  demux->nastreams++;
	}
      } else if ((id & 0xf0) == 0xe0) {
	nvstream = id & 0xf;
	//debug_message_fnc("MPEG_VIDEO %d\n", nvstream);
	if (!(vstream & (1 << nvstream))) {
	  vstream |= 1 << nvstream;
	  demux->nvstreams++;
	}
      } else if (id < 0xb9 && id >= 0xa0) {
	debug_message_fnc("Looks like video stream.\n");
	vstream = 1;
	demux->nvstreams = 1;
	info->ver = 3;
	goto end;
      }
      break;
    }
    memmove(buf, buf + skip, used_size - skip);
    used_size -= skip;
  } while (1);

 end:
  debug_message_fnc("OK\n");
  ds = DEMULTIPLEX_OK;
 error:
  if (identify_only)
    destroy(demux);
  free(buf);
  return ds;
}

/* XXX: should be 33 bits long */
static inline unsigned long
get_timestamp(unsigned char *p)
{
  unsigned long t;

  t = (p[0] >> 1) & 7;
  t <<= 15;
  t |= ((p[1] << 8) | p[2]) >> 1;
  t <<= 15;
  t |= ((p[3] << 8) | p[4]) >> 1;

  return t;
}

#define CONTINUE_IF_RUNNING if (demux->running) continue; else break

#define TS_BASE 90000

static void *
demux_main(void *arg)
{
  Demultiplexer *demux = (Demultiplexer *)arg;
  MpegInfo *info = (MpegInfo *)demux->private_data;
  DemuxedPacket *dp;
  unsigned char *buf, id;
  int read_total, read_size, used_size, used_size_prev = 0, skip;
  int nvstream = 0, nastream = 0;
  int v_or_a;

  if (demux->running)
    pthread_exit((void *)0);

  if ((buf = malloc(DEMULTIPLEXER_MPEG_BUFFER_SIZE)) == NULL)
    pthread_exit((void *)0);
  used_size = 0;
  read_total = 0;

  demux->running = 1;
  do {
    if (used_size < (DEMULTIPLEXER_MPEG_BUFFER_SIZE >> 1)) {
      if ((read_size = stream_read(demux->st, buf + used_size,
				   DEMULTIPLEXER_MPEG_BUFFER_SIZE - used_size)) < 0) {
	show_message_fnc("read error.\n");
	goto error;
      }
      used_size += read_size;
      read_total += read_size;
      if (read_size == 0) {
	//debug_message_fnc("%d %d\n", used_size, used_size_prev);
	if (used_size <= 12 || used_size_prev == used_size)
	  break;
	used_size_prev = used_size;
      }
    }

    if (info->ver == 3) {
      /* Video stream */
      skip = used_size;
      dp = malloc(sizeof(DemuxedPacket));
      dp->pts = dp->dts = -1;
      dp->size = skip;
      dp->data = malloc(dp->size);
      memcpy(dp->data, buf, dp->size);
      fifo_put(demux->vstream, dp, demultiplexer_destroy_packet);
      used_size = 0;
      CONTINUE_IF_RUNNING;
    }

    if (buf[0]) {
      memmove(buf, buf + 1, used_size - 1);
      used_size--;
      CONTINUE_IF_RUNNING;
    }
    if (buf[1]) {
      memmove(buf, buf + 2, used_size - 2);
      used_size -= 2;
      CONTINUE_IF_RUNNING;
    }
    if (buf[2] != 1) {
      if (buf[2]) {
	memmove(buf, buf + 3, used_size - 3);
	used_size -= 3;
	CONTINUE_IF_RUNNING;
      } else {
	memmove(buf, buf + 1, used_size - 1);
	used_size--;
	CONTINUE_IF_RUNNING;
      }
    }

    id = buf[3];
    switch (id) {
    case MPEG_PROGRAM_END:
      goto end;
    case MPEG_PACK_HEADER:
      if ((buf[4] & 0xc0) == 0x40) {
	skip = 14 + (buf[13] & 7);
	if (used_size < skip) {
	  CONTINUE_IF_RUNNING;
	}
	info->ver = 2;
      } else if ((buf[4] & 0xf0) == 0x20) {
	skip = 12;
	if (used_size < skip) {
	  CONTINUE_IF_RUNNING;
	}
	info->ver = 1;
      } else {
	err_message_fnc("MPEG neither I nor II.\n");
	goto error;
      }
      break;
    case MPEG_SYSTEM_START:
      skip = 6 + utils_get_big_uint16(buf + 4);
      if (used_size < skip) {
	CONTINUE_IF_RUNNING;
      }
      break;
    case MPEG_RESERVED_STREAM1:
      skip = 6 + utils_get_big_uint16(buf + 4);
      if (used_size < skip) {
	CONTINUE_IF_RUNNING;
      }
      debug_message_fnc("MPEG_RESERVED_STREAM1\n");
      break;
    case MPEG_PRIVATE_STREAM1: /* AC3 */
      skip = 6 + utils_get_big_uint16(buf + 4);
      if (used_size < skip) {
	CONTINUE_IF_RUNNING;
      }
      debug_message_fnc("MPEG_PRIVATE_STREAM1\n");
      break;
    case MPEG_PADDING:
      skip = 6 + utils_get_big_uint16(buf + 4);
      if (used_size < skip) {
	CONTINUE_IF_RUNNING;
      }
      break;
    case MPEG_PRIVATE_STREAM2:
      skip = 6 + utils_get_big_uint16(buf + 4);
      if (used_size < skip) {
	CONTINUE_IF_RUNNING;
      }
      debug_message_fnc("MPEG_PRIVATE_STREAM2\n");
      break;
    default:
      skip = 6 + utils_get_big_uint16(buf + 4);
      if (used_size < skip) {
	CONTINUE_IF_RUNNING;
      }
      if ((id & 0xe0) == 0xc0) {
	nastream = id & 0x1f;
	v_or_a = 2;
      } else if ((id & 0xf0) == 0xe0) {
	nvstream = id & 0x0f;
	v_or_a = 1;
      } else {
	v_or_a = 0;
      }

      if ((v_or_a == 1 && nvstream == demux->nvstream) ||
	  (v_or_a == 2 && nastream == demux->nastream)) {
	unsigned char *p;
	unsigned int pts, dts;

	/* stuffing */
	for (p = buf + 6; *p == 0xff && p < buf + skip; p++) ;

	if ((*p & 0xc0) == 0x40) {
	  /* buffer scale, buffer size */
	  p += 2;
	}

	switch (*p >> 4) {
	case 0:
	  p++;
	  break;
	case 2:
	  pts = get_timestamp(p);
	  dts = pts;
	  p += 5;
	  break;
	case 3:
	  /* presentation time stamp, decoding time stamp */
	  pts = get_timestamp(p);
	  dts = get_timestamp(p + 5);
	  p += 10;
	  break;
	default:
	  if ((*p & 0xc0) == 0x80) {
	    unsigned int flags, header_len;

	    /* MPEG II */
	    if ((*p & 0x30) != 0) {
	      warning_fnc("Encrypted multiplex cannot be handled.\n");
	      CONTINUE_IF_RUNNING;
	    }
	    p++;

	    flags = *p++;
	    header_len = *p++;

	    if ((flags & 0xc0) == 0x80) {
	      pts = get_timestamp(p);
	      dts = pts;
	      p += 5;
	      header_len -= 5;
	    } else if ((flags & 0xc0) == 0xc0) {
	      pts = get_timestamp(p);
	      dts = get_timestamp(p + 5);
	      p += 10;
	      header_len -= 10;
	    }
	    p += header_len;
	  } else {
	    warning_fnc("*p == %02X\n", *p);
	    CONTINUE_IF_RUNNING;
	  }
	  break;
	}
	if (p < buf + skip) {
	  dp = malloc(sizeof(DemuxedPacket));
	  dp->ts_base = TS_BASE;
	  dp->pts = pts;
	  dp->dts = dts;
	  dp->size = buf + skip - p;
	  dp->data = malloc(dp->size);
	  memcpy(dp->data, p, dp->size);
	  fifo_put((v_or_a == 1) ? demux->vstream : demux->astream, dp, demultiplexer_destroy_packet);
	} else {
	  warning_fnc("p >= buf + skip! demux error?\n");
	}
      } else if (!v_or_a) {
	if (id < 0xb9 && id >= 0xa0) {
	  /* Video stream */
	  debug_message_fnc("identified as video stream.  Put data in raw.\n");
	  skip = used_size;
	  dp = malloc(sizeof(DemuxedPacket));
	  dp->pts = dp->dts = -1;
	  dp->size = skip;
	  dp->data = malloc(dp->size);
	  memcpy(dp->data, buf, dp->size);
	  fifo_put(demux->vstream, dp, demultiplexer_destroy_packet);
	  info->ver = 3;
	} else {
	  debug_message_fnc("Unknown id %02X %d bytes\n", id, skip - 6);
	}
      }
      break;
    }
    memmove(buf, buf + skip, used_size - skip);
    used_size -= skip;
  } while (demux->running);

 end:
  demultiplexer_set_eof(demux, 1);
  demux->running = 0;
  free(buf);
  debug_message_fnc("exiting.\n");
  pthread_exit((void *)1);

 error:
  demux->running = 0;
  free(buf);
  return (void *)0;
}

static int
start(Demultiplexer *demux)
{
  if (demux->running)
    return 0;

  debug_message_fn(" demultiplexer_mpeg\n");

  pthread_create(&demux->thread, NULL, demux_main, demux);

  return 1;
}

static int
stop(Demultiplexer *demux)
{
  void *ret;

  debug_message_fn(" demultiplexer_mpeg\n");

  demux->running = 0;
  if (demux->thread) {
    pthread_join(demux->thread, &ret);
    demux->thread = 0;
  }

  debug_message_fn(" demultiplexer_mpeg OK\n");

  return 1;
}

static int
demux_rewind(Demultiplexer *demux)
{
  if (demux->running)
    return 0;
  return stream_rewind(demux->st);
}
