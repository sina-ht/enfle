/*
 * archiver.h -- archiver header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Oct 17 22:52:10 2000.
 * $Id: archiver.h,v 1.2 2000/10/17 14:04:01 sian Exp $
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

#ifndef _ARCHIVER_H
#define _ARCHIVER_H

#include "enfle-plugins.h"
#include "archiver-extra.h"
#include "archive.h"
#include "stream.h"

typedef struct _archiver Archiver;
struct _archiver {
  int (*identify)(EnflePlugins *, Archive *, Stream *);
  ArchiverStatus (*open)(EnflePlugins *, Archive *, char *, Stream *);
};

#define archiver_identify(ar, eps, a, s) (ar)->identify((eps), (a), (s))
#define archiver_open(ar, eps, a, n, s) (ar)->open((eps), (a), (n), (s))
#define archiver_destroy(ar) if ((ar)) free((ar))

Archiver *archiver_create(void);

#endif
