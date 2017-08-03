/*
 * streamer.h -- streamer header
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Feb  9 12:28:30 2002.
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

#ifndef _STREAMER_H
#define _STREAMER_H

#include "stream.h"
#include "utils/libconfig.h"
#include "enfle-plugins.h"

int streamer_identify(EnflePlugins *, Stream *, char *, Config *);
int streamer_open(EnflePlugins *, Stream *, char *, char *);

#endif
