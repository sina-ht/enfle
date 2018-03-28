/*
 * demultiplexer_old.h -- Demultiplexer abstraction layer header
 * (C)Copyright 2001-2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Feb 13 00:07:50 2004.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _DEMULTIPLEXER_OLD_H
#define _DEMULTIPLEXER_OLD_H

#include <pthread.h>

typedef struct _demultiplexer_old Demultiplexer_old;
struct _demultiplexer_old {
  pthread_t thread;
  int running;
  int eof;
  void *private_data;

  int (*examine)(Demultiplexer_old *);
  int (*start)(Demultiplexer_old *);
  int (*stop)(Demultiplexer_old *);
  int (*demux_rewind)(Demultiplexer_old *);
  void (*destroy)(Demultiplexer_old *);
};

typedef struct _demuxed_packet_old {
  int pts_dts_flag;
  unsigned long pts, dts;
  unsigned int size;
  void *data;
} DemuxedPacket_old;

#define DECLARE_DEMULTIPLEXER_OLD_METHODS \
  static int examine(Demultiplexer_old *); \
  static int start(Demultiplexer_old *); \
  static int stop(Demultiplexer_old *); \
  static int demux_rewind(Demultiplexer_old *); \
  static void destroy(Demultiplexer_old *)

#define PREPARE_DEMULTIPLEXER_OLD_TEMPLATE \
  static Demultiplexer_old template = { \
    examine: examine, \
    start: start, \
    stop: stop, \
    demux_rewind: demux_rewind, \
    destroy: destroy \
  }

/* protected */
Demultiplexer_old *_demultiplexer_old_create(void);
void _demultiplexer_old_destroy(Demultiplexer_old *);

void demultiplexer_old_destroy_packet(void *);

#define demultiplexer_old_set_eof(de, f) (de)->eof = (f)
#define demultiplexer_old_get_eof(de) (de)->eof

#define demultiplexer_old_examine(de) (de)->examine((de))
#define demultiplexer_old_start(de) (de)->start((de))
#define demultiplexer_old_stop(de) (de)->stop((de))
#define demultiplexer_old_rewind(de) (de)->demux_rewind((de))
#define demultiplexer_old_destroy(de) (de)->destroy((de))

#endif
