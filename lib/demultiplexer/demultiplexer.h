/*
 * demultiplexer.h -- Demultiplexer abstraction layer header
 * (C)Copyright 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Feb 20 21:52:51 2001.
 * $Id: demultiplexer.h,v 1.1 2001/02/20 13:54:20 sian Exp $
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
  int (*examine)(Demultiplexer *);
  int (*start)(Demultiplexer *);
  int (*stop)(Demultiplexer *);
  void (*destroy)(Demultiplexer *);

  pthread_t thread;
  int running;
  void *private_data;
};

#define DECLARE_DEMULTIPLEXER_METHODS \
  static int examine(Demultiplexer *); \
  static int start(Demultiplexer *); \
  static int stop(Demultiplexer *); \
  static void destroy(Demultiplexer *)

#define PREPARE_DEMULTIPLEXER_TEMPLATE \
  static Demultiplexer template = { \
    examine: examine, \
    start: start, \
    stop: stop, \
    destroy: destroy \
  }

Demultiplexer *demultiplexer_create(void);

#define demultiplex_examine(de) (de)->examine((de))
#define demultiplex_start(de) (de)->start((de))
#define demultiplex_stop(de) (de)->stop((de))
#define demultiplex_destroy(de) (de)->destroy((de))

#endif
