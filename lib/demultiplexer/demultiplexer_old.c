/*
 * demultiplexer_old.c -- Demultiplexer abstraction layer
 * (C)Copyright 2001-2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Feb 13 00:08:03 2004.
 * $Id: demultiplexer_old.c,v 1.1 2004/02/14 05:09:32 sian Exp $
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

#include "compat.h"
#include "common.h"

#include "demultiplexer_old.h"

Demultiplexer_old *
_demultiplexer_old_create(void)
{
  Demultiplexer_old *demux;

  if ((demux = calloc(1, sizeof(Demultiplexer_old))) == NULL)
    return NULL;

  return demux;
}

void
_demultiplexer_old_destroy(Demultiplexer_old *demux)
{
  free(demux);
}

void
demultiplexer_old_destroy_packet(void *d)
{
  DemuxedPacket_old *dp = (DemuxedPacket_old *)d;

  if (dp) {
    if (dp->data)
      free(dp->data);
    free(dp);
  }
}
