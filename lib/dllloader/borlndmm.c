/*
 * borlndmm.c -- implementation of routines in borlndmm.dll
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Jan  6 01:38:01 2001.
 * $Id: borlndmm.c,v 1.2 2001/01/06 23:52:52 sian Exp $
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

#include "w32api.h"
#include "module.h"

#include "borlndmm.h"

#include "common.h"

static void unknown_symbol(void);

static Symbol_info symbol_infos[] = {
  { NULL, unknown_symbol }
};

/* unimplemened */

static void
unknown_symbol(void)
{
  show_message("unknown symbol in borlndmm called\n");
}

/* export */

Symbol_info *
borlndmm_get_export_symbols(void)
{
  module_register("borlndmm.dll", symbol_infos);
  return symbol_infos;
}
