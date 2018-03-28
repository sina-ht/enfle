/*
 * borlndmm.c -- implementation of routines in borlndmm.dll
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Feb 18 01:39:44 2002.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include "compat.h"
#include "common.h"

#include "w32api.h"
#include "module.h"

#include "borlndmm.h"

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
