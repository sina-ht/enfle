/*
 * ui-plugin.h -- UI plugin interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Feb 10 00:03:33 2004.
 * $Id: ui-plugin.h,v 1.9 2004/02/14 05:28:08 sian Exp $
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

#ifndef _UI_PLUGIN_H
#define _UI_PLUGIN_H

#include "ui-extra.h"
#include "enfle-plugin.h"
#include "utils/libconfig.h"

typedef struct _ui_plugin {
  ENFLE_PLUGIN_COMMON_DATA;

  int (*ui_main)(UIData *);
} UIPlugin;

#ifndef STATIC
ENFLE_PLUGIN_ENTRIES;
#endif

#endif
