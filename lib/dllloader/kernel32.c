/*
 * kernel32.c -- implementation of routines in kernel32.dll
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Jun 19 01:31:11 2001.
 * $Id: kernel32.c,v 1.10 2001/06/19 08:16:19 sian Exp $
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

#define W32API_REQUEST_MEM_ALLOC
#define W32API_REQUEST_MEM_REALLOC
#define W32API_REQUEST_MEM_FREE
#include "mm.h"
#include "w32api.h"
#include "module.h"

#include "kernel32.h"

#define REQUIRE_UNISTD_H
#define REQUIRE_STRING_H
#include "compat.h"

#include "common.h"

#ifdef USE_PTHREAD
#  include <pthread.h>
#endif

#if 1
#define more_debug_message(format, args...)
#else
#define more_debug_message(format, args...) fprintf(stderr, format, ## args)
#endif

/* file related */
DECLARE_W32API(HANDLE, CreateFileA, (LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE));
DECLARE_W32API(HFILE, _lopen, (LPCSTR, INT));
DECLARE_W32API(BOOL, ReadFile, (HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED));
DECLARE_W32API(UINT, _lread, (HFILE, LPVOID, UINT));
DECLARE_W32API(BOOL, WriteFile, (HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED));
DECLARE_W32API(UINT, _lclose, (HFILE));
DECLARE_W32API(DWORD, SetFilePointer, (HANDLE, LONG, LONG *, DWORD));
DECLARE_W32API(LONG, _llseek, (HFILE, LONG, INT));
DECLARE_W32API(DWORD, GetFileSize, (HANDLE, LPDWORD));
DECLARE_W32API(DWORD, GetFileType, (HANDLE));
/* handle related */
DECLARE_W32API(HANDLE, GetStdHandle, (DWORD));
DECLARE_W32API(UINT, SetHandleCount, (UINT));
DECLARE_W32API(BOOL, CloseHandle, (HANDLE));
/* module related */
DECLARE_W32API(HMODULE, LoadLibraryA, (LPCSTR));
DECLARE_W32API(HMODULE, LoadLibraryExA, (LPCSTR, HANDLE, DWORD));
DECLARE_W32API(HMODULE, GetModuleHandleA, (LPCSTR));
DECLARE_W32API(DWORD, GetModuleFileNameA, (HMODULE, LPSTR, DWORD));
/* memory related */
DECLARE_W32API(HLOCAL, LocalAlloc, (UINT, DWORD));
DECLARE_W32API(HLOCAL, LocalReAlloc, (HLOCAL, DWORD, UINT));
DECLARE_W32API(HLOCAL, LocalFree, (HLOCAL));
DECLARE_W32API(LPVOID, LocalLock, (HLOCAL));
DECLARE_W32API(BOOL, LocalUnlock, (HLOCAL));
DECLARE_W32API(HGLOBAL, GlobalAlloc, (UINT, DWORD));
DECLARE_W32API(HGLOBAL, GlobalReAlloc, (HGLOBAL, DWORD, UINT));
DECLARE_W32API(HGLOBAL, GlobalFree, (HGLOBAL));
DECLARE_W32API(LPVOID, GlobalLock, (HGLOBAL));
DECLARE_W32API(BOOL, GlobalUnlock, (HGLOBAL));
DECLARE_W32API(VOID, GlobalMemoryStatus, (LPMEMORYSTATUS));
DECLARE_W32API(LPVOID, VirtualAlloc, (LPVOID, DWORD, DWORD, DWORD));
DECLARE_W32API(BOOL, VirtualFree, (LPVOID, DWORD, DWORD));
DECLARE_W32API(DWORD, VirtualQuery, (LPCVOID, LPMEMORY_BASIC_INFORMATION, DWORD));
DECLARE_W32API(BOOL, IsBadReadPtr, (LPCVOID, UINT));
DECLARE_W32API(BOOL, IsBadWritePtr, (LPCVOID, UINT));
/* heap related */
DECLARE_W32API(HANDLE, HeapCreate, (DWORD, DWORD, DWORD));
DECLARE_W32API(LPVOID, HeapAlloc, (HANDLE, DWORD, DWORD));
DECLARE_W32API(LPVOID, HeapReAlloc, (HANDLE, DWORD, LPVOID, DWORD));
DECLARE_W32API(BOOL, HeapFree, (HANDLE, DWORD, LPVOID));
DECLARE_W32API(BOOL, HeapDestroy, (HANDLE));
DECLARE_W32API(HANDLE, GetProcessHeap, (void));
DECLARE_W32API(DWORD, GetProcessHeaps, (DWORD, HANDLE *));
/* string related */
DECLARE_W32API(INT, lstrlenA, (LPCSTR));
DECLARE_W32API(LPSTR, lstrcpyA, (LPSTR, LPCSTR));
/* process related */
DECLARE_W32API(FARPROC, GetProcAddress, (HMODULE, LPCSTR));
DECLARE_W32API(void, ExitProcess, (DWORD)) __attribute__ ((noreturn));
/* thread related */
DECLARE_W32API(HANDLE, GetCurrentThread, (void));
DECLARE_W32API(DWORD, GetCurrentThreadId, (void));
DECLARE_W32API(LCID, GetThreadLocale, (void));
DECLARE_W32API(BOOL, DisableThreadLibraryCalls, (HMODULE));
DECLARE_W32API(HANDLE, CreateThread, (SECURITY_ATTRIBUTES *, DWORD, LPTHREAD_START_ROUTINE, LPVOID, DWORD, LPDWORD));
/* thread local-variable related */
DECLARE_W32API(DWORD, TlsAlloc, (void));
DECLARE_W32API(BOOL, TlsFree, (DWORD));
DECLARE_W32API(LPVOID, TlsGetValue, (DWORD));
DECLARE_W32API(BOOL, TlsSetValue, (DWORD, LPVOID));
/* critical section */
DECLARE_W32API(void, InitializeCriticalSection, (CRITICAL_SECTION *));
DECLARE_W32API(void, EnterCriticalSection, (CRITICAL_SECTION *));
DECLARE_W32API(void, LeaveCriticalSection, (CRITICAL_SECTION *));
DECLARE_W32API(void, DeleteCriticalSection, (CRITICAL_SECTION *));
/* exception */
DECLARE_W32API(DWORD, UnhandledExceptionFilter, (PEXCEPTION_POINTERS));
DECLARE_W32API(void, RaiseException, (DWORD, DWORD, DWORD, const LPDWORD));
/* Environment */
DECLARE_W32API(LPSTR, GetCommandLineA, (void));
DECLARE_W32API(LPSTR, GetEnvironmentStringsA, (void));
DECLARE_W32API(LPWSTR, GetEnvironmentStringsW, (void));
DECLARE_W32API(BOOL, FreeEnvironmentStringsA, (LPSTR));
DECLARE_W32API(BOOL, FreeEnvironmentStringsW, (LPWSTR));
DECLARE_W32API(VOID, GetStartupInfoA, (LPSTARTUPINFOA));
/* codepage */
DECLARE_W32API(INT, GetLocaleInfoA, (LCID, LCTYPE, LPSTR, INT));
DECLARE_W32API(UINT, GetACP, (void));
DECLARE_W32API(UINT, GetOEMCP, (void));
DECLARE_W32API(BOOL, GetCPInfo, (UINT, LPCPINFO));
DECLARE_W32API(BOOL, GetStringTypeW, (DWORD, LPCWSTR, INT, LPWORD));
DECLARE_W32API(INT, MultiByteToWideChar, (UINT, DWORD, LPCSTR, INT, LPWSTR, INT));
DECLARE_W32API(INT, WideCharToMultiByte, (UINT, DWORD, LPCWSTR, INT, LPSTR, INT, LPCSTR, BOOL *));
/* date and time */
DECLARE_W32API(BOOL, EnumCalendarInfoA, (CALINFO_ENUMPROCA, LCID, CALID, CALTYPE));
DECLARE_W32API(VOID, GetLocalTime, (LPSYSTEMTIME));
/* miscellaneous */
DECLARE_W32API(DWORD, GetLastError, (void));
DECLARE_W32API(LONG, GetVersion, (void));
DECLARE_W32API(BOOL, GetVersionExA, (OSVERSIONINFOA *));
/* unimplemented */
DECLARE_W32API(void, unknown_symbol, (void));

static Symbol_info symbol_infos[] = {
  { "CreateFileA", CreateFileA },
  { "_lopen", _lopen },
  { "ReadFile", ReadFile },
  { "_lread", _lread },
  { "WriteFile", WriteFile },
  { "_lclose", _lclose },
  { "SetFilePointer", SetFilePointer },
  { "_llseek", _llseek },
  { "GetFileSize", GetFileSize },
  { "GetFileType", GetFileType },
  { "GetStdHandle", GetStdHandle },
  { "SetHandleCount", SetHandleCount },
  { "CloseHandle", CloseHandle },
  { "LoadLibraryA", LoadLibraryA },
  { "LoadLibraryExA", LoadLibraryExA },
  { "GetModuleHandleA", GetModuleHandleA },
  { "GetModuleFileNameA", GetModuleFileNameA },
  { "LocalAlloc", LocalAlloc },
  { "LocalReAlloc", LocalReAlloc },
  { "LocalFree", LocalFree },
  { "LocalLock", LocalLock },
  { "LocalUnlock", LocalUnlock },
  { "GlobalAlloc", GlobalAlloc },
  { "GlobalReAlloc", GlobalReAlloc },
  { "GlobalFree", GlobalFree },
  { "GlobalLock", GlobalLock },
  { "GlobalUnlock", GlobalUnlock },
  { "GlobalMemoryStatus", GlobalMemoryStatus },
  { "VirtualAlloc", VirtualAlloc },
  { "VirtualFree", VirtualFree },
  { "VirtualQuery", VirtualQuery },
  { "IsBadReadPtr", IsBadReadPtr },
  { "IsBadWritePtr", IsBadWritePtr },
  { "HeapCreate", HeapCreate },
  { "HeapAlloc", HeapAlloc },
  { "HeapReAlloc", HeapReAlloc },
  { "HeapFree", HeapFree },
  { "HeapDestroy", HeapDestroy },
  { "GetProcessHeap", GetProcessHeap },
  { "GetProcessHeaps", GetProcessHeaps },
  { "lstrlenA", lstrlenA },
  { "lstrcpyA", lstrcpyA },
  { "GetProcAddress", GetProcAddress },
  { "ExitProcess", ExitProcess },
  { "GetCurrentThread", GetCurrentThread },
  { "GetCurrentThreadId", GetCurrentThreadId },
  { "GetThreadLocale", GetThreadLocale },
  { "DisableThreadLibraryCalls", DisableThreadLibraryCalls },
  { "CreateThread", CreateThread },
  { "TlsAlloc", TlsAlloc },
  { "TlsFree", TlsFree },
  { "TlsGetValue", TlsGetValue },
  { "TlsSetValue", TlsSetValue },
  { "InitializeCriticalSection", InitializeCriticalSection },
  { "EnterCriticalSection", EnterCriticalSection },
  { "LeaveCriticalSection", LeaveCriticalSection },
  { "DeleteCriticalSection", DeleteCriticalSection },
  { "UnhandledExceptionFilter", UnhandledExceptionFilter },
  { "RaiseException", RaiseException },
  { "GetCommandLineA", GetCommandLineA },
  { "GetEnvironmentStrings", GetEnvironmentStringsA },
  { "GetEnvironmentStringsA", GetEnvironmentStringsA },
  { "GetEnvironmentStringsW", GetEnvironmentStringsW },
  { "FreeEnvironmentStringsA", FreeEnvironmentStringsA },
  { "FreeEnvironmentStringsW", FreeEnvironmentStringsW },
  { "GetStartupInfoA", GetStartupInfoA },
  { "GetLocaleInfoA", GetLocaleInfoA },
  { "GetACP", GetACP },
  { "GetOEMCP", GetOEMCP },
  { "GetCPInfo", GetCPInfo },
  { "GetStringTypeW", GetStringTypeW },
  { "MultiByteToWideChar", MultiByteToWideChar },
  { "WideCharToMultiByte", WideCharToMultiByte },
  { "EnumCalendarInfoA", EnumCalendarInfoA },
  { "GetLocalTime", GetLocalTime },
  { "GetLastError", GetLastError },
  { "GetVersion", GetVersion },
  { "GetVersionExA", GetVersionExA },
  { NULL, unknown_symbol }
};

/* file related */

DEFINE_W32API(HANDLE, CreateFileA,
	      (LPCSTR filename, DWORD access, DWORD sharing,
	       LPSECURITY_ATTRIBUTES sa, DWORD creation,
	       DWORD attributes, HANDLE template))
{
  HANDLE handle;

  debug_message("CreateFileA(%s) called: ", filename);
  handle = (HANDLE)fopen(filename, "rb");
  debug_message("%p\n", handle);

  return handle;
}

DEFINE_W32API(HFILE, _lopen,
	      (LPCSTR path, INT mode))
{
  DWORD access = 0, sharing = 0;

  debug_message("_lopen(%s, %d) called\n", path, mode);
  return CreateFileA(path, access, sharing, NULL, OPEN_EXISTING, 0, (HANDLE)-1);
}

DEFINE_W32API(BOOL, ReadFile,
	      (HANDLE handle, LPVOID buffer, DWORD bytes_to_read,
	       LPDWORD bytes_read, LPOVERLAPPED overlapped))
{
  more_debug_message("ReadFile(%d bytes) called\n", bytes_to_read);

  if (bytes_read)
    *bytes_read = 0;
  if (bytes_to_read == 0)
    return TRUE;
  *bytes_read = fread(buffer, 1, bytes_to_read, (FILE *)handle);

  return (*bytes_read >= 0) ? TRUE : FALSE;
}

DEFINE_W32API(UINT, _lread,
	      (HFILE handle, LPVOID buffer, UINT count))
{
  DWORD result;

  more_debug_message("_lread(%d bytes) called\n", count);
  if (!ReadFile(handle, buffer, count, &result, NULL))
    return -1;
  return result;
}

DEFINE_W32API(BOOL, WriteFile,
	      (HANDLE handle, LPCVOID buffer, DWORD bytes_to_write, LPDWORD bytes_written,
	       LPOVERLAPPED overlapped))
{
  debug_message("WriteFile() called\n");

  return TRUE;
}

DEFINE_W32API(UINT, _lclose,
	       (HFILE handle))
{
  debug_message("_lclose() called\n");
  return 0;
}

DEFINE_W32API(DWORD, SetFilePointer,
	      (HANDLE handle, LONG offset, LONG *high, DWORD whence))
{
  debug_message("SetFilePointer(%p, %ld, %d) called\n", handle, offset, whence);
  if (high && *high) {
    debug_message("*high != 0\n");
    return 0;
  }
  return (fseek((FILE *)handle, offset, whence) == 0) ? 1 : 0;
}

DEFINE_W32API(LONG, _llseek,
	      (HFILE handle, LONG offset, INT whence))
{
  debug_message("_llseek() called\n");
  return SetFilePointer(handle, offset, NULL, whence);
}

DEFINE_W32API(DWORD, GetFileSize,
	      (HANDLE handle, LPDWORD filesizehigh))
{
  DWORD size, cur;

  debug_message("GetFileSize() called\n");
  if (filesizehigh)
    *filesizehigh = 0;
  cur = ftell((FILE *)handle);
  fseek((FILE *)handle, 0, SEEK_END);
  size = ftell((FILE *)handle);
  fseek((FILE *)handle, cur, SEEK_SET);

  return size;
}

DEFINE_W32API(DWORD, GetFileType,
	      (HANDLE handle))
{
  debug_message("GetFileType() called\n");

  return FILE_TYPE_DISK;
}

/* handle related */

DEFINE_W32API(HANDLE, GetStdHandle,
	      (DWORD std))
{
  debug_message("GetStdHandle(%d) called\n", std);

  switch (std) {
  case STD_INPUT_HANDLE:
    return (HANDLE)stdin;
  case STD_OUTPUT_HANDLE:
    return (HANDLE)stdout;
  case STD_ERROR_HANDLE:
    return (HANDLE)stderr;
  default:
    break;
  }
  return (HANDLE)INVALID_HANDLE_VALUE;
}

DEFINE_W32API(UINT, SetHandleCount,
	      (UINT c))
{
  return (c < 256) ? c : 256;
}

DEFINE_W32API(BOOL, CloseHandle,
	      (HANDLE handle))
{
  debug_message("CloseHandle(%p)\n", handle);
  return TRUE;
}

/* module related */

DEFINE_W32API(HMODULE, LoadLibraryA,
	      (LPCSTR name))
{
  debug_message("LoadLibraryA(%s)\n", name);
  return LoadLibraryExA(name, 0, 0);
}

DEFINE_W32API(HMODULE, LoadLibraryExA,
	      (LPCSTR name, HANDLE handle, DWORD flags))
{
  debug_message("LoadLibraryExA(%s)\n", name);
  return NULL;
}

DEFINE_W32API(HMODULE, GetModuleHandleA,
	      (LPCSTR name))
{
  debug_message("GetModuleHandleA(%s)\n", name);
  return module_lookup(name);
}

DEFINE_W32API(DWORD, GetModuleFileNameA,
	      (HMODULE handle, LPSTR fileName, DWORD size))
{
  debug_message("GetModuleFileNameA(%d) called\n", (int)handle);
  /* XXX */
  return 0;
}

/* memory related */

DEFINE_W32API(HLOCAL, LocalAlloc,
	      (UINT flags, DWORD size))
{
  void *p;

  more_debug_message("LocalAlloc(%X, %d bytes) called: ", flags, size);

  p = w32api_mem_alloc(size);
  if ((flags & LMEM_ZEROINIT) && p)
    memset(p, 0, size);

  more_debug_message("%p\n", p);

  return p;
}

DEFINE_W32API(HLOCAL, LocalReAlloc,
	      (HLOCAL handle, DWORD size, UINT flags))
{
  more_debug_message("LocalReAlloc(%p, %d, %X) called\n", handle, size, flags);
  return w32api_mem_realloc(handle, size);
}

DEFINE_W32API(HLOCAL, LocalFree,
	      (HLOCAL handle))
{
  more_debug_message("LocalFree(%p) called\n", handle);
  w32api_mem_free(handle);
  return NULL;
}

DEFINE_W32API(LPVOID, LocalLock,
	      (HLOCAL handle))
{
  more_debug_message("LocalLock() called\n");
  return handle;
}

DEFINE_W32API(BOOL, LocalUnlock,
	      (HLOCAL handle))
{
  more_debug_message("LocalUnlock() called\n");
  return TRUE;
}

DEFINE_W32API(HGLOBAL, GlobalAlloc,
	      (UINT flags, DWORD size))
{
  void *p;

  more_debug_message("GlobalAlloc(0x%X, %d bytes) called: ", flags, size);

  p = w32api_mem_alloc(size);
  if ((flags & GMEM_ZEROINIT) && p)
    memset(p, 0, size);

  more_debug_message("%p\n", p);

  return p;
}

DEFINE_W32API(HGLOBAL, GlobalReAlloc,
	      (HGLOBAL handle, DWORD size, UINT flags))
{
  more_debug_message("GlobalReAlloc(%d, %d, 0x%X) called\n", (int)handle, size, flags);
  return w32api_mem_realloc(handle, size);
}

DEFINE_W32API(HGLOBAL, GlobalFree,
	      (HGLOBAL handle))
{
  more_debug_message("GlobalFree(%p) called\n", handle);
  w32api_mem_free(handle);
  return NULL;
}

DEFINE_W32API(LPVOID, GlobalLock,
	      (HGLOBAL handle))
{
  more_debug_message("GlobalLock() called\n");
  return handle;
}

DEFINE_W32API(BOOL, GlobalUnlock,
	      (HGLOBAL handle))
{
  more_debug_message("GlobalUnlock() called\n");
  return TRUE;
}

DEFINE_W32API(VOID, GlobalMemoryStatus,
	      (LPMEMORYSTATUS lpstat))
{
  more_debug_message("GlobalMemoryStatus() called\n");
}

typedef struct _vm_commited VMCommited;
struct _vm_commited {
  void *address;
  unsigned int size;
  VMCommited *next;
  VMCommited *prev;
};

typedef struct _vm_reserved VMReserved;
struct _vm_reserved {
  void *base;
  unsigned int size;
  VMCommited *commited;
  VMReserved *next;
  VMReserved *prev;
};

static VMReserved *vmr = NULL;

DEFINE_W32API(LPVOID, VirtualAlloc,
	      (LPVOID ptr, DWORD size, DWORD type, DWORD protect))
{
  void *p;

  more_debug_message("VirtualAlloc(%p, size %d, type 0x%X, protect %d) called\n", ptr, size, type, protect);

  if (type & MEM_RESERVE) {
    p = w32api_mem_alloc(size);
    if (vmr == NULL) {
      if ((vmr = calloc(1, sizeof(VMReserved))) == NULL)
	return NULL;
    } else {
      if ((vmr->next = calloc(1, sizeof(VMReserved))) == NULL)
	return NULL;
      vmr->next->prev = vmr;
      vmr = vmr->next;
    }
    vmr->base = p;
    vmr->size = size;
    /* vmr->commited = NULL; vmr->next = vmr->prev = NULL; (calloc() do this) */

    debug_message("VirtualAlloc: reserve: %p\n", p);

    return p;
  }

  if (type & MEM_COMMIT) {
    VMReserved *v;

    for (v = vmr; v; v = v->prev) {
      if (v->base <= ptr && ptr + size < v->base + v->size) {
	VMCommited *vc;

	if (v->commited) {
	  for (vc = v->commited; vc; vc = vc->prev) {
	    if (vc->address <= ptr && ptr < vc->address + vc->size)
	      return NULL;
	    if (vc->address <= ptr + size && ptr + size < vc->address + vc->size)
	      return NULL;
	  }
	  if ((v->commited->next = calloc(1, sizeof(VMCommited))) == NULL)
	    return NULL;
	  v->commited->next->prev = v->commited;
	  v->commited = v->commited->next;
	} else {
	  if ((v->commited = calloc(1, sizeof(VMCommited))) == NULL)
	    return NULL;
	}
	v->commited->address = ptr;
	v->commited->size = size;

	debug_message("VirtualAlloc: commit: %p\n", ptr);

	return ptr;
      }
    }
    return NULL;
  }

  debug_message("VirtualAlloc: neither MEM_RESERVE nor MEM_COMMIT\n");
  return NULL;
}

DEFINE_W32API(BOOL, VirtualFree,
	      (LPVOID ptr, DWORD size, DWORD type))
{
  more_debug_message("VirtualFree(%p, size %d, type 0x%X) called\n", ptr, size, type);

  if (type & MEM_RELEASE) {
    VMReserved *v;
    VMCommited *vc, *vc_p;

    for (v = vmr; v; v = v->prev) {
      if (v->base == ptr) {
	free(ptr);
	if (v->next)
	  v->next->prev = v->prev;
	if (v->prev)
	  v->prev->next = v->next;
	for (vc = v->commited; vc; vc = vc_p) {
	  vc_p = vc->prev;
	  free(vc);
	}
	free(v);
	return TRUE;
      }
    }
    return FALSE;
  }

  if (type & MEM_DECOMMIT) {
    VMReserved *v;

    for (v = vmr; v; v = v->prev) {
      if (v->base <= ptr && ptr + size <= v->base + v->size) {
	VMCommited *vc;

	for (vc = v->commited; vc; vc = vc->prev) {
	  if (vc->address == ptr) {
	    if (vc->next)
	      vc->next->prev = vc->prev;
	    if (vc->prev)
	      vc->prev->next = vc->next;
	    free(vc);
	    return TRUE;
	  }
	}
	return FALSE;
      }
    }
    return FALSE;
  }

  debug_message("VirtualFree: neither MEM_RELEASE nor MEM_DECOMMIT\n");
  return FALSE;
}

DEFINE_W32API(DWORD, VirtualQuery,
	      (LPCVOID p, LPMEMORY_BASIC_INFORMATION info, DWORD len))
{
  debug_message("VirtualQuery(%p size %d) called\n", p, len);
  return sizeof(*info);
}

DEFINE_W32API(BOOL, IsBadReadPtr,
	      (LPCVOID ptr, UINT size))
{
  debug_message("IsBadReadPtr(%p size %d) called\n", ptr, size);
  return TRUE;
}

DEFINE_W32API(BOOL, IsBadWritePtr,
	      (LPCVOID ptr, UINT size))
{
  debug_message("IsBadWritePtr(%p size %d) called\n", ptr, size);
  return TRUE;
}

/* heap related */

DEFINE_W32API(HANDLE, HeapCreate,
	      (DWORD flags, DWORD init, DWORD max))
{
  more_debug_message("HeapCreate(%X, %d, %d) called\n", flags, init, max);
  if (init)
    return (HANDLE)w32api_mem_alloc(init);
  debug_message("HeapCreate called with init == 0\n");
  return (HANDLE)w32api_mem_alloc(100000);
}

DEFINE_W32API(LPVOID, HeapAlloc,
	      (HANDLE handle, DWORD flags, DWORD size))
{
  void *p;

  p = w32api_mem_alloc(size);

  more_debug_message("HeapAlloc(%p, %X, %d) called -> %p\n", handle, flags, size, p);

  if ((flags & HEAP_ZERO_MEMORY) && p)
    memset(p, 0, size);

  return p;
}

DEFINE_W32API(LPVOID, HeapReAlloc,
	      (HANDLE handle, DWORD flags, LPVOID ptr, DWORD size))
{
  more_debug_message("HeapReAlloc(%p, %X, %p, %d) called\n", handle, flags, ptr, size);
  return w32api_mem_realloc(ptr, size);
}

DEFINE_W32API(BOOL, HeapFree,
	      (HANDLE handle, DWORD flags, LPVOID ptr))
{
  more_debug_message("HeapFree(%p, %X, %p) called\n", handle, flags, ptr);
  w32api_mem_free(ptr);

  return TRUE;
}

DEFINE_W32API(BOOL, HeapDestroy,
	      (HANDLE handle))
{
  more_debug_message("HeapDestroy(%p) called\n", handle);
  w32api_mem_free(handle);
  return TRUE;
}

DEFINE_W32API(HANDLE, GetProcessHeap,
	      (void))
{
  more_debug_message("GetProcessHeap() called\n");
  return (HANDLE)1;
}

DEFINE_W32API(DWORD, GetProcessHeaps,
	      (DWORD count, HANDLE *heaps))
{
  debug_message("GetProcessHeaps(count %d, handle *%p) called\n", count, heaps);
  return 1;
}

/* string related */

DEFINE_W32API(INT, lstrlenA,
	       (LPCSTR str))
{
  debug_message("lstrlenA(%s) called\n", str);

  if (str)
    return strlen(str);
  return 0;
}

DEFINE_W32API(LPSTR, lstrcpyA,
	      (LPSTR dest, LPCSTR src))
{
  /* debug_message("lstrcpyA(%p, %s) called\n", dest, src); */

  memcpy(dest, src, strlen(src) + 1);

  return dest;
}

/* process related */

DEFINE_W32API(FARPROC, GetProcAddress,
	     (HMODULE handle, LPCSTR funcname))
{
  Symbol_info *syminfo;
  int i;

  debug_message("GetProcAddress(%p, %s)\n", handle, funcname);

  if ((syminfo = (Symbol_info *)handle) == NULL)
    return NULL;

  /* hashing should be used */
  for (i = 0; syminfo[i].name; i++)
    if (strcmp(syminfo[i].name, funcname) == 0) {
      debug_message("GetProcAddress: resolve: %s -> %p\n", funcname, syminfo[i].value);
      return syminfo[i].value;
    }

  return NULL;
}

DEFINE_W32API(void, ExitProcess, (DWORD status))
{
  debug_message("ExitProcess() called\n");
  exit(status);
}

/* thread related */

DEFINE_W32API(HANDLE, GetCurrentThread,
	      (void))
{
  debug_message("GetCurrentThread() called\n");
  return (HANDLE)0xfffffffe;
}

DEFINE_W32API(DWORD, GetCurrentThreadId,
	      (void))
{
  debug_message("GetCurrentThreadId() called\n");
  return getpid();
}

DEFINE_W32API(LCID, GetThreadLocale,
	      (void))
{
  debug_message("GetThreadLocale() called\n");
  return 0;
}

DEFINE_W32API(BOOL, DisableThreadLibraryCalls, (HMODULE module))
{
  debug_message("DisableThreadLibraryCalls() called\n");
  return TRUE;
}

DEFINE_W32API(HANDLE, CreateThread, (SECURITY_ATTRIBUTES *sa, DWORD stack, LPTHREAD_START_ROUTINE start, LPVOID param, DWORD flags, LPDWORD id))
{
  debug_message("CreateThread() called\n");
  return 0;
}

/* thread local-variable related */

DEFINE_W32API(DWORD, TlsAlloc,
	      (void))
{
  void *p;

  debug_message("TlsAlloc() called\n");
  if ((p = w32api_mem_alloc(sizeof(void *))) == NULL)
    return -1;
  return (DWORD)p;
}

DEFINE_W32API(BOOL, TlsFree,
	      (DWORD index))
{
  debug_message("TlsFree(%p) called\n", (void *)index);
  w32api_mem_free((void *)index);
  return TRUE;
}

DEFINE_W32API(LPVOID, TlsGetValue,
	      (DWORD index))
{
  void **p = (void **)index;

  debug_message("TlsGetValue(%p) called\n", (void *)index);

  return *p;
}

DEFINE_W32API(BOOL, TlsSetValue,
	      (DWORD index, LPVOID value))
{
  void **p = (void **)index;

  debug_message("TlsSetValue(%p, %p) called ", (void *)index, value);

  *p = value;

  debug_message("OK\n");

  return TRUE;
}

/* critical section */

typedef struct _cs_private {
#ifdef USE_PTHREAD
  pthread_t thread;
  pthread_mutex_t mutex;
#endif
  int is_locked;
} CSPrivate;

DEFINE_W32API(void, InitializeCriticalSection,
	      (CRITICAL_SECTION *cs))
{
  CSPrivate *csp;

  debug_message("InitializeCriticalSection() called\n");
  if ((csp = calloc(1, sizeof(CSPrivate))) == NULL)
    return;
#ifdef USE_PTHREAD
  pthread_mutex_init(&csp->mutex, NULL);
#endif
  //csp->is_locked = 0;
  *(void **)cs = csp;
  return;
}

DEFINE_W32API(void, EnterCriticalSection,
	      (CRITICAL_SECTION *cs))
{
  CSPrivate *csp = *(CSPrivate **)cs;

  debug_message("EnterCriticalSection() called\n");

  if (csp->is_locked)
#ifdef USE_PTHREAD
    if (csp->thread == pthread_self())
#endif
      return;
#ifdef USE_PTHREAD
  pthread_mutex_lock(&csp->mutex);
#endif
  csp->is_locked = 1;
#ifdef USE_PTHREAD
  csp->thread = pthread_self();
#endif

  return;
}

DEFINE_W32API(void, LeaveCriticalSection,
	      (CRITICAL_SECTION *cs))
{
  CSPrivate *csp = *(CSPrivate **)cs;

  debug_message("LeaveCriticalSection() called\n");

  csp->is_locked = 0;
#ifdef USE_PTHREAD
  pthread_mutex_unlock(&csp->mutex);
#endif

  return;
}

DEFINE_W32API(void, DeleteCriticalSection,
	      (CRITICAL_SECTION *cs))
{
  CSPrivate *csp = *(CSPrivate **)cs;

  debug_message("DeleteCriticalSection() called\n");

#ifdef USE_PTHREAD
  pthread_mutex_destroy(&csp->mutex);
#endif
  free(csp);

  return;
}

/* exception */

DEFINE_W32API(DWORD, UnhandledExceptionFilter,
	      (PEXCEPTION_POINTERS pp))
{
  debug_message("UnhandledExceptionFilter() called\n");
  return 0;
}

DEFINE_W32API(void, RaiseException,
	      (DWORD code, DWORD flags, DWORD nbargs, const LPDWORD args))
{
  debug_message("RaiseException() called\n");
}

/* environment */

DEFINE_W32API(LPSTR, GetCommandLineA,
	      (void))
{
  debug_message("GetCommandLineA() called\n");
  return (LPSTR)"";
}

DEFINE_W32API(LPSTR, GetEnvironmentStringsA,
	      (void))
{
  debug_message("GetEnvironmentStringsA() called\n");
  return (LPSTR)"";
}

DEFINE_W32API(LPWSTR, GetEnvironmentStringsW,
	      (void))
{
  debug_message("GetEnvironmentStringsW() called\n");
  return (LPWSTR)"";
}

DEFINE_W32API(BOOL, FreeEnvironmentStringsA,
	      (LPSTR ptr))
{
  debug_message("FreeEnvironmentStringsA() called\n");

  return TRUE;
}

DEFINE_W32API(BOOL, FreeEnvironmentStringsW,
	      (LPWSTR ptr))
{
  debug_message("FreeEnvironmentStringsW() called\n");

  return TRUE;
}

DEFINE_W32API(VOID, GetStartupInfoA,
	      (LPSTARTUPINFOA info))
{
  debug_message("GetStartupInfoA() called\n");
}

/* codepage */

DEFINE_W32API(INT, GetLocaleInfoA,
	      (LCID id, LCTYPE type, LPSTR buf, INT len))
{
  debug_message("GetLocaleInfoA(%d, %d)\n", id, type);
  if (len)
    buf[0] = '\0';
  return 0;
}

DEFINE_W32API(UINT, GetACP,
	      (void))
{
  debug_message("GetACP() called\n");
  return 0;
}

DEFINE_W32API(UINT, GetOEMCP,
	      (void))
{
  debug_message("GetOEMCP() called\n");
  return 0;
}

DEFINE_W32API(BOOL, GetCPInfo,
	      (UINT cp, LPCPINFO info))
{
  debug_message("GetCPInfo() called\n");
  return TRUE;
}

DEFINE_W32API(BOOL, GetStringTypeW, (DWORD type, LPCWSTR src, INT count, LPWORD ctype))
{
  debug_message("GetStringTypeW() called\n");
  return TRUE;
}

/***********************************************************************
 *              MultiByteToWideChar   (KERNEL32)
 *
 * PARAMS
 *   page [in]    Codepage character set to convert from
 *   flags [in]   Character mapping flags
 *   src [in]     Source string buffer
 *   srclen [in]  Length of source string buffer
 *   dst [in]     Destination buffer
 *   dstlen [in]  Length of destination buffer
 *
 * NOTES
 *   The returned length includes the null terminator character.
 *
 * RETURNS
 *   Success: If dstlen > 0, number of characters written to destination
 *            buffer.  If dstlen == 0, number of characters needed to do
 *            conversion.
 *   Failure: 0. Occurs if not enough space is available.
 *
 * ERRORS
 *   ERROR_INSUFFICIENT_BUFFER
 *   ERROR_INVALID_PARAMETER
 *   ERROR_NO_UNICODE_TRANSLATION
 *
 * INT WINAPI MultiByteToWideChar( UINT page, DWORD flags, LPCSTR src, INT srclen,
 *                                 LPWSTR dst, INT dstlen )
 */
DEFINE_W32API(INT, MultiByteToWideChar,
	      (UINT page, DWORD flags, LPCSTR src, INT srclen, LPWSTR dest, INT destlen))
{
  debug_message("MultiByteToWideChar() called\n");

  if (dest && destlen > 0)
    *dest = 0;

  return 0;
}

/***********************************************************************
 *              WideCharToMultiByte   (KERNEL32)
 *
 * PARAMS
 *   page [in]    Codepage character set to convert to
 *   flags [in]   Character mapping flags
 *   src [in]     Source string buffer
 *   srclen [in]  Length of source string buffer
 *   dst [in]     Destination buffer
 *   dstlen [in]  Length of destination buffer
 *   defchar [in] Default character to use for conversion if no exact
 *                  conversion can be made
 *   used [out]   Set if default character was used in the conversion
 *
 * NOTES
 *   The returned length includes the null terminator character.
 *
 * RETURNS
 *   Success: If dstlen > 0, number of characters written to destination
 *            buffer.  If dstlen == 0, number of characters needed to do
 *            conversion.
 *   Failure: 0. Occurs if not enough space is available.
 *
 * ERRORS
 *   ERROR_INSUFFICIENT_BUFFER
 *   ERROR_INVALID_PARAMETER
 *
 * INT WINAPI WideCharToMultiByte( UINT page, DWORD flags, LPCWSTR src, INT srclen,
 *                                 LPSTR dst, INT dstlen, LPCSTR defchar, BOOL *used )
 */
DEFINE_W32API(INT, WideCharToMultiByte,
	      (UINT page, DWORD flags, LPCWSTR src, INT srclen, LPSTR dest, INT destlen,
	       LPCSTR defchar, BOOL *used))
{
  debug_message("WideCharToMultiByte() called\n");

  if (dest && destlen > 0)
    *dest = 0;

  return 0;
}

/* date and time */

DEFINE_W32API(BOOL, EnumCalendarInfoA,
	      (CALINFO_ENUMPROCA proc, LCID id, CALID calid, CALTYPE caltype))
{
  return FALSE;
}

DEFINE_W32API(VOID, GetLocalTime,
	      (LPSYSTEMTIME t))
{
  debug_message("GetLocalTime() called\n");
}

/* miscellaneous */

DEFINE_W32API(DWORD, GetLastError,
	      (void))
{
  debug_message("GetLastError() called\n");
  return 0;
}

DEFINE_W32API(LONG, GetVersion,
	      (void))
{
  debug_message("GetVersion() called\n");

  return 0xc0000a04; /* Windows98 */
}

DEFINE_W32API(BOOL, GetVersionExA,
	      (OSVERSIONINFOA *v))
{
  debug_message("GetVersionExA() called\n");

  v->dwMajorVersion = 4;
  v->dwMinorVersion = 10;
  v->dwBuildNumber = 0x40a07ce;
  v->dwPlatformId = VER_PLATFORM_WIN32_WINDOWS;
  strcpy(v->szCSDVersion, "Win98");

  return TRUE;
}

/* unimplemened */

DEFINE_W32API(void, unknown_symbol,
	      (void))
{
  show_message("unknown symbol in kernel32 called\n");
}

/* export */

Symbol_info *
kernel32_get_export_symbols(void)
{
  module_register("kernel32.dll", symbol_infos);
  return symbol_infos;
}
