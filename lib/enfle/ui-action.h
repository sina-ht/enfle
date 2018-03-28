/*
 * ui-action.h -- UI plugin action structure
 * (C)Copyright 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Jul  7 22:50:45 2002.
 *
 * SPDX-License-Identifier: GPL-2.0-only
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

#define UI_ACTION_END { NULL, NULL, NULL, ENFLE_KEY_Empty, ENFLE_MOD_None, ENFLE_Button_None, 0, 0 }

#endif
