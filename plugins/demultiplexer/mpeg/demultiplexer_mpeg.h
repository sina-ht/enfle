/*
 * demultiplexer_mpeg.h -- MPEG stream demultiplexer header
 * (C)Copyright 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Jun  5 22:24:42 2006.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _DEMULTIPLEXER_MPEG_H
#define _DEMULTIPLEXER_MPEG_H

#include "enfle/demultiplexer.h"
#include "enfle/stream.h"

typedef struct _mpeg_info {
  int ver;
  int has_valid_pts;
  unsigned long prev_pts;
} MpegInfo;

Demultiplexer *demultiplexer_mpeg_create(void);

#endif
