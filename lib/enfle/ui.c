/*
 * ui.c -- UI plugin interface
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Sep 18 07:18:49 2000.
 * $Id: ui.c,v 1.1 2000/09/30 17:36:36 sian Exp $
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

#include "common.h"

#include "ui.h"

static char *load(UI *, Plugin *);
static int unload(UI *, char *);
static int call(UI *, char *, UIData *);
static void destroy(UI *);
static Dlist *get_names(UI *);
static unsigned char *get_description(UI *, char *);
static unsigned char *get_author(UI *, char *);

static UI ui_template = {
  pl: NULL,
  load: load,
  unload: unload,
  call: call,
  destroy: destroy,
  get_names: get_names,
  get_description: get_description,
  get_author: get_author
};

UI *
ui_create(void)
{
  UI *ui;

  if ((ui = (UI *)calloc(1, sizeof(UI))) == NULL)
    return NULL;
  memcpy(ui, &ui_template, sizeof(UI));

  if ((ui->pl = pluginlist_create()) == NULL) {
    free(ui);
    return NULL;
  }

  return ui;
}

static char *
load(UI *ui, Plugin *p)
{
  PluginList *pl = ui->pl;
  UIPlugin *uip;

  uip = plugin_get(p);

  if (!pluginlist_add(pl, p, uip->name)) {
    plugin_unload(p);
    return NULL;
  }

  return uip->name;
}

static int
unload(UI *ui, char *pluginname)
{
  Plugin *p;

  if ((p = pluginlist_get(ui->pl, pluginname)) == NULL)
    return 0;
  plugin_unload(p);

  return pluginlist_delete(ui->pl, pluginname);
}

static int
call(UI *ui, char *pluginname, UIData *uidata)
{
  Plugin *p;
  UIPlugin *uip;

  if ((p = pluginlist_get(ui->pl, pluginname)) == NULL)
    return 0;
  uip = plugin_get(p);

  return uip->ui_main(uidata);
}

static void
destroy(UI *ui)
{
  pluginlist_destroy(ui->pl);
  free(ui);
}

static Dlist *
get_names(UI *ui)
{
  return pluginlist_get_names(ui->pl);
}

static unsigned char *
get_description(UI *ui, char *pluginname)
{
  Plugin *p;
  UIPlugin *uip;

  if ((p = pluginlist_get(ui->pl, pluginname)) == NULL)
    return NULL;
  uip = plugin_get(p);

  return uip->description;
}

static unsigned char *
get_author(UI *ui, char *pluginname)
{
  Plugin *p;
  UIPlugin *uip;

  if ((p = pluginlist_get(ui->pl, pluginname)) == NULL)
    return NULL;
  uip = plugin_get(p);

  return uip->author;
}
