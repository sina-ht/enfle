/*
 * kernel32.c -- implementation of routines in kernel32.dll
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Sep 21 19:35:03 2001.
 * $Id: kernel32.c,v 1.14 2001/09/21 11:51:54 sian Exp $
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
DECLARE_W32API(HANDLE, GetCurrentProcess, (void));
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
/* interlocked */
DECLARE_W32API(LONG, InterlockedExchangeAdd, (PLONG,LONG));
DECLARE_W32API(LONG, InterlockedIncrement, (PLONG));
DECLARE_W32API(LONG, InterlockedDecrement, (PLONG));
/* exception */
DECLARE_W32API(DWORD, UnhandledExceptionFilter, (PEXCEPTION_POINTERS));
DECLARE_W32API(void, RaiseException, (DWORD, DWORD, DWORD, const LPDWORD));
/* Environment */
DECLARE_W32API(LPSTR, GetCommandLineA, (void));
DECLARE_W32API(DWORD, GetEnvironmentVariableA, (LPCSTR,LPSTR,DWORD));
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
  { "GetCurrentProcess", GetCurrentProcess },
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
  { "InterlockedExchangeAdd", InterlockedExchangeAdd },
  { "InterlockedIncrement", InterlockedIncrement },
  { "InterlockedDecrement", InterlockedDecrement },
  { "UnhandledExceptionFilter", UnhandledExceptionFilter },
  { "RaiseException", RaiseException },
  { "GetCommandLineA", GetCommandLineA },
  { "GetEnvironmentVariableA", GetEnvironmentVariableA },
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
	      (LPCSTR filename, DWORD _access, DWORD sharing,
	       LPSECURITY_ATTRIBUTES sa, DWORD creation,
	       DWORD attributes, HANDLE template))
{
  HANDLE handle;

  debug_message(__FUNCTION__ "(%s) called: ", filename);
  handle = (HANDLE)fopen(filename, "rb");
  debug_message("%p\n", handle);

  return handle;
}

DEFINE_W32API(HFILE, _lopen,
	      (LPCSTR path, INT mode))
{
  DWORD _access = 0, sharing = 0;

  debug_message(__FUNCTION__ "(%s, %d) called\n", path, mode);
  return CreateFileA(path, _access, sharing, NULL, OPEN_EXISTING, 0, (HANDLE)-1);
}

DEFINE_W32API(BOOL, ReadFile,
	      (HANDLE handle, LPVOID buffer, DWORD bytes_to_read,
	       LPDWORD bytes_read, LPOVERLAPPED overlapped))
{
  int _bytes_read;
  more_debug_message(__FUNCTION__ "(%d bytes) called\n", bytes_to_read);

  if (bytes_read)
    *bytes_read = 0;
  if (bytes_to_read == 0)
    return TRUE;
  _bytes_read = fread(buffer, 1, bytes_to_read, (FILE *)handle);
  *bytes_read = (DWORD)_bytes_read;

  return (_bytes_read >= 0) ? TRUE : FALSE;
}

DEFINE_W32API(UINT, _lread,
	      (HFILE handle, LPVOID buffer, UINT count))
{
  DWORD result;

  more_debug_message(__FUNCTION__ "(%d bytes) called\n", count);
  if (!ReadFile(handle, buffer, count, &result, NULL))
    return -1;
  return result;
}

DEFINE_W32API(BOOL, WriteFile,
	      (HANDLE handle, LPCVOID buffer, DWORD bytes_to_write, LPDWORD bytes_written,
	       LPOVERLAPPED overlapped))
{
  debug_message(__FUNCTION__ "(handle %p, buffer %p, to_write %d) called\n", handle, buffer, bytes_to_write);

  return TRUE;
}

DEFINE_W32API(UINT, _lclose,
	       (HFILE handle))
{
  debug_message(__FUNCTION__ "(%p) called\n", handle);
  return 0;
}

DEFINE_W32API(DWORD, SetFilePointer,
	      (HANDLE handle, LONG offset, LONG *high, DWORD whence))
{
  debug_message(__FUNCTION__ "(%p, %ld, %d) called\n", handle, offset, whence);
  if (high && *high) {
    debug_message("*high != 0\n");
    return 0;
  }
  return (fseek((FILE *)handle, offset, whence) == 0) ? 1 : 0;
}

DEFINE_W32API(LONG, _llseek,
	      (HFILE handle, LONG offset, INT whence))
{
  debug_message(__FUNCTION__ "() called\n");
  return SetFilePointer(handle, offset, NULL, whence);
}

DEFINE_W32API(DWORD, GetFileSize,
	      (HANDLE handle, LPDWORD filesizehigh))
{
  DWORD size, cur;

  debug_message(__FUNCTION__ "(%p) called: ", handle);
  if (filesizehigh)
    *filesizehigh = 0;
  cur = ftell((FILE *)handle);
  fseek((FILE *)handle, 0, SEEK_END);
  size = ftell((FILE *)handle);
  fseek((FILE *)handle, cur, SEEK_SET);

  debug_message("%d bytes\n", size);
  return size;
}

DEFINE_W32API(DWORD, GetFileType,
	      (HANDLE handle))
{
  debug_message(__FUNCTION__ "(%p) called\n", handle);

  return FILE_TYPE_DISK;
}

/* handle related */

DEFINE_W32API(HANDLE, GetStdHandle,
	      (DWORD std))
{
  debug_message(__FUNCTION__ "(%d) called: ", std);

  switch (std) {
  case STD_INPUT_HANDLE:
    debug_message("stdin returned.\n");
    return (HANDLE)stdin;
  case STD_OUTPUT_HANDLE:
    debug_message("stdout returned.\n");
    return (HANDLE)stdout;
  case STD_ERROR_HANDLE:
    debug_message("stderr returned.\n");
    return (HANDLE)stderr;
  default:
    debug_message("unknown.\n");
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
  debug_message(__FUNCTION__ "(%p)\n", handle);
  return TRUE;
}

/* module related */

DEFINE_W32API(HMODULE, LoadLibraryA,
	      (LPCSTR name))
{
  debug_message(__FUNCTION__ "(%s)\n", name);
  return LoadLibraryExA(name, 0, 0);
}

DEFINE_W32API(HMODULE, LoadLibraryExA,
	      (LPCSTR name, HANDLE handle, DWORD flags))
{
  debug_message(__FUNCTION__ "(%s, handle %p)\n", name, handle);
  return NULL;
}

DEFINE_W32API(HMODULE, GetModuleHandleA,
	      (LPCSTR name))
{
  debug_message(__FUNCTION__ "(%s)\n", name);
  return module_lookup(name);
}

DEFINE_W32API(DWORD, GetModuleFileNameA,
	      (HMODULE handle, LPSTR s, DWORD size))
{
  debug_message(__FUNCTION__ "(%d, %p[%s], %d) called\n", (int)handle, s, s, size);
  /* XXX */
  strcpy(s, "c:\\windows\\system\\enfle.dll");

  return strlen(s);
}

/* memory related */

DEFINE_W32API(HLOCAL, LocalAlloc,
	      (UINT flags, DWORD size))
{
  void *p;

  more_debug_message(__FUNCTION__ "(%X, %d bytes) called: ", flags, size);

  p = w32api_mem_alloc(size);
  if ((flags & LMEM_ZEROINIT) && p)
    memset(p, 0, size);

  more_debug_message("%p\n", p);

  return p;
}

DEFINE_W32API(HLOCAL, LocalReAlloc,
	      (HLOCAL handle, DWORD size, UINT flags))
{
  more_debug_message(__FUNCTION__ "(%p, %d, %X) called\n", handle, size, flags);
  return w32api_mem_realloc(handle, size);
}

DEFINE_W32API(HLOCAL, LocalFree,
	      (HLOCAL handle))
{
  more_debug_message(__FUNCTION__ "(%p) called\n", handle);
  w32api_mem_free(handle);
  return NULL;
}

DEFINE_W32API(LPVOID, LocalLock,
	      (HLOCAL handle))
{
  more_debug_message(__FUNCTION__ "() called\n");
  return handle;
}

DEFINE_W32API(BOOL, LocalUnlock,
	      (HLOCAL handle))
{
  more_debug_message(__FUNCTION__ "(%p) called\n", handle);
  return TRUE;
}

DEFINE_W32API(HGLOBAL, GlobalAlloc,
	      (UINT flags, DWORD size))
{
  void *p;

  more_debug_message(__FUNCTION__ "(0x%X, %d bytes) called: ", flags, size);

  p = w32api_mem_alloc(size);
  if ((flags & GMEM_ZEROINIT) && p)
    memset(p, 0, size);

  more_debug_message("%p\n", p);

  return p;
}

DEFINE_W32API(HGLOBAL, GlobalReAlloc,
	      (HGLOBAL handle, DWORD size, UINT flags))
{
  more_debug_message(__FUNCTION__ "(%d, %d, 0x%X) called\n", (int)handle, size, flags);
  return w32api_mem_realloc(handle, size);
}

DEFINE_W32API(HGLOBAL, GlobalFree,
	      (HGLOBAL handle))
{
  more_debug_message(__FUNCTION__ "(%p) called\n", handle);
  w32api_mem_free(handle);
  return NULL;
}

DEFINE_W32API(LPVOID, GlobalLock,
	      (HGLOBAL handle))
{
  more_debug_message(__FUNCTION__ "() called\n");
  return handle;
}

DEFINE_W32API(BOOL, GlobalUnlock,
	      (HGLOBAL handle))
{
  more_debug_message(__FUNCTION__ "(%p) called\n", handle);
  return TRUE;
}

DEFINE_W32API(VOID, GlobalMemoryStatus,
	      (LPMEMORYSTATUS lpstat))
{
  more_debug_message(__FUNCTION__ "() called\n");
  /* XXX */
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

  more_debug_message(__FUNCTION__ "(%p, size %d, type 0x%X, protect %d) called\n", ptr, size, type, protect);

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

    debug_message(__FUNCTION__ ": reserve: %p\n", p);

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

	debug_message(__FUNCTION__ ": commit: %p\n", ptr);

	return ptr;
      }
    }
    return NULL;
  }

  debug_message(__FUNCTION__ ": neither MEM_RESERVE nor MEM_COMMIT\n");
  return NULL;
}

DEFINE_W32API(BOOL, VirtualFree,
	      (LPVOID ptr, DWORD size, DWORD type))
{
  more_debug_message(__FUNCTION__ "(%p, size %d, type 0x%X) called\n", ptr, size, type);

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

  debug_message(__FUNCTION__ ": neither MEM_RELEASE nor MEM_DECOMMIT\n");
  return FALSE;
}

DEFINE_W32API(DWORD, VirtualQuery,
	      (LPCVOID p, LPMEMORY_BASIC_INFORMATION info, DWORD len))
{
  debug_message(__FUNCTION__ "(%p size %d) called\n", p, len);
  return sizeof(*info);
}

DEFINE_W32API(BOOL, IsBadReadPtr,
	      (LPCVOID ptr, UINT size))
{
  debug_message(__FUNCTION__ "(%p size %d) called\n", ptr, size);
  return TRUE;
}

DEFINE_W32API(BOOL, IsBadWritePtr,
	      (LPCVOID ptr, UINT size))
{
  debug_message(__FUNCTION__ "(%p size %d) called\n", ptr, size);
  return TRUE;
}

/* heap related */

DEFINE_W32API(HANDLE, HeapCreate,
	      (DWORD flags, DWORD init, DWORD max))
{
  more_debug_message(__FUNCTION__ "(%X, %d, %d) called\n", flags, init, max);
  if (init)
    return (HANDLE)w32api_mem_alloc(init);
  debug_message(__FUNCTION__ "() called with init == 0\n");
  return (HANDLE)w32api_mem_alloc(100000);
}

DEFINE_W32API(LPVOID, HeapAlloc,
	      (HANDLE handle, DWORD flags, DWORD size))
{
  void *p;

  p = w32api_mem_alloc(size);

  more_debug_message(__FUNCTION__ "(%p, %X, %d) called -> %p\n", handle, flags, size, p);

  if ((flags & HEAP_ZERO_MEMORY) && p)
    memset(p, 0, size);

  return p;
}

DEFINE_W32API(LPVOID, HeapReAlloc,
	      (HANDLE handle, DWORD flags, LPVOID ptr, DWORD size))
{
  more_debug_message(__FUNCTION__ "(%p, %X, %p, %d) called\n", handle, flags, ptr, size);
  return w32api_mem_realloc(ptr, size);
}

DEFINE_W32API(BOOL, HeapFree,
	      (HANDLE handle, DWORD flags, LPVOID ptr))
{
  more_debug_message(__FUNCTION__ "(%p, %X, %p) called\n", handle, flags, ptr);
  w32api_mem_free(ptr);

  return TRUE;
}

DEFINE_W32API(BOOL, HeapDestroy,
	      (HANDLE handle))
{
  more_debug_message(__FUNCTION__ "(%p) called\n", handle);
  w32api_mem_free(handle);
  return TRUE;
}

DEFINE_W32API(HANDLE, GetProcessHeap,
	      (void))
{
  more_debug_message(__FUNCTION__ "() called\n");
  return (HANDLE)1;
}

DEFINE_W32API(DWORD, GetProcessHeaps,
	      (DWORD count, HANDLE *heaps))
{
  debug_message(__FUNCTION__ "(count %d, handle *%p) called\n", count, heaps);
  return 1;
}

/* string related */

DEFINE_W32API(INT, lstrlenA,
	       (LPCSTR str))
{
  debug_message(__FUNCTION__ "(%s) called\n", str);

  if (str)
    return strlen(str);
  return 0;
}

DEFINE_W32API(LPSTR, lstrcpyA,
	      (LPSTR dest, LPCSTR src))
{
  /* debug_message(__FUNCTION__ "(%p, %s) called\n", dest, src); */

  memcpy(dest, src, strlen(src) + 1);

  return dest;
}

/* process related */

DEFINE_W32API(HANDLE, GetCurrentProcess,
	      (void))
{
  debug_message(__FUNCTION__ "() called.\n");
  return (HANDLE)0xffffffff;
}

DEFINE_W32API(FARPROC, GetProcAddress,
	     (HMODULE handle, LPCSTR funcname))
{
  Symbol_info *syminfo;
  int i;

  debug_message(__FUNCTION__ "(%p, %s)\n", handle, funcname);

  if ((syminfo = (Symbol_info *)handle) == NULL)
    return NULL;

  /* hashing should be used */
  for (i = 0; syminfo[i].name; i++)
    if (strcmp(syminfo[i].name, funcname) == 0) {
      debug_message(__FUNCTION__ ": resolve: %s -> %p\n", funcname, syminfo[i].value);
      return syminfo[i].value;
    }

  return NULL;
}

DEFINE_W32API(void, ExitProcess, (DWORD status))
{
  debug_message(__FUNCTION__ "() called\n");
  exit(status);
}

/* thread related */

DEFINE_W32API(HANDLE, GetCurrentThread,
	      (void))
{
  debug_message(__FUNCTION__ "() called\n");
  return (HANDLE)0xfffffffe;
}

DEFINE_W32API(DWORD, GetCurrentThreadId,
	      (void))
{
  debug_message(__FUNCTION__ "() called\n");
  return getpid();
}

DEFINE_W32API(LCID, GetThreadLocale,
	      (void))
{
  debug_message(__FUNCTION__ "() called\n");
  return 0;
}

DEFINE_W32API(BOOL, DisableThreadLibraryCalls, (HMODULE module))
{
  debug_message(__FUNCTION__ "(module %p) called\n", module);
  return TRUE;
}

DEFINE_W32API(HANDLE, CreateThread, (SECURITY_ATTRIBUTES *sa, DWORD stack, LPTHREAD_START_ROUTINE start, LPVOID param, DWORD flags, LPDWORD id))
{
  debug_message(__FUNCTION__ "(sa %p, stack %d, start %p, param %p, flags %d, id %p) called\n", sa, stack, start, param, flags, id);
  return 0;
}

/* thread local-variable related */

DEFINE_W32API(DWORD, TlsAlloc,
	      (void))
{
  void *p;

  debug_message(__FUNCTION__ "() called\n");
  if ((p = w32api_mem_alloc(sizeof(void *))) == NULL)
    return -1;
  return (DWORD)p;
}

DEFINE_W32API(BOOL, TlsFree,
	      (DWORD i))
{
  debug_message(__FUNCTION__ "(%p) called\n", (void *)i);
  w32api_mem_free((void *)i);
  return TRUE;
}

DEFINE_W32API(LPVOID, TlsGetValue,
	      (DWORD i))
{
  void **p = (void **)i;

  debug_message(__FUNCTION__ "(%p) called\n", (void *)i);

  return *p;
}

DEFINE_W32API(BOOL, TlsSetValue,
	      (DWORD i, LPVOID value))
{
  void **p = (void **)i;

  debug_message(__FUNCTION__ "(%p, %p) called ", (void *)i, value);

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

  debug_message(__FUNCTION__ "() called\n");

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

  debug_message(__FUNCTION__ "() called\n");

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

  debug_message(__FUNCTION__ "() called\n");

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

  debug_message(__FUNCTION__ "() called\n");

#ifdef USE_PTHREAD
  pthread_mutex_destroy(&csp->mutex);
#endif
  free(csp);

  return;
}

/* Interlocked */
//PVOID WINAPI InterlockedCompareExchange(PVOID*, PVOID, PVOID);
//LONG  WINAPI InterlockedExchange(PLONG, LONG);

DEFINE_W32API(LONG, InterlockedExchangeAdd,
	      (PLONG dest, LONG incr))
{
  LONG ret;
  __asm__ __volatile__("lock; xaddl %0,(%1)"
		       : "=r" (ret)
		       : "r" (dest), "0" (incr)
		       : "memory" );
  return ret;
}

DEFINE_W32API(LONG, InterlockedIncrement,
	      (PLONG dest))
{
    LONG result = InterlockedExchangeAdd(dest, 1) + 1;
    debug_message(__FUNCTION__ "(0x%p => %ld) => %ld\n", dest, *dest, result);
    return result;
}

DEFINE_W32API(LONG, InterlockedDecrement,
	      (PLONG dest))
{
    LONG result = InterlockedExchangeAdd(dest, -1) - 1;
    debug_message(__FUNCTION__ "(0x%p => %ld) => %ld\n", dest, *dest, result);
    return result;
}

/* exception */

DEFINE_W32API(DWORD, UnhandledExceptionFilter,
	      (PEXCEPTION_POINTERS pp))
{
  debug_message(__FUNCTION__ "(%p) called\n", pp);
  return 0;
}

DEFINE_W32API(void, RaiseException,
	      (DWORD code, DWORD flags, DWORD nbargs, const LPDWORD args))
{
  debug_message(__FUNCTION__ "(code %d) called\n", code);
}

/* environment */

DEFINE_W32API(LPSTR, GetCommandLineA,
	      (void))
{
  debug_message(__FUNCTION__ "() called\n");
  return (LPSTR)"c:\\enfle.exe";
}

/*
 * I usually see __MSVCRT_HEAP_SELECT as name.
 * So, treat it specially.
 */
DEFINE_W32API(DWORD, GetEnvironmentVariableA,
	      (LPCSTR name, LPSTR field,DWORD size))
{
  debug_message(__FUNCTION__ "(%s, %p, %x) called\n", name, field, size);
  if (strcmp(name, "__MSVCRT_HEAP_SELECT") == 0) {
    strcpy(field, "__GLOBAL_HEAP_SELECTED,1");
  } else {
    if (field) {
      field[0] = '\0';
    }
  }

  return strlen(field);
}

DEFINE_W32API(LPSTR, GetEnvironmentStringsA,
	      (void))
{
  debug_message(__FUNCTION__ "() called\n");
  return (LPSTR)"Name=Value";
}

DEFINE_W32API(LPWSTR, GetEnvironmentStringsW,
	      (void))
{
  debug_message(__FUNCTION__ "() called\n");
  return (LPWSTR)0;
}

DEFINE_W32API(BOOL, FreeEnvironmentStringsA,
	      (LPSTR ptr))
{
  debug_message(__FUNCTION__ "(%p[%s]) called\n", ptr, ptr);
  return TRUE;
}

DEFINE_W32API(BOOL, FreeEnvironmentStringsW,
	      (LPWSTR ptr))
{
  debug_message(__FUNCTION__ "(%p[%s]) called\n", ptr, (char *)ptr);
  return TRUE;
}

DEFINE_W32API(VOID, GetStartupInfoA,
	      (LPSTARTUPINFOA info))
{
  debug_message(__FUNCTION__ "() called\n");
  memset(info, 0, sizeof(*info));
  info->cb = sizeof(*info);
}

/* codepage */

DEFINE_W32API(INT, GetLocaleInfoA,
	      (LCID id, LCTYPE type, LPSTR buf, INT len))
{
  debug_message(__FUNCTION__ "(%d, %d, %p, %d)\n", id, type, buf, len);
  if (buf && len)
    buf[0] = '\0';
  return 0;
}

/*
 * CodePages
 * 0000 => 7-bit ASCII
 * 03a4 => Japan (Shift - JIS X-0208)
 * 03b5 => Korea (Shift - KSC 5601)
 * 03b6 => Taiwan (Big5)
 * 04b0 => Unicode
 * 04e2 => Latin-2 (Eastern European)
 * 04e3 => Cyrillic
 * 04e4 => Multilingual
 * 04e5 => Greek
 * 04e6 => Turkish
 * 04e7 => Hebrew
 * 04e8 => Arabic
 */
DEFINE_W32API(UINT, GetACP,
	      (void))
{
  debug_message(__FUNCTION__ "() called\n");
  return 0;
}

DEFINE_W32API(UINT, GetOEMCP,
	      (void))
{
  debug_message(__FUNCTION__ "() called\n");
  return 0;
}

DEFINE_W32API(BOOL, GetCPInfo,
	      (UINT cp, LPCPINFO info))
{
  debug_message(__FUNCTION__ "(cp %d) called\n", cp);
  return TRUE;
}

DEFINE_W32API(BOOL, GetStringTypeW, (DWORD type, LPCWSTR src, INT count, LPWORD ctype))
{
  debug_message(__FUNCTION__ "() called\n");
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
	      (UINT page, DWORD flags, LPCSTR src, INT srclen,
	       LPWSTR dest, INT destlen))
{
  debug_message(__FUNCTION__ "() called\n");

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
	      (UINT page, DWORD flags, LPCWSTR src, INT srclen,
	       LPSTR dest, INT destlen, LPCSTR defchar, BOOL *used))
{
  debug_message(__FUNCTION__ "() called\n");

  if (dest && destlen > 0)
    *dest = 0;

  return 0;
}

/* date and time */

DEFINE_W32API(BOOL, EnumCalendarInfoA,
	      (CALINFO_ENUMPROCA proc, LCID id, CALID calid, CALTYPE caltype))
{
  debug_message(__FUNCTION__ "() called\n");
  return FALSE;
}

DEFINE_W32API(VOID, GetLocalTime,
	      (LPSYSTEMTIME t))
{
  debug_message(__FUNCTION__ "(%p) called\n", t);
}

/* miscellaneous */

DEFINE_W32API(DWORD, GetLastError,
	      (void))
{
  debug_message(__FUNCTION__ "() called: 0\n");
  return 0;
}

DEFINE_W32API(LONG, GetVersion,
	      (void))
{
  debug_message(__FUNCTION__ "() called\n");

  return 0xc0000a04; /* Windows98 */
}

DEFINE_W32API(BOOL, GetVersionExA,
	      (OSVERSIONINFOA *v))
{
  debug_message(__FUNCTION__ "() called\n");

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
