/*
 * mp3.c -- MP3 stream demultiplexer plugin
 * (C)Copyright 2001-2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Nov 18 23:08:09 2011.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include <stdlib.h>

#define REQUIRE_UNISTD_H
#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#ifndef USE_PTHREAD
#  error pthread is mandatory for demultiplexer_mp3
#endif

#include "enfle/demultiplexer-plugin.h"
#include "enfle/fourcc.h"

typedef struct _mp3info {
  unsigned int first_frame_start;
} Mp3Info;

DECLARE_DEMULTIPLEXER_PLUGIN_METHODS;
DECLARE_DEMULTIPLEXER_METHODS;
PREPARE_DEMULTIPLEXER_TEMPLATE;

static DemultiplexerPlugin plugin = {
  .type = ENFLE_PLUGIN_DEMULTIPLEXER,
  .name = "MP3",
  .description = "MP3 Demultiplexer plugin version 0.1",
  .author = "Hiroshi Takekawa",

  .identify = identify,
  .examine = examine
};

ENFLE_PLUGIN_ENTRY(demultiplexer_mp3)
{
  DemultiplexerPlugin *dp;

  if ((dp = (DemultiplexerPlugin *)calloc(1, sizeof(DemultiplexerPlugin))) == NULL)
    return NULL;
  memcpy(dp, &plugin, sizeof(DemultiplexerPlugin));

  return (void *)dp;
}

ENFLE_PLUGIN_EXIT(demultiplexer_mp3, p)
{
  free(p);
}

/* demultiplexer plugin methods */

#define DEMULTIPLEXER_MP3_BUFFER_SIZE 65536
#define DEMULTIPLEXER_MP3_IDENTIFY_SIZE 4096
#define DEMULTIPLEXER_MP3_DETERMINE_SIZE 65536

static Demultiplexer *
create(void)
{
  Demultiplexer *demux;
  Mp3Info *info;

  if ((demux = _demultiplexer_create()) == NULL)
    return NULL;
  memcpy(demux, &template, sizeof(Demultiplexer));

  if ((info = calloc(1, sizeof(Mp3Info))) == NULL) {
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
  //Mp3Info *info = (Mp3Info *)demux->private_data;
  DemultiplexerStatus ds;

  demux->st = st;
  if ((ds = __examine(demux, 0)) != DEMULTIPLEX_OK) {
    destroy(demux);
    return NULL;
  }

  /* Dummy */
  m->width = 120;
  m->height = 80;
  rational_set_0(m->framerate);
  m->num_of_frames = 1;

  m->v_fourcc = FCC_NONE;
  m->a_fourcc = WAVEFORMAT_TAG_MP3;

  return demux;
}

static DemultiplexerStatus
__examine(Demultiplexer *demux, int identify_only)
{
  Mp3Info *info = (Mp3Info *)demux->private_data;
  DemultiplexerStatus ds;
  unsigned char *buf;
  int read_size, tag_size;

  debug_message("%s...\n", identify_only ? "identify" : "examine");

  demux->nastreams = 0;
  demux->nvstreams = 0;
  //demux->vstreams[0] = 0;
  demux->astreams[0] = 0;
  info->first_frame_start = 0;

  if ((buf = malloc(DEMULTIPLEXER_MP3_BUFFER_SIZE)) == NULL)
    return DEMULTIPLEX_ERROR;

  if ((read_size = stream_read(demux->st, buf, DEMULTIPLEXER_MP3_BUFFER_SIZE)) < 0) {
    err_message_fnc("stream_read error.\n");
    ds = DEMULTIPLEX_ERROR;
    goto quit;
  }
  if (read_size < 3) {
    ds = DEMULTIPLEX_NOT;
    goto quit;
  }

  if (memcmp(buf, "ID3", 3) != 0) {
    if (buf[0] == 0xff && (buf[1] & 0xe0) == 0xe0) {
      ds = DEMULTIPLEX_OK;
      goto quit;
    }
    ds = DEMULTIPLEX_NOT;
    goto quit;
  }
  if (read_size < 10) {
    ds = DEMULTIPLEX_NOT;
    goto quit;
  }

  debug_message_fnc("id3tag v2 found\n");
  tag_size = (buf[6] << 21) | (buf[7] << 14) | (buf[8] << 7) | buf[9];
  info->first_frame_start = tag_size + 10;
  if (info->first_frame_start + 4 > DEMULTIPLEXER_MP3_BUFFER_SIZE) {
    warning_fnc("Long tag detected.  Frame header check skipped.  Maybe this will not be harmful.\n");
    ds = DEMULTIPLEX_OK;
    goto quit;
  }

  ds = DEMULTIPLEX_NOT;
  if (buf[info->first_frame_start] == 0xff && (buf[info->first_frame_start + 1] & 0xe0) == 0xe0)
    ds = DEMULTIPLEX_OK;
 quit:
  if (ds == DEMULTIPLEX_OK)
    demux->nastreams = 1;
  if (identify_only)
    destroy(demux);
  free(buf);
  return ds;
}

static void *
demux_main(void *arg)
{
  Demultiplexer *demux = (Demultiplexer *)arg;
  //Mp3Info *info = (Mp3Info *)demux->private_data;
  DemuxedPacket *dp;
  unsigned char *buf;
  int read_size;

  if (demux->running)
    return (void *)0;

  if ((buf = malloc(DEMULTIPLEXER_MP3_BUFFER_SIZE)) == NULL)
    return (void *)0;

  demux->running = 1;
  do {
    if ((read_size = stream_read(demux->st, buf,
				 DEMULTIPLEXER_MP3_BUFFER_SIZE)) < 0) {
      show_message_fnc("read error.\n");
      goto error;
    }
    if (read_size == 0)
	break;

    dp = malloc(sizeof(DemuxedPacket));
    dp->pts = dp->dts = -1;
    dp->size = read_size;
    dp->data = malloc(dp->size);
    memcpy(dp->data, buf, dp->size);
    fifo_put(demux->astream, dp, demultiplexer_destroy_packet);
  } while (demux->running);

  demultiplexer_set_eof(demux, 1);
  demux->running = 0;
  free(buf);
  debug_message_fnc("exiting.\n");
  return (void *)1;

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

  debug_message_fn(" demultiplexer_mp3\n");

  pthread_create(&demux->thread, NULL, demux_main, demux);

  return 1;
}

static int
stop(Demultiplexer *demux)
{
  void *ret;

  debug_message_fn(" demultiplexer_mp3\n");

  demux->running = 0;
  if (demux->thread) {
    pthread_join(demux->thread, &ret);
    demux->thread = 0;
  }

  debug_message_fn(" demultiplexer_mp3 OK\n");

  return 1;
}

static int
demux_rewind(Demultiplexer *demux)
{
  Mp3Info *info = (Mp3Info *)demux->private_data;

  if (demux->running)
    return 0;
  return stream_seek(demux->st, info->first_frame_start, _SET);
}
