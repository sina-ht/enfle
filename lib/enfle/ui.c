/*
 * ui.c -- UI plugin interface
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Jun 19 01:32:13 2001.
 * $Id: ui.c,v 1.4 2001/06/19 08:16:19 sian Exp $
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

#include <stdlib.h>

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "ui.h"
#include "ui-plugin.h"

static int call(EnflePlugins *, char *, UIData *);

static UI template = {
  call: call
};

UI *
ui_create(void)
{
  UI *ui;

  if ((ui = (UI *)calloc(1, sizeof(UI))) == NULL)
    return NULL;
  memcpy(ui, &template, sizeof(UI));

  return ui;
}

static int
call(EnflePlugins *eps, char *pluginname, UIData *uidata)
{
  Plugin *p;
  UIPlugin *uip;

  if ((p = pluginlist_get(eps->pls[ENFLE_PLUGIN_UI], pluginname)) == NULL)
    return 0;
  uip = plugin_get(p);
  if (uip)
    return uip->ui_main(uidata);
  return 0;
}
