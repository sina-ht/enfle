/*
 * user32.c -- implementation of routines in user32.dll
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Sep 21 19:36:37 2001.
 * $Id: user32.c,v 1.7 2001/09/21 11:51:54 sian Exp $
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

#include <stdarg.h>

#include "w32api.h"
#include "module.h"

#include "user32.h"

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

DECLARE_W32API(void, unknown_symbol, (void));
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

  if (buffer && len > 0)
    *buffer = 0;

  return 0;
}

DEFINE_W32API(INT, GetSystemMetrics, (INT i))
{
  debug_message("GetSystemMetrics(%d) called\n", i);
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

DEFINE_W32API(void, unknown_symbol, (void))
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
