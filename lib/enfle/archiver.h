/*
 * archiver.h -- archiver header
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Jul  3 20:23:08 2001.
 * $Id: archiver.h,v 1.3 2001/07/10 12:59:45 sian Exp $
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

#ifndef _ARCHIVER_H
#define _ARCHIVER_H

#include "enfle-plugins.h"
#include "archiver-extra.h"
#include "archive.h"
#include "stream.h"

int archiver_identify(EnflePlugins *, Archive *, Stream *);
ArchiverStatus archiver_open(EnflePlugins *, Archive *, char *, Stream *);

#endif
