/*
 * advapi32.c -- implementation of routines in advapi32.dll
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Apr  9 00:13:31 2004.
 * $Id: advapi32.c,v 1.4 2004/04/12 04:13:13 sian Exp $
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

#include "compat.h"
#include "common.h"

#include "w32api.h"
#include "module.h"

#include "advapi32.h"

DECLARE_W32API(DWORD, GetUserNameA, (LPSTR, LPDWORD));
DECLARE_W32API(long, RegOpenKeyA, (long, LPCSTR, int *));
DECLARE_W32API(DWORD, RegOpenKeyExA, (HKEY, LPCSTR, DWORD, REGSAM, LPHKEY));
DECLARE_W32API(DWORD, RegCloseKey, (HKEY));
DECLARE_W32API(DWORD, RegCreateKeyExA, (HKEY, LPCSTR, DWORD, LPSTR, DWORD, REGSAM, SECURITY_ATTRIBUTES *, LPHKEY, LPDWORD));
DECLARE_W32API(DWORD, RegDeleteKeyA, (HKEY, LPCSTR));
DECLARE_W32API(DWORD, RegQueryValueExA, (HKEY, LPCSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD));
DECLARE_W32API(DWORD, RegSetValueExA, (HKEY, LPCSTR, DWORD, DWORD, CONST BYTE *, DWORD));

static void unknown_symbol(void);

static Symbol_info symbol_infos[] = {
  { "GetUserNameA", GetUserNameA },
  { "RegOpenKeyA", RegOpenKeyA },
  { "RegOpenKeyExA", RegOpenKeyExA },
  { "RegCloseKey", RegCloseKey },
  { "RegCreateKeyExA", RegCreateKeyExA },
  { "RegDeleteKeyA", RegDeleteKeyA },
  { "RegQueryValueExA", RegQueryValueExA },
  { "RegSetValueExA", RegSetValueExA },
  { NULL, unknown_symbol }
};

DEFINE_W32API(DWORD, GetUserNameA,
	      (LPSTR a, LPDWORD b))
{
  debug_message_fn("(%p) called\n", a);
  return 0;
}

DEFINE_W32API(long, RegOpenKeyA,
	      (long a, LPCSTR b, int *c))
{
  debug_message_fn("(%ld, %s) called\n", a, b);
  return 0;
}

DEFINE_W32API(DWORD, RegOpenKeyExA,
	      (HKEY handle, LPCSTR name, DWORD reserved, REGSAM access, LPHKEY key_return))
{
  debug_message_fn("(%s) called\n", name);
  return 0;
}

DEFINE_W32API(DWORD, RegCloseKey,
	      (HKEY handle))
{
  debug_message_fn("() called\n");
  return 1;
}

DEFINE_W32API(DWORD, RegCreateKeyExA,
	      (HKEY handle, LPCSTR name, DWORD reserved, LPSTR class, DWORD options,
	       REGSAM access, SECURITY_ATTRIBUTES *sa, LPHKEY key_return, LPDWORD dispos))
{
  debug_message_fn("(%s) called\n", name);
  return 1;
}

DEFINE_W32API(DWORD, RegDeleteKeyA, (HKEY handle, LPCSTR name))
{
  debug_message_fn("(%s) called\n", name);
  return 1;
}

DEFINE_W32API(DWORD, RegQueryValueExA,
	      (HKEY handle, LPCSTR name, LPDWORD reserved, LPDWORD type,
	       LPBYTE data, LPDWORD count))
{
  debug_message_fn("(%s) called\n", name);
  return 1;
}

DEFINE_W32API(DWORD, RegSetValueExA,
	      (HKEY handle, LPCSTR name, DWORD reserved, DWORD type,
	       CONST BYTE *data, DWORD count))
{
  debug_message_fn("(%s) called\n", name);
  return 1;
}

/* unimplemened */

static void
unknown_symbol(void)
{
  show_message("unknown symbol in advapi32 called\n");
}

/* export */

Symbol_info *
advapi32_get_export_symbols(void)
{
  module_register("advapi32.dll", symbol_infos);
  return symbol_infos;
}
