/*
 * x11ximage.h -- X11 XImage related functions header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file if part of Enfle.
 *
 * Last Modified: Wed Sep 20 00:47:30 2000.
 * $Id: x11ximage.h,v 1.1 2000/09/30 17:36:36 sian Exp $
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

#ifndef _X11XIMAGE_H
#define _X11XIMAGE_H

#include "image.h"

void x11ximage_convert_image(XImage *, Image *);

#endif
