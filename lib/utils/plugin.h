/*
 * plugin.h -- plugin interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Nov 11 06:00:40 2000.
 * $Id: plugin.h,v 1.2 2000/11/14 00:54:45 sian Exp $
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

#ifndef _PLUGIN_H
#define _PLUGIN_H

typedef struct _plugin Plugin;
struct _plugin {
  void *handle;
  void *substance;
  void (*substance_unload)(void *);
  char *filepath;
  char *err;

  int (*load)(Plugin *, char *, const char *, const char *);
  int (*unload)(Plugin *);
  void *(*get)(Plugin *);
  void (*destroy)(Plugin *);
};

#define plugin_load(p, n, en, ex) (p)->load((p), n, en, ex)
#define plugin_unload(p) (p)->unload((p))
#define plugin_get(p) (p)->get((p))
#define plugin_destroy(p) (p)->destroy(p)

Plugin *plugin_create(void);

#endif
