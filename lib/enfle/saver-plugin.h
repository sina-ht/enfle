/*
 * saver-plugin.h -- Saver plugin interface header
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Apr 13 21:28:46 2001.
 * $Id: saver-plugin.h,v 1.1 2001/04/18 05:12:50 sian Exp $
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

#ifndef _SAVER_PLUGIN_H
#define _SAVER_PLUGIN_H

#include "enfle-plugin.h"
#include "utils/libconfig.h"
#include "image.h"

typedef struct _saver_plugin {
  ENFLE_PLUGIN_COMMON_DATA;

  char *(*get_ext)(Config *);
  int (*save)(Image *, FILE *, void *);
} SaverPlugin;

#define DECLARE_SAVER_PLUGIN_METHODS \
 static char *get_ext(Config *); \
 static int save(Image *, FILE *, void *)

#define DEFINE_SAVER_PLUGIN_GET_EXT(c) \
 static char * \
 get_ext(Config * ## c ##)
#define DEFINE_SAVER_PLUGIN_SAVE(p, fp, params) \
 static int \
 save(Image * ## p ##, FILE * ## fp ##, void * ## params ##)

ENFLE_PLUGIN_ENTRIES;

#endif
