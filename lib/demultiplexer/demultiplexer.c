/*
 * demultiplexer.c -- Demultiplexer abstraction layer
 * (C)Copyright 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Feb 24 03:43:36 2001.
 * $Id: demultiplexer.c,v 1.2 2001/02/24 08:23:31 sian Exp $
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

#include "compat.h"
#include "common.h"

#include "demultiplexer.h"

Demultiplexer *
_demultiplexer_create(void)
{
  Demultiplexer *demux;

  if ((demux = calloc(1, sizeof(Demultiplexer))) == NULL)
    return NULL;
  pthread_mutex_init(&demux->io_mutex, NULL);

  return demux;
}

void
_demultiplexer_destroy(Demultiplexer *demux)
{
  pthread_mutex_destroy(&demux->io_mutex);
  free(demux);
}
