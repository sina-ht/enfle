/*
 * demultiplexer.c -- Demultiplexer abstraction layer
 * (C)Copyright 2001-2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Jan 18 13:56:37 2004.
 * $Id: demultiplexer.c,v 1.3 2004/01/18 07:13:01 sian Exp $
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

  return demux;
}

void
_demultiplexer_destroy(Demultiplexer *demux)
{
  free(demux);
}
