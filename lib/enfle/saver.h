/*
 * saver.h -- Saver header
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Jul  3 20:19:46 2001.
 * $Id: saver.h,v 1.3 2001/07/10 12:59:45 sian Exp $
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _SAVER_H
#define _SAVER_H

#include "enfle-plugins.h"
#include "utils/libconfig.h"
#include "image.h"

int saver_save(EnflePlugins *, char *, Image *, FILE *, Config *, void *);
char *saver_get_ext(EnflePlugins *, char *, Config *);

#endif
