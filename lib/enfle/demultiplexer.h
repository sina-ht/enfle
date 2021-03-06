/*
 * demultiplexer.h -- Demultiplexer abstraction layer header
 * (C)Copyright 2001-2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat May  1 17:49:37 2004.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _DEMULTIPLEXER_H
#define _DEMULTIPLEXER_H

#include <pthread.h>

typedef struct _demultiplexer Demultiplexer;

#include "enfle-plugins.h"
#include "stream.h"
#include "utils/libconfig.h"
#include "utils/fifo.h"

typedef enum {
  DEMULTIPLEX_ERROR = -1,
  DEMULTIPLEX_NOT,
  DEMULTIPLEX_OK
} DemultiplexerStatus;

#define MAX_VSTREAMS 4
#define MAX_ASTREAMS 8
struct _demultiplexer {
  Stream *st;
  FIFO *vstream;
  FIFO *astream;
  int nvstreams;
  int nastreams;
  int nvstream;
  int nastream;
  int vstreams[MAX_VSTREAMS];
  int astreams[MAX_ASTREAMS];
  pthread_t thread;
  int running;
  int eof;
  void *private_data;

  int (*start)(Demultiplexer *);
  int (*stop)(Demultiplexer *);
  int (*demux_rewind)(Demultiplexer *);
  void (*destroy)(Demultiplexer *);
};

typedef struct _demuxed_packet {
  int is_key;
  unsigned long ts_base;
  unsigned long pts, dts;
  unsigned int size;
  void *data;
} DemuxedPacket;

#define DECLARE_DEMULTIPLEXER_METHODS \
  static int start(Demultiplexer *); \
  static int stop(Demultiplexer *); \
  static int demux_rewind(Demultiplexer *); \
  static void destroy(Demultiplexer *)

#define PREPARE_DEMULTIPLEXER_TEMPLATE \
  static Demultiplexer template = { \
    .start = start, \
    .stop = stop, \
    .demux_rewind = demux_rewind, \
    .destroy = destroy \
  }

#define demultiplexer_set_eof(de, f) (de)->eof = (f)
#define demultiplexer_get_eof(de) (de)->eof

#define demultiplexer_start(de) (de)->start((de))
#define demultiplexer_stop(de) (de)->stop((de))
#define demultiplexer_rewind(de) (de)->demux_rewind((de))
#define demultiplexer_destroy(de) (de)->destroy((de))

#define demultiplexer_set_input(de, st) (de)->st = (st)
#define demultiplexer_set_vst(de, v) (de)->vstream = (v)
#define demultiplexer_set_ast(de, a) (de)->astream = (a)
#define demultiplexer_nvideos(de) (de)->nvstreams
#define demultiplexer_naudios(de) (de)->nastreams
#define demultiplexer_set_video(de, nv) (de)->nvstream = (nv)
#define demultiplexer_set_audio(de, na) (de)->nastream = (na)

/* protected */
Demultiplexer *_demultiplexer_create(void);
void _demultiplexer_destroy(Demultiplexer *);

#include "movie.h"

int demultiplexer_identify(EnflePlugins *, Movie *, Stream *, Config *);
Demultiplexer *demultiplexer_examine(EnflePlugins *, char *, Movie *, Stream *, Config *);
void demultiplexer_destroy_packet(void *);


#endif
