/*
 * archiver.h -- archiver header
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Feb  9 12:26:15 2002.
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

#ifndef _ARCHIVER_H
#define _ARCHIVER_H

#include "enfle-plugins.h"
#include "archiver-extra.h"
#include "archive.h"
#include "stream.h"
#include "utils/libconfig.h"

int archiver_identify(EnflePlugins *, Archive *, Stream *, Config *);
ArchiverStatus archiver_open(EnflePlugins *, Archive *, char *, Stream *);

#endif
