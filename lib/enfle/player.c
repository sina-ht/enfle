/*
 * player.c -- player plugin interface
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Oct  9 02:04:20 2000.
 * $Id: player.c,v 1.1 2000/10/08 17:38:15 sian Exp $
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

#include <stdio.h>
#include <stdlib.h>

#include "common.h"

#include "stream.h"
#include "player.h"

static char *load(Player *, Plugin *);
static int unload(Player *, char *);
static int identify(Player *, Image *, Stream *);
static PlayerStatus play(Player *, char *, Image *, Stream *);
static void destroy(Player *);
static Dlist *get_names(Player *);
static unsigned char *get_description(Player *, char *);
static unsigned char *get_author(Player *, char *);

static Player player_template = {
  pl: NULL,
  load: load,
  unload: unload,
  identify: identify,
  play: play,
  destroy: destroy,
  get_names: get_names,
  get_description: get_description,
  get_author: get_author
};

Player *
player_create(void)
{
  Player *l;

  if ((l = (Player *)calloc(1, sizeof(Player))) == NULL)
    return NULL;
  memcpy(l, &player_template, sizeof(Player));

  if ((l->pl = pluginlist_create()) == NULL) {
    free(l);
    return NULL;
  }

  return l;
}

/* methods */

static char *
load(Player *l, Plugin *p)
{
  PluginList *pl = l->pl;
  PlayerPlugin *lp;

  lp = plugin_get(p);

  if (!pluginlist_add(pl, p, lp->name)) {
    plugin_unload(p);
    return NULL;
  }

  return lp->name;
}

static int
unload(Player *l, char *pluginname)
{
  Plugin *p;

  if ((p = pluginlist_get(l->pl, pluginname)) == NULL)
    return 0;
  plugin_unload(p);

  return pluginlist_delete(l->pl, pluginname);
}

static int
identify(Player *l, Image *ip, Stream *st)
{
  Dlist *dl;
  Dlist_data *dd;

  dl = pluginlist_list(l->pl);
  dlist_iter(dl, dd) {
    Plugin *p;
    PlayerPlugin *lp;
    char *pluginname;

    pluginname = dlist_data(dd);
    if ((p = pluginlist_get(l->pl, pluginname)) == NULL) {
      fprintf(stderr, "BUG: %s player plugin not found but in list.\n", pluginname);
      exit(-1);
    }
    lp = plugin_get(p);

    stream_rewind(st);
    if (lp->identify(ip, st) == PLAY_OK) {
      ip->format = pluginname;
      dlist_move_to_top(dl, dd);
      return 1;
    }
  }

  return 0;
}

static PlayerStatus
play(Player *l, char *pluginname, Image *ip, Stream *st)
{
  Plugin *p;
  PlayerPlugin *lp;

  if ((p = pluginlist_get(l->pl, pluginname)) == NULL)
    return 0;
  lp = plugin_get(p);

  stream_rewind(st);
  return lp->play(ip, st);
}

static void
destroy(Player *l)
{
  pluginlist_destroy(l->pl);
  free(l);
}

static Dlist *
get_names(Player *l)
{
  return pluginlist_get_names(l->pl);
}

static unsigned char *
get_description(Player *l, char *pluginname)
{
  Plugin *p;
  PlayerPlugin *lp;

  if ((p = pluginlist_get(l->pl, pluginname)) == NULL)
    return NULL;
  lp = plugin_get(p);

  return lp->description;
}

static unsigned char *
get_author(Player *l, char *pluginname)
{
  Plugin *p;
  PlayerPlugin *lp;

  if ((p = pluginlist_get(l->pl, pluginname)) == NULL)
    return NULL;
  lp = plugin_get(p);

  return lp->author;
}
