/*
 * borlndmm.c -- implementation of routines in borlndmm.dll
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
