/*
 * demultiplexer.h -- Demultiplexer abstraction layer header
 * (C)Copyright 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Sep 18 14:06:09 2001.
 * $Id: demultiplexer.h,v 1.7 2001/09/18 05:22:24 sian Exp $
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

#ifndef _DEMULTIPLEXER_H
#define _DEMULTIPLEXER_H

#include <pthread.h>

typedef struct _demultiplexer Demultiplexer;
struct _demultiplexer {
  pthread_t thread;
  pthread_mutex_t io_mutex;
  int running;
  int eof;
  void *private_data;

  int (*examine)(Demultiplexer *);
  int (*start)(Demultiplexer *);
  int (*stop)(Demultiplexer *);
  int (*demux_rewind)(Demultiplexer *);
  void (*destroy)(Demultiplexer *);
};

#define DECLARE_DEMULTIPLEXER_METHODS \
  static int examine(Demultiplexer *); \
  static int start(Demultiplexer *); \
  static int stop(Demultiplexer *); \
  static int demux_rewind(Demultiplexer *); \
  static void destroy(Demultiplexer *)

#define PREPARE_DEMULTIPLEXER_TEMPLATE \
  static Demultiplexer template = { \
    examine: examine, \
    start: start, \
    stop: stop, \
    demux_rewind: demux_rewind, \
    destroy: destroy \
  }

/* protected */
Demultiplexer *_demultiplexer_create(void);
void _demultiplexer_destroy(Demultiplexer *);

#define demultiplexer_set_eof(de, f) (de)->eof = (f)
#define demultiplexer_get_eof(de) (de)->eof

#define demultiplexer_io_lock(de) pthread_mutex_lock(&((de)->io_mutex))
#define demultiplexer_io_unlock(de) pthread_mutex_unlock(&((de)->io_mutex))
#define demultiplexer_examine(de) (de)->examine((de))
#define demultiplexer_start(de) (de)->start((de))
#define demultiplexer_stop(de) (de)->stop((de))
#define demultiplexer_rewind(de) (de)->demux_rewind((de))
#define demultiplexer_destroy(de) (de)->destroy((de))

#endif
