/*
 * kernel32.c -- implementation of routines in kernel32.dll
 */

#define W32API_REQUEST_MEM_ALLOC
#define W32API_REQUEST_MEM_REALLOC
#define W32API_REQUEST_MEM_FREE
#include "mm.h"
#include "w32api.h"
#include "module.h"

#include "kernel32.h"

#define REQUIRE_UNISTD_H
#include "compat.h"

#include "common.h"

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
/* heap related */
DECLARE_W32API(HANDLE, HeapCreate, (DWORD, DWORD, DWORD));
DECLARE_W32API(LPVOID, HeapAlloc, (HANDLE, DWORD, DWORD));
DECLARE_W32API(LPVOID, HeapReAlloc, (HANDLE, DWORD, LPVOID, DWORD));
DECLARE_W32API(BOOL, HeapFree, (HANDLE, DWORD, LPVOID));
DECLARE_W32API(BOOL, HeapDestroy, (HANDLE));
DECLARE_W32API(HANDLE, GetProcessHeap, ());
DECLARE_W32API(DWORD, GetProcessHeaps, (DWORD, HANDLE *));
/* string related */
DECLARE_W32API(INT, lstrlenA, (LPCSTR));
DECLARE_W32API(LPSTR, lstrcpyA, (LPSTR, LPCSTR));
/* process related */
DECLARE_W32API(FARPROC, GetProcAddress, (HMODULE, LPCSTR));
DECLARE_W32API(void, ExitProcess, (DWORD));
/* thread related */
DECLARE_W32API(HANDLE, GetCurrentThread, ());
DECLARE_W32API(DWORD, GetCurrentThreadId, ());
DECLARE_W32API(LCID, GetThreadLocale, ());
/* thread local-variable related */
DECLARE_W32API(DWORD, TlsAlloc, ());
DECLARE_W32API(BOOL, TlsFree, (DWORD));
DECLARE_W32API(LPVOID, TlsGetValue, (DWORD));
DECLARE_W32API(BOOL, TlsSetValue, (DWORD, LPVOID));
/* critical section */
DECLARE_W32API(void, InitializeCriticalSection, (CRITICAL_SECTION *));
DECLARE_W32API(LONG, EnterCriticalSection, (CRITICAL_SECTION *));
DECLARE_W32API(LONG, LeaveCriticalSection, (CRITICAL_SECTION *));
DECLARE_W32API(LONG, DeleteCriticalSection, (CRITICAL_SECTION *));
/* exception */
DECLARE_W32API(DWORD, UnhandledExceptionFilter, (PEXCEPTION_POINTERS));
DECLARE_W32API(void, RaiseException, (DWORD, DWORD, DWORD, const LPDWORD));
/* Environment */
DECLARE_W32API(LPSTR, GetCommandLineA, ());
DECLARE_W32API(LPSTR, GetEnvironmentStringsA, ());
DECLARE_W32API(LPWSTR, GetEnvironmentStringsW, ());
DECLARE_W32API(BOOL, FreeEnvironmentStringsA, (LPSTR));
DECLARE_W32API(BOOL, FreeEnvironmentStringsW, (LPWSTR));
DECLARE_W32API(VOID, GetStartupInfoA, (LPSTARTUPINFOA));
/* codepage */
DECLARE_W32API(INT, GetLocaleInfoA, (LCID, LCTYPE, LPSTR, INT));
DECLARE_W32API(UINT, GetACP, ());
DECLARE_W32API(UINT, GetOEMCP, ());
DECLARE_W32API(BOOL, GetCPInfo, (UINT, LPCPINFO));
DECLARE_W32API(BOOL, GetStringTypeW, (DWORD, LPCWSTR, INT, LPWORD));
DECLARE_W32API(INT, MultiByteToWideChar, (UINT, DWORD, LPCSTR, INT, LPWSTR, INT));
DECLARE_W32API(INT, WideCharToMultiByte, (UINT, DWORD, LPCWSTR, INT, LPSTR, INT, LPCSTR, BOOL *));
/* date and time */
DECLARE_W32API(BOOL, EnumCalendarInfoA, (CALINFO_ENUMPROCA, LCID, CALID, CALTYPE));
DECLARE_W32API(VOID, GetLocalTime, (LPSYSTEMTIME));
/* miscellaneous */
DECLARE_W32API(DWORD, GetLastError, ());
DECLARE_W32API(LONG, GetVersion, ());
DECLARE_W32API(BOOL, GetVersionExA, (OSVERSIONINFOA *));
/* unimplemented */
DECLARE_W32API(void, unknown_symbol, ());

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
  debug_message("CreateFileA(%s) called\n", filename);
  return (HANDLE)fopen(filename, "rb");
}

DEFINE_W32API(HFILE, _lopen,
	      (LPCSTR path, INT mode))
{
  DWORD access = 0, sharing = 0;

  debug_message("_lopen(%s, %d) called\n", path, mode);
  /* FILE_ConvertOFMode(mode, &access, &sharing); */
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

  more_debug_message("LocalAlloc(%X, %d bytes) called ", flags, size);

  p = w32api_mem_alloc(size);
  if ((flags & LMEM_ZEROINIT) && p)
    memset(p, 0, size);

  more_debug_message("OK.\n");

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
  more_debug_message("LocalFree() called\n");
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

  more_debug_message("GlobalAlloc(%X, %d bytes) called\n", flags, size);

  p = w32api_mem_alloc(size);
  if ((flags & GMEM_ZEROINIT) && p)
    memset(p, 0, size);
  return p;
}

DEFINE_W32API(HGLOBAL, GlobalReAlloc,
	      (HGLOBAL handle, DWORD size, UINT flags))
{
  more_debug_message("GlobalReAlloc(%d, %d, %X) called\n", (int)handle, size, flags);
  return w32api_mem_realloc(handle, size);
}

DEFINE_W32API(HGLOBAL, GlobalFree,
	      (HGLOBAL handle))
{
  more_debug_message("GlobalFree() called\n");
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

DEFINE_W32API(LPVOID, VirtualAlloc,
	      (LPVOID ptr, DWORD size, DWORD type, DWORD protect))
{
  more_debug_message("VirtualAlloc(%p size %d %d %d) called\n", ptr, size, type, protect);

  /* XXX */
  return w32api_mem_alloc(size);
}

DEFINE_W32API(BOOL, VirtualFree,
	      (LPVOID ptr, DWORD size, DWORD type))
{
  more_debug_message("VirtualFree(%p size %d %d) called\n", ptr, size, type);

  /* XXX */
  free(ptr);

  return TRUE;
}

DEFINE_W32API(DWORD, VirtualQuery, (LPCVOID p, LPMEMORY_BASIC_INFORMATION info, DWORD len))
{
  debug_message("VirtualQuery(%p size %d) called\n", p, len);
  return sizeof(*info);
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
  more_debug_message("HeapAlloc(%p, %X, %d) called\n", handle, flags, size);
  return w32api_mem_alloc(size);
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
	      ())
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
  debug_message("lstrcpyA(%p, %s) called\n", dest, src);

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
	      ())
{
  debug_message("GetCurrentThread() called\n");
  return (HANDLE)0xfffffffe;
}

DEFINE_W32API(DWORD, GetCurrentThreadId,
	      ())
{
  debug_message("GetCurrentThreadId() called\n");
  return getpid();
}

DEFINE_W32API(LCID, GetThreadLocale,
	      ())
{
  debug_message("GetThreadLocale() called\n");
  return 0;
}

/* thread local-variable related */

DEFINE_W32API(DWORD, TlsAlloc,
	      ())
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

DEFINE_W32API(void, InitializeCriticalSection,
	      (CRITICAL_SECTION *cs))
{
  debug_message("InitializeCriticalSection() called\n");
}

DEFINE_W32API(LONG, EnterCriticalSection,
	      (CRITICAL_SECTION *cs))
{
  debug_message("EnterCriticalSection() called\n");
  return 0;
}

DEFINE_W32API(LONG, LeaveCriticalSection,
	      (CRITICAL_SECTION *cs))
{
  debug_message("LeaveCriticalSection() called\n");
  return 0;
}

DEFINE_W32API(LONG, DeleteCriticalSection,
	      (CRITICAL_SECTION *cs))
{
  debug_message("DeleteCriticalSection() called\n");
  return 0;
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
	      ())
{
  debug_message("GetCommandLineA() called\n");
  return "";
}

DEFINE_W32API(LPSTR, GetEnvironmentStringsA,
	      ())
{
  debug_message("GetEnvironmentStringsA() called\n");
  return "";
}

DEFINE_W32API(LPWSTR, GetEnvironmentStringsW,
	      ())
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
  return 0;
}

DEFINE_W32API(UINT, GetACP, ())
{
  debug_message("GetACP() called\n");
  return 0;
}

DEFINE_W32API(UINT, GetOEMCP, ())
{
  debug_message("GetOEMCP() called\n");
  return 0;
}

DEFINE_W32API(BOOL, GetCPInfo, (UINT cp, LPCPINFO info))
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
	      ())
{
  debug_message("GetLastError() called\n");
  return 0;
}

DEFINE_W32API(LONG, GetVersion,
	      ())
{
  debug_message("GetVersion() called\n");

  return 0xc0000a04; /* Windows98 */
}

DEFINE_W32API(BOOL, GetVersionExA,
	      (OSVERSIONINFOA *v))
{
  debug_message("GetVersionExA() called\n");

  return TRUE;
}

/* unimplemened */

DEFINE_W32API(void, unknown_symbol,
	      ())
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
