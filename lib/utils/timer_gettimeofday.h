/*
 * timer_gettimeofday.h -- timer gettimeofday() implementation header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Oct 16 03:24:00 2000.
 * $Id: timer_gettimeofday.h,v 1.1 2000/10/16 19:24:33 sian Exp $
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

#ifndef _TIMER_GETTIMEOFDAY_H
#define _TIMER_GETTIMEOFDAY_H

#include "timer_impl.h"

Timer_impl *timer_gettimeofday(void);

#endif
