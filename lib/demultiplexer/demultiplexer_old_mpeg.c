/*
 * demultiplexer_old_mpeg.c -- MPEG stream demultiplexer
 * (C)Copyright 2001-2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Feb 13 00:08:25 2004.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include <stdlib.h>

#define REQUIRE_UNISTD_H
#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#ifndef USE_PTHREAD
#  error pthread is mandatory for demultiplexer_old_mpeg
#endif

#include "demultiplexer_old_mpeg.h"
#define UTILS_NEED_GET_BIG_UINT16
#include "enfle/utils.h"

DECLARE_DEMULTIPLEXER_OLD_METHODS;
PREPARE_DEMULTIPLEXER_OLD_TEMPLATE;

Demultiplexer_old *
demultiplexer_mpeg_create(void)
{
  Demultiplexer_old *demux;
  MpegInfo *info;

  if ((demux = _demultiplexer_old_create()) == NULL)
    return NULL;
  memcpy(demux, &template, sizeof(Demultiplexer_old));

  if ((info = calloc(1, sizeof(MpegInfo))) == NULL) {
    _demultiplexer_old_destroy(demux);
    return NULL;
  }

  demux->private_data = (void *)info;

  return demux;
}  

#define DEMULTIPLEXER_MPEG_BUFFER_SIZE 65536
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

static int
examine(Demultiplexer_old *demux)
{
  MpegInfo *info = (MpegInfo *)demux->private_data;
  unsigned char *buf, id;
  int read_total, read_size, used_size, used_size_prev = 0, skip;
  int nvstream, nastream;
  int vstream, astream;

  debug_message_fn("()\n");

  if ((buf = malloc(DEMULTIPLEXER_MPEG_BUFFER_SIZE)) == NULL)
    return 0;
  used_size = 0;
  read_total = 0;

  vstream = 0;
  astream = 0;
  info->ver = 0;
  info->nastreams = 0;
  info->nvstreams = 0;

  do {
    if (read_total < DEMULTIPLEXER_MPEG_DETERMINE_SIZE) {
      if ((read_size = stream_read(info->st, buf + used_size,
				   DEMULTIPLEXER_MPEG_BUFFER_SIZE - used_size)) < 0) {
	err_message_fnc("stream_read error.\n");
	goto error;
      }
      if (read_size == 0)
	break;
      used_size += read_size;
      read_total += read_size;
    }

    if (read_total >= DEMULTIPLEXER_MPEG_DETERMINE_SIZE) {
      if (used_size <= 12 || used_size_prev == used_size) {
	if (info->ver == 0) {
	  debug_message_fnc("Not determined as MPEG.\n");
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
	goto error;
      }
      //debug_message_fnc("MPEG_PACK_HEADER: version %d skip %d\n", info->ver, skip);
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
	  info->nastreams++;
	}
      } else if ((id & 0xf0) == 0xe0) {
	nvstream = id & 0xf;
	//debug_message_fnc("MPEG_VIDEO %d\n", nvstream);
	if (!(vstream & (1 << nvstream))) {
	  vstream |= 1 << nvstream;
	  info->nvstreams++;
	}
      } else if (id < 0xb9 && id >= 0xa0) {
	debug_message_fnc("Looks like video stream.\n");
	vstream = 1;
	info->nvstreams = 1;
	info->ver = 3;
	goto end;
      } else {
	debug_message_fnc("Unknown id %02X %d bytes\n", id, skip - 6);
      }
      break;
    }
    memmove(buf, buf + skip, used_size - skip);
    used_size -= skip;
  } while (1);

 end:
  debug_message_fnc("OK\n");
  free(buf);
  return 1;

 error:
  free(buf);
  return 0;
}

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

static void *
demux_main(void *arg)
{
  Demultiplexer_old *demux = (Demultiplexer_old *)arg;
  MpegInfo *info = (MpegInfo *)demux->private_data;
  DemuxedPacket_old *dp;
  unsigned char *buf, id;
  int read_total, read_size, used_size, used_size_prev = 0, skip;
  int nvstream = 0, nastream = 0;
  int v_or_a;

  if ((buf = malloc(DEMULTIPLEXER_MPEG_BUFFER_SIZE)) == NULL)
    pthread_exit((void *)0);
  used_size = 0;
  read_total = 0;

  if (demux->running)
    pthread_exit((void *)0);

  demux->running = 1;
  do {
    if (used_size < (DEMULTIPLEXER_MPEG_BUFFER_SIZE >> 1)) {
      if ((read_size = stream_read(info->st, buf + used_size,
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
      dp = malloc(sizeof(DemuxedPacket_old));
      dp->pts_dts_flag = 0xff;
      dp->pts = dp->dts = -1;
      dp->size = skip;
      dp->data = malloc(dp->size);
      memcpy(dp->data, buf, dp->size);
      fifo_put(info->vstream, dp, demultiplexer_old_destroy_packet);
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
	show_message_fnc("MPEG neither I nor II.\n");
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

      if ((v_or_a == 1 && nvstream == info->nvstream) ||
	  (v_or_a == 2 && nastream == info->nastream)) {
	if ((buf[6] & 0xc0) == 0x80) {
	  /* MPEG II */
	  dp = malloc(sizeof(DemuxedPacket_old));
	  dp->pts_dts_flag = 0xff;
	  dp->pts = dp->dts = -1;
	  dp->size = skip - 9 - buf[8];
	  dp->data = malloc(dp->size);
	  memcpy(dp->data, buf + 9 + buf[8], dp->size);
	  fifo_put((v_or_a == 1) ? info->vstream : info->astream, dp, demultiplexer_old_destroy_packet);
	} else {
	  unsigned char *p;
	  int pts_dts_flag;
	  unsigned long pts = 0, dts = 0;

	  for (p = buf + 6; *p == 0xff && p < buf + skip; p++) ;
	  if ((*p & 0xc0) == 0x40) {
	    /* buffer scale, buffer size */
	    p += 2;
	  }
	  pts_dts_flag = *p >> 4;
	  switch (pts_dts_flag) {
	  case 0:
	    p++;
	    break;
	  case 2:
	    /* presentation time stamp */
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
	    goto error;
	  }
	  if (p < buf + skip) {
	    dp = malloc(sizeof(DemuxedPacket_old));
	    dp->pts_dts_flag = pts_dts_flag;
	    dp->pts = pts;
	    dp->dts = dts;
	    dp->size = buf + skip - p;
	    dp->data = malloc(dp->size);
	    memcpy(dp->data, p, dp->size);
	    fifo_put((v_or_a == 1) ? info->vstream : info->astream, dp, demultiplexer_old_destroy_packet);
	  }
	}
      } else if (!v_or_a) {
	if (id < 0xb9 && id >= 0xa0) {
	  /* Video stream */
	  debug_message_fnc("identified as video stream.  Put data in raw.\n");
	  skip = used_size;
	  dp = malloc(sizeof(DemuxedPacket_old));
	  dp->pts_dts_flag = 0xff;
	  dp->pts = dp->dts = -1;
	  dp->size = skip;
	  dp->data = malloc(dp->size);
	  memcpy(dp->data, buf, dp->size);
	  fifo_put(info->vstream, dp, demultiplexer_old_destroy_packet);
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
  demultiplexer_old_set_eof(demux, 1);
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
start(Demultiplexer_old *demux)
{
  if (demux->running)
    return 0;

  debug_message_fn(" demultiplexer_old_mpeg\n");

  pthread_create(&demux->thread, NULL, demux_main, demux);

  return 1;
}

static int
stop(Demultiplexer_old *demux)
{
  void *ret;

  debug_message_fn(" demultiplexer_old_mpeg\n");

  demux->running = 0;
  if (demux->thread) {
    pthread_join(demux->thread, &ret);
    demux->thread = 0;
  }

  debug_message_fn(" demultiplexer_old_mpeg OK\n");

  return 1;
}

static int
demux_rewind(Demultiplexer_old *demux)
{
  MpegInfo *info = (MpegInfo *)demux->private_data;

  if (demux->running)
    return 0;
  return stream_rewind(info->st);
}

static void
destroy(Demultiplexer_old *demux)
{
  stop(demux);
  if (demux->private_data)
    free(demux->private_data);
  _demultiplexer_old_destroy(demux);
}
