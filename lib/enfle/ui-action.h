/*
 * ui-action.h -- UI plugin action structure
 * (C)Copyright 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Jul  7 22:50:45 2002.
 * $Id: ui-action.h,v 1.1 2002/08/03 04:54:45 sian Exp $
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

#ifndef _UI_ACTION_H
#define _UI_ACTION_H

#include "video.h"

typedef struct _ui_action {
  const char *name;
  int (*func)(void *);
  void *arg;
  VideoKey key;
  VideoModifierKey mod;
  VideoButton button;
  int group_id;
  int id;
} UIAction;

#define UI_ACTION_END { NULL, NULL, NULL, ENFLE_KEY_Empty, ENFLE_MOD_None, ENFLE_Button_None }

#endif