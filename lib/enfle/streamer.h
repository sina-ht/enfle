/*
 * streamer.h -- streamer header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Oct 17 22:52:30 2000.
 * $Id: streamer.h,v 1.2 2000/10/17 14:04:01 sian Exp $
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

#ifndef _STREAMER_H
#define _STREAMER_H

#include "stream.h"
#include "enfle-plugins.h"

typedef struct _streamer Streamer;
struct _streamer {
  int (*identify)(EnflePlugins *, Stream *, char *);
  int (*open)(EnflePlugins *, Stream *, char *, char *);
};

#define streamer_identify(st, eps, s, path) (st)->identify((eps), (s), (path))
#define streamer_open(st, eps, s, n, path) (st)->open((eps), (s), (n), (path))
#define streamer_destroy(st) if ((st)) free((st))

Streamer *streamer_create(void);

#endif
