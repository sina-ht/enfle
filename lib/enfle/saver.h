/*
 * saver.h -- Saver header
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Jun 19 01:58:35 2001.
 * $Id: saver.h,v 1.2 2001/06/19 08:16:19 sian Exp $
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

#ifndef _SAVER_H
#define _SAVER_H

#include "enfle-plugins.h"
#include "utils/libconfig.h"
#include "image.h"

typedef struct _saver Saver;
struct _saver {
  int (*save)(EnflePlugins *, char *, Image *, FILE *, Config *, void *);
  char *(*get_ext)(EnflePlugins *, char *, Config *);
};

#define saver_save(s, eps, n, p, fp, c, d) (s)->save((eps), (n), (p), (fp), (c), (d))
#define saver_get_ext(s, eps, n, c) (s)->get_ext((eps), (n), (c))
#define saver_destroy(s) if ((s)) free((s))

Saver *saver_create(void);

#endif
