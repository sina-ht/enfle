/*
 * user32.c -- implementation of routines in user32.dll
 */

#include <stdarg.h>

#include "w32api.h"
#include "module.h"

#include "user32.h"

#include "common.h"

DECLARE_W32API(void, unknown_symbol, ());
DECLARE_W32API(INT, MessageBoxA, (HWND, LPCSTR, LPCSTR, UINT));
DECLARE_W32API(INT, wsprintfA, (LPSTR, LPCSTR, ...));

static Symbol_info symbol_infos[] = {
  { "MessageBoxA", MessageBoxA },
  { "wsprintfA", wsprintfA },
  { NULL, unknown_symbol }
};

DEFINE_W32API(INT, MessageBoxA, (HWND handle, LPCSTR text, LPCSTR title, UINT type))
{
  debug_message("MessageBoxA(%s: %s)\n", title, text);
  return 1;
}

DEFINE_W32API(INT, wsprintfA, (LPSTR buf, LPCSTR format, ...))
{
  va_list va;

  va_start(va, format);
  vsnprintf(buf, 1024, format, va);
  va_end(va);

  debug_message("wsprintfA: formatted: %s", buf);

  return strlen(buf);
}

/* unimplemened */

DEFINE_W32API(void, unknown_symbol, ())
{
  show_message("unknown symbol in user32 called\n");
}

/* export */

Symbol_info *
user32_get_export_symbols(void)
{
  module_register("user32.dll", symbol_infos);
  return symbol_infos;
}
