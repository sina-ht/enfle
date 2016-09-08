/*
 * user32.c -- implementation of routines in user32.dll
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Thu Apr 15 00:21:11 2004.
 * $Id: user32.c,v 1.12 2004/04/18 06:25:12 sian Exp $
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

#include <stdlib.h>
#include <stdarg.h>

#include "w32api.h"
#include "module.h"

#include "utils/converter.h"
#include "pe_image.h"
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
DECLARE_W32API(INT, GetDesktopWindow, (void));
DECLARE_W32API(INT, GetDC, (INT));
DECLARE_W32API(INT, ReleaseDC, (INT, INT));
DECLARE_W32API(INT, IsRectEmpty, (CONST RECT *));

static Symbol_info symbol_infos[] = {
  { "EnumThreadWindows", EnumThreadWindows },
  { "MessageBoxA", MessageBoxA },
  { "LoadStringA", LoadStringA },
  { "GetSystemMetrics", GetSystemMetrics },
  { "GetKeyboardType", GetKeyboardType },
  { "wsprintfA", wsprintfA },
  { "GetDesktopWindow", GetDesktopWindow },
  { "GetDC", GetDC },
  { "ReleaseDC", ReleaseDC },
  { "IsRectEmpty", IsRectEmpty },
  { NULL, unknown_symbol }
};

DEFINE_W32API(BOOL, EnumThreadWindows, (DWORD id, WNDENUMPROC func, LPARAM param))
{
  debug_message_fn("(%d, %p) called\n", id, func);
  return TRUE;
}

DEFINE_W32API(INT, MessageBoxA, (HWND handle, LPCSTR text, LPCSTR title, UINT type))
{
  debug_message_fn("(%s: %s) called\n", title, text);
  return 1;
}

DEFINE_W32API(INT, LoadStringA, (HINSTANCE handle, UINT id, LPSTR buffer, INT len))
{
  PE_image *p = (PE_image *)handle;
  unsigned int rid = ((id >> 4) & 0xffff) + 1;
  unsigned int sid = id & 0xf;
  //unsigned int size;
  char buf[256];
  void *d;

  debug_message_fn("(%p, %d(rid: %d, sid: %d), %p, %d) called\n", handle, id, rid, sid, buffer, len);

  if (len == 0 || buffer == NULL)
    return 0;

  memset(buffer, 0, len);
  
  snprintf(buf, 256, "/0x6/0x%x/0x%x", rid, 0);
  if ((d = hash_lookup_str(p->resource, buf)) == NULL) {
    /* Japanese */
    snprintf(buf, 256, "/0x6/0x%x/0x%x", rid, 0x411);
    d = hash_lookup_str(p->resource, buf);
  }

  if (d) {
    unsigned char *data = d;
    char *s, *dest;
    unsigned int i;
    int l, L;

    //size = (((((data[0] << 8) | data[1]) << 8) | data[2]) << 8) | data[3];
    data += 4;
    for (i = 0; i < sid; i++)
      data += (*data + 1) << 1;
    l = *data << 1;
    s = calloc(1, l + 2);
    memcpy(s, data + 2, l);
    L = converter_convert(s, &dest, l, (char *)"UCS-2LE", (char *)"EUC-JP");
    strncpy(buffer, dest, len);
    free(dest);
  }

  return strlen(buffer);
}

DEFINE_W32API(INT, GetSystemMetrics, (INT i))
{
  debug_message_fn("(%d) called\n", i);
  return 0;
}

DEFINE_W32API(INT, GetKeyboardType, (INT type))
{
  debug_message_fn("(%d) called\n", type);

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

  debug_message_fn(": formatted: %s", buf);

  return strlen(buf);
}

DEFINE_W32API(INT, GetDesktopWindow,
       (void))
{
  debug_message_fn("() called\n");
  return 0;
}

DEFINE_W32API(INT, GetDC,
	      (INT hwnd))
{
  debug_message_fn("(0x%x) called\n", hwnd);
  return 1;
}

DEFINE_W32API(INT, ReleaseDC,
	      (INT hwnd, INT hdc))
{
  debug_message_fn("(0x%x, 0x%x) called\n", hwnd, hdc);
  return 1;
}

DEFINE_W32API(INT, IsRectEmpty,
	      (CONST RECT *lprc))
{
  debug_message_fn("(%p) called\n", lprc);
  return 0;
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
