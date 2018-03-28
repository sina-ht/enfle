/*
 * ui.c -- UI plugin interface
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Feb 18 01:39:58 2002.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include "compat.h"
#include "common.h"

#include "ui.h"
#include "ui-plugin.h"

int
ui_call(EnflePlugins *eps, char *pluginname, UIData *uidata)
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
