/*
 * ui-extra.h -- UI plugin extra definition
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Oct 17 22:45:33 2000.
 * $Id: ui-extra.h,v 1.1 2000/10/17 14:04:01 sian Exp $
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

#ifndef _UI_EXTRA_H
#define _UI_EXTRA_H

#include "libconfig.h"
#include "enfle-plugins.h"
#include "streamer.h"
#include "loader.h"
#include "archiver.h"
#include "player.h"
#include "archive.h"

struct _ui_screen {
  unsigned int width, height;
  int depth, bits_per_pixel;
  void *private;
};

struct _ui_data {
  Config *c;
  EnflePlugins *eps;
  Streamer *st;
  Loader *ld;
  Archiver *ar;
  Player *player;
  Archive *a;
  UIScreen *screen;
  void *private;
};

#endif
