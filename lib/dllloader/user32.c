/*
 * user32.c -- implementation of routines in user32.dll
 */

#include <stdarg.h>

#include "w32api.h"
#include "module.h"

#include "user32.h"

#include "common.h"

DECLARE_W32API(void, unknown_symbol, ());
DECLARE_W32API(BOOL, EnumThreadWindows, (DWORD, WNDENUMPROC, LPARAM));
DECLARE_W32API(INT, MessageBoxA, (HWND, LPCSTR, LPCSTR, UINT));
DECLARE_W32API(INT, LoadStringA, (HINSTANCE, UINT, LPSTR, INT));
DECLARE_W32API(INT, GetSystemMetrics, (INT));
DECLARE_W32API(INT, GetKeyboardType, (INT));
DECLARE_W32API(INT, wsprintfA, (LPSTR, LPCSTR, ...));

static Symbol_info symbol_infos[] = {
  { "EnumThreadWindows", EnumThreadWindows },
  { "MessageBoxA", MessageBoxA },
  { "LoadStringA", LoadStringA },
  { "GetSystemMetrics", GetSystemMetrics },
  { "GetKeyboardType", GetKeyboardType },
  { "wsprintfA", wsprintfA },
  { NULL, unknown_symbol }
};

DEFINE_W32API(BOOL, EnumThreadWindows, (DWORD id, WNDENUMPROC func, LPARAM param))
{
  debug_message("EnumThreadWindows(%d, %p) called\n", id, func);
  return TRUE;
}

DEFINE_W32API(INT, MessageBoxA, (HWND handle, LPCSTR text, LPCSTR title, UINT type))
{
  debug_message("MessageBoxA(%s: %s) called\n", title, text);
  return 1;
}

DEFINE_W32API(INT, LoadStringA, (HINSTANCE handle, UINT id, LPSTR buffer, INT len))
{
  debug_message("LoadStringA(%p, %d, %p, %d) called\n", handle, id, buffer, len);
  return 0;
}

DEFINE_W32API(INT, GetSystemMetrics, (INT index))
{
  debug_message("GetSystemMetrics(%d) called\n", index);
  return 0;
}

DEFINE_W32API(INT, GetKeyboardType, (INT type))
{
  debug_message("GetKeyboardType(%d) called\n", type);

  switch (type) {
  case 0:
    return 4;
  case 1:
    return 0;
  case 2:
    return 12;
  default:
    return 0;
  }
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
