/*
 * ui-extra.h -- UI plugin extra definition
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Oct 21 02:46:03 2000.
 * $Id: ui-extra.h,v 1.2 2000/10/20 18:13:39 sian Exp $
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

typedef struct _ui_data UIData;

#include "libconfig.h"
#include "enfle-plugins.h"
#include "streamer.h"
#include "loader.h"
#include "archiver.h"
#include "player.h"
#include "archive.h"
#include "video.h"
#include "video-plugin.h"

struct _ui_data {
  Config *c;
  EnflePlugins *eps;
  VideoPlugin *vp;
  VideoWindow *vw;
  Streamer *st;
  Loader *ld;
  Archiver *ar;
  Player *player;
  Archive *a;
  void *private;
};

#endif
