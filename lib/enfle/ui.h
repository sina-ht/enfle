/*
 * ui.h -- UI header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Sep 18 07:18:58 2000.
 * $Id: ui.h,v 1.1 2000/09/30 17:36:36 sian Exp $
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

#ifndef _UI_H
#define _UI_H

#include "pluginlist.h"
#include "ui-plugin.h"

typedef struct _ui UI;
struct _ui {
  PluginList *pl;

  char *(*load)(UI *, Plugin *);
  int (*unload)(UI *, char *);
  int (*call)(UI *, char *, UIData *);
  void (*destroy)(UI *);
  Dlist *(*get_names)(UI *);
  unsigned char *(*get_description)(UI *, char *);
  unsigned char *(*get_author)(UI *, char *);
};

#define ui_load(ui, p) (ui)->load((ui), (p))
#define ui_unload(ui, n) (ui)->unload((ui), (n))
#define ui_call(ui, n, d) (ui)->call((ui), (n), (d))
#define ui_destroy(ui) (ui)->destroy((ui))
#define ui_get_names(ui) (ui)->get_names((ui))
#define ui_get_description(ui, n) (ui)->get_description((ui), (n))
#define ui_get_author(ui, n) (ui)->get_author((ui), (n))

UI *ui_create(void);

#endif
