/*
 * demultiplexer_mpeg.c -- MPEG stream demultiplexer
 * (C)Copyright 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Thu Feb 22 01:28:14 2001.
 * $Id: demultiplexer_mpeg.c,v 1.3 2001/02/21 17:56:29 sian Exp $
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
#include "compat.h"
#include "common.h"

#include "demultiplexer_mpeg.h"
#include "enfle/utils.h"

#ifndef USE_PTHREAD
#  error pthread is mandatory for demultiplexer_mpeg
#endif

DECLARE_DEMULTIPLEXER_METHODS;
PREPARE_DEMULTIPLEXER_TEMPLATE;

Demultiplexer *
demultiplexer_mpeg_create(void)
{
  Demultiplexer *demux;
  MpegInfo *info;

  if ((demux = demultiplexer_create()) == NULL)
    return NULL;
  memcpy(demux, &template, sizeof(Demultiplexer));

  if ((info = calloc(1, sizeof(MpegInfo))) == NULL) {
    free(demux);
    return NULL;
  }

  demux->private_data = (void *)info;

  return demux;
}  

#define DEMULTIPLEXER_MPEG_BUFFER_SIZE 65536
#define DEMULTIPLEXER_MPEG_DETERMINE_SIZE 65536
#define MPEG_PROGRAM_END 0xb9
#define MPEG_PACK_HEADER 0xba
#define MPEG_SYSTEM_START 0xbb
#define MPEG_AUDIO_AC3 0xbd /* suspicious */
#define MPEG_PADDING 0xbe

static int
examine(Demultiplexer *demux)
{
  MpegInfo *info = (MpegInfo *)demux->private_data;
  unsigned char *buf, id;
  int read_total, read_size, used_size, skip;
  int nvstream, nastream;

  debug_message(__FUNCTION__ "()\n");

  if ((buf = malloc(DEMULTIPLEXER_MPEG_BUFFER_SIZE)) == NULL)
    return 0;
  used_size = 0;
  read_total = 0;

  info->ver = 0;
  info->nastreams = 0;
  info->nvstreams = 0;

  do {
    if ((read_size = stream_read(info->st, buf + used_size, DEMULTIPLEXER_MPEG_BUFFER_SIZE - used_size)) < 0) {
      show_message(__FUNCTION__ ": read error.\n");
      goto error;
    }
    if (read_size == 0) {
      if (used_size < 4)
	break;
    } else {
      used_size += read_size;
      read_total += read_size;
      if (read_total > DEMULTIPLEXER_MPEG_DETERMINE_SIZE && info->ver == 0) {
	debug_message(__FUNCTION__ ": determined as not MPEG.\n");
	goto error;
      }
      if (used_size < 4)
	continue;
    }

    if (buf[0]) {
      memcpy(buf, buf + 1, used_size - 1);
      used_size--;
      continue;
    }
    if (buf[1]) {
      memcpy(buf, buf + 2, used_size - 2);
      used_size -= 2;
      continue;
    }
    if (buf[2] != 1) {
      if (buf[2]) {
	memcpy(buf, buf + 3, used_size - 3);
	used_size -= 3;
	continue;
      } else {
	memcpy(buf, buf + 1, used_size - 1);
	used_size--;
	continue;
      }
    }

    id = buf[3];
    switch (id) {
    case MPEG_PROGRAM_END:
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
	show_message(__FUNCTION__ ": weird version\n");
	goto error;
      }
      break;
    case MPEG_SYSTEM_START:
      skip = 6 + utils_get_big_uint16(buf + 4);
      if (used_size < skip)
	continue;
      break;
    case MPEG_AUDIO_AC3:
      skip = 6 + utils_get_big_uint16(buf + 4);
      if (used_size < skip)
	continue;
      debug_message("MPEG_AUDIO_AC3??\n");
      break;
    case MPEG_PADDING:
      skip = 6 + utils_get_big_uint16(buf + 4);
      if (used_size < skip)
	continue;
      break;
    default:
      skip = 6 + utils_get_big_uint16(buf + 4);
      if (used_size < skip)
	continue;
      if ((id & 0xe0) == 0xc0) {
	nastream = id & 0x1f;
	if (info->nastreams < nastream)
	  info->nastreams = nastream;
      } else if ((id & 0xf0) == 0xe0) {
	nvstream = id & 0xf;
	if (info->nvstreams < nvstream)
	  info->nvstreams = nvstream;
      } else if (id < 0xb9) {
	debug_message("Looks like video stream.\n");
	goto end;
      } else {
	debug_message("Unknown id %02X %d bytes\n", id, skip - 6);
      }
      break;
    }
    memcpy(buf, buf + skip, used_size - skip);
    used_size -= skip;
  } while (1);

 end:
  free(buf);
  return 1;

 error:
  free(buf);
  return 0;
}

static void *
demux_main(void *arg)
{
  Demultiplexer *demux = (Demultiplexer *)arg;
  MpegInfo *info = (MpegInfo *)demux->private_data;
  unsigned char *buf, id;
  int read_total, read_size, used_size, skip;
  int nvstream, nastream;

  debug_message(__FUNCTION__ "()\n");

  if ((buf = malloc(DEMULTIPLEXER_MPEG_BUFFER_SIZE)) == NULL)
    pthread_exit((void *)0);
  used_size = 0;
  read_total = 0;

  do {
    if ((read_size = stream_read(info->st, buf + used_size, DEMULTIPLEXER_MPEG_BUFFER_SIZE - used_size)) < 0) {
      show_message(__FUNCTION__ ": read error.\n");
      goto error;
    }
    if (read_size == 0) {
      demux->eof = 1;
      break;
    } else {
      used_size += read_size;
      read_total += read_size;
      if (read_total > DEMULTIPLEXER_MPEG_DETERMINE_SIZE && info->ver == 0) {
	debug_message(__FUNCTION__ ": determined as not MPEG.\n");
	goto error;
      }
      if (used_size < 4)
	continue;
    }

    if (buf[0]) {
      memcpy(buf, buf + 1, used_size - 1);
      used_size--;
      continue;
    }
    if (buf[1]) {
      memcpy(buf, buf + 2, used_size - 2);
      used_size -= 2;
      continue;
    }
    if (buf[2] != 1) {
      if (buf[2]) {
	memcpy(buf, buf + 3, used_size - 3);
	used_size -= 3;
	continue;
      } else {
	memcpy(buf, buf + 1, used_size - 1);
	used_size--;
	continue;
      }
    }

    id = buf[3];
    switch (id) {
    case MPEG_PROGRAM_END:
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
	show_message(__FUNCTION__ ": weird version\n");
	goto error;
      }
      break;
    case MPEG_SYSTEM_START:
      skip = 6 + utils_get_big_uint16(buf + 4);
      if (used_size < skip)
	continue;
      break;
    case MPEG_AUDIO_AC3:
      skip = 6 + utils_get_big_uint16(buf + 4);
      if (used_size < skip)
	continue;
      debug_message("MPEG_AUDIO_AC3??\n");
      break;
    case MPEG_PADDING:
      skip = 6 + utils_get_big_uint16(buf + 4);
      if (used_size < skip)
	continue;
      break;
    default:
      skip = 6 + utils_get_big_uint16(buf + 4);
      if (used_size < skip)
	continue;
      if ((id & 0xe0) == 0xc0) {
	nastream = id & 0x1f;
	if (nastream == info->nastream) {
	  if ((buf[6] & 0xc0) == 0x80) {
	    /* ?, flags, header_len */
	    write(info->v_fd, buf + 9 + buf[8], skip - 9 - buf[8]);
	  } else {
	    unsigned char *p;

	    for (p = buf + 6; *p == 0xff && p < buf + skip; p++) ;
	    if ((*p & 0xc0) == 0x40) {
	      /* buffer scale, buffer size */
	      p += 2;
	    }
	    switch ((*p & 0xf0) >> 4) {
	    case 0:
	      p++;
	      break;
	    case 2:
	      /* presentation time stamp */
	      p += 5;
	      break;
	    case 3:
	      /* presentation time stamp, decoding time stamp */
	      p += 10;
	      break;
	    default:
	      goto error;
	    }
	    if (p < buf + skip)
	      write(info->v_fd, p, buf + skip - p);
	  }
	}
      } else if ((id & 0xf0) == 0xe0) {
	nvstream = id & 0xf;
	if (nvstream == info->nvstream) {
	  if ((buf[6] & 0xc0) == 0x80) {
	    /* ?, flags, header_len */
	    write(info->v_fd, buf + 9 + buf[8], skip - 9 - buf[8]);
	  } else {
	    unsigned char *p;

	    for (p = buf + 6; *p == 0xff && p < buf + skip; p++) ;
	    if ((*p & 0xc0) == 0x40) {
	      /* buffer scale, buffer size */
	      p += 2;
	    }
	    switch ((*p & 0xf0) >> 4) {
	    case 0:
	      p++;
	      break;
	    case 2:
	      /* presentation time stamp */
	      p += 5;
	      break;
	    case 3:
	      /* presentation time stamp, decoding time stamp */
	      p += 10;
	      break;
	    default:
	      goto error;
	    }
	    if (p < buf + skip)
	      write(info->v_fd, p, buf + skip - p);
	  }
	}
      } else if (id < 0xb9) {
	debug_message("Looks like video stream.\n");
	goto end;
      } else {
	debug_message("Unknown id %02X %d bytes\n", id, skip - 6);
      }
      break;
    }
    memcpy(buf, buf + skip, used_size - skip);
    used_size -= skip;
  } while (demux->running);

 end:
  free(buf);
  pthread_exit((void *)1);

 error:
  free(buf);
  pthread_exit((void *)0);
}

static int
start(Demultiplexer *demux)
{
  if (demux->running)
    return 0;
  demux->running = 1;
  pthread_create(&demux->thread, NULL, demux_main, demux);

  return 1;
}

static int
stop(Demultiplexer *demux)
{
  void *ret;

  if (!demux->running)
    return 0;
  demux->running = 0;
  pthread_join(demux->thread, &ret);

  return 1;
}

static void
destroy(Demultiplexer *demux)
{
  stop(demux);
  if (demux->private_data)
    free(demux->private_data);
  free(demux);
}
