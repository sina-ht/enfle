/*
 * player.h -- player plugin interface header
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Oct 12 23:35:19 2001.
 * $Id: player.h,v 1.10 2001/10/14 12:32:17 sian Exp $
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

#ifndef _PLAYER_H
#define _PLAYER_H

#include "enfle-plugins.h"
#include "stream.h"
#include "movie.h"
#include "video.h"

int player_identify(EnflePlugins *, Movie *, Stream *, Config *);
PlayerStatus player_load(EnflePlugins *, VideoWindow *, char *, Movie *, Stream *, Config *);

#endif
