/*
 * demultiplexer_old.c -- Demultiplexer abstraction layer
 * (C)Copyright 2001-2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Feb 13 00:08:03 2004.
 *
 * SPDX-License-Identifier: GPL-2.0-only
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
