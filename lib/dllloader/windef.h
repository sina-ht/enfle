/*
 * windef.h
 */

#ifndef _WINDEF_H
#define _WINDEF_H

#define __stdcall __attribute__((__stdcall__))
#define CALLBACK __stdcall
#define PASCAL __stdcall

#undef FALSE
#define FALSE 0

#undef TRUE
#define TRUE  1

#undef NULL
#define NULL  0

#define CONST const

typedef unsigned short int WORD;
typedef unsigned int DWORD;
typedef unsigned int UINT;
typedef unsigned char BYTE;
typedef unsigned long ULONG;
typedef char CHAR;
typedef long LONG;
typedef void VOID;
typedef int INT;
typedef int BOOL;
typedef unsigned short WCHAR;
typedef CHAR *LPSTR;
typedef const CHAR *LPCSTR;
typedef WCHAR *LPWSTR;
typedef const WCHAR *LPCWSTR;
typedef BYTE *LPBYTE;
typedef LONG LRESULT;
typedef LONG LPARAM;
typedef ULONG *ULONG_PTR;
typedef VOID *LPVOID;
typedef const VOID *LPCVOID;
typedef VOID *HANDLE;
typedef WORD *LPWORD;
typedef DWORD *LPDWORD;
typedef DWORD LCID;
typedef DWORD LCTYPE;
typedef DWORD CALID;
typedef DWORD CALTYPE;

#define DECLARE_HANDLE(a) \
  typedef HANDLE a; \
  typedef a *P##a; \
  typedef a *LP##a

DECLARE_HANDLE(HFILE);
DECLARE_HANDLE(HINSTANCE);
DECLARE_HANDLE(HLOCAL);
DECLARE_HANDLE(HGLOBAL);
DECLARE_HANDLE(HKEY);
DECLARE_HANDLE(HWND);

typedef HINSTANCE HMODULE;

/* callbacks */

typedef LRESULT CALLBACK (*FARPROC)(void);
typedef BOOL CALLBACK (*WNDENUMPROC)(HWND, LPARAM);
typedef BOOL CALLBACK (*DllEntryProc)(HMODULE, DWORD, LPVOID);
typedef BOOL CALLBACK (*CALINFO_ENUMPROCA)(LPSTR);
typedef DWORD CALLBACK (*LPTHREAD_START_ROUTINE)(LPVOID);

/* DLL */

#define DLL_PROCESS_DETACH 0
#define DLL_PROCESS_ATTACH 1
#define DLL_THREAD_ATTACH  2
#define DLL_THREAD_DETACH  3

/* file handle */

#define STD_INPUT_HANDLE  -10
#define STD_OUTPUT_HANDLE -11
#define STD_ERROR_HANDLE  -12

#define INVALID_HANDLE_VALUE -1

#define FILE_TYPE_UNKNOWN       0
#define FILE_TYPE_DISK          1
#define FILE_TYPE_CHAR          2
#define FILE_TYPE_PIPE          3
#define FILE_TYPE_REMOTE        32768

/* memory */

#define LMEM_FIXED          0   
#define LMEM_MOVEABLE       0x0002
#define LMEM_NOCOMPACT      0x0010
#define LMEM_NODISCARD      0x0020
#define LMEM_ZEROINIT       0x0040
#define LMEM_MODIFY         0x0080
#define LMEM_DISCARDABLE    0x0F00
#define LMEM_DISCARDED      0x4000
#define LMEM_LOCKCOUNT      0x00FF

#define LPTR (LMEM_FIXED | LMEM_ZEROINIT)
#define LHND (LMEM_MOVEABLE | LMEM_ZEROINIT)

#define NONZEROLHND         (LMEM_MOVEABLE)
#define NONZEROLPTR         (LMEM_FIXED)

#define GMEM_FIXED          0x0000
#define GMEM_MOVEABLE       0x0002
#define GMEM_NOCOMPACT      0x0010
#define GMEM_NODISCARD      0x0020
#define GMEM_ZEROINIT       0x0040
#define GMEM_MODIFY         0x0080
#define GMEM_DISCARDABLE    0x0100
#define GMEM_NOT_BANKED     0x1000
#define GMEM_SHARE          0x2000
#define GMEM_DDESHARE       0x2000
#define GMEM_NOTIFY         0x4000
#define GMEM_LOWER          GMEM_NOT_BANKED
#define GMEM_DISCARDED      0x4000
#define GMEM_LOCKCOUNT      0x00ff
#define GMEM_INVALID_HANDLE 0x8000

#define GHND                (GMEM_MOVEABLE | GMEM_ZEROINIT)
#define GPTR                (GMEM_FIXED | GMEM_ZEROINIT)

typedef struct _memorystatus {
  DWORD dwLength;
  DWORD dwMemoryLoad;
  DWORD dwTotalPhys;
  DWORD dwAvailPhys;
  DWORD dwTotalPageFile;
  DWORD dwAvailPageFile;
  DWORD dwTotalVirtual;
  DWORD dwAvailVirtual;
} MEMORYSTATUS, *LPMEMORYSTATUS;

typedef struct _memory_basic_information {
  LPVOID BaseAddress;
  LPVOID AllocationBase;
  DWORD AllocationProtect;
  DWORD RegionSize;
  DWORD State;
  DWORD Protect;
  DWORD Type;
} MEMORY_BASIC_INFORMATION,*LPMEMORY_BASIC_INFORMATION,*PMEMORY_BASIC_INFORMATION;

#define MEM_COMMIT              0x00001000
#define MEM_RESERVE             0x00002000
#define MEM_DECOMMIT            0x00004000
#define MEM_RELEASE             0x00008000
#define MEM_FREE                0x00010000
#define MEM_PRIVATE             0x00020000
#define MEM_MAPPED              0x00040000
#define MEM_RESET               0x00080000
#define MEM_TOP_DOWN            0x00100000

/* heap */

#define HEAP_NO_SERIALIZE               0x00000001
#define HEAP_GROWABLE                   0x00000002
#define HEAP_GENERATE_EXCEPTIONS        0x00000004
#define HEAP_ZERO_MEMORY                0x00000008
#define HEAP_REALLOC_IN_PLACE_ONLY      0x00000010
#define HEAP_TAIL_CHECKING_ENABLED      0x00000020
#define HEAP_FREE_CHECKING_ENABLED      0x00000040
#define HEAP_DISABLE_COALESCE_ON_FREE   0x00000080
#define HEAP_CREATE_ALIGN_16            0x00010000
#define HEAP_CREATE_ENABLE_TRACING      0x00020000
/* This is taken from wine */
#define HEAP_SHARED                     0x04000000  

/* registry */

typedef DWORD ACCESS_MASK;
typedef ACCESS_MASK REGSAM;

/* critical section */

typedef struct _list_entry {
  struct _list_entry *Flink;
  struct _list_entry *Blink;
} LIST_ENTRY, *PLIST_ENTRY;

typedef struct _rtl_critical_section_debug
{
  WORD Type;
  WORD CreatorBackTraceIndex;
  struct _rtl_critical_section *CriticalSection;
  LIST_ENTRY ProcessLocksList;
  DWORD EntryCount;
  DWORD ContentionCount;
  DWORD Spare[ 2 ];
} RTL_CRITICAL_SECTION_DEBUG, *PRTL_CRITICAL_SECTION_DEBUG, RTL_RESOURCE_DEBUG, *PRTL_RESOURCE_DEBUG;

typedef struct _rtl_critical_section {
  PRTL_CRITICAL_SECTION_DEBUG DebugInfo;
  LONG LockCount;
  LONG RecursionCount;
  HANDLE OwningThread;
  HANDLE LockSemaphore;
  ULONG_PTR SpinCount;
} RTL_CRITICAL_SECTION, *PRTL_CRITICAL_SECTION;

typedef RTL_CRITICAL_SECTION CRITICAL_SECTION;
typedef PRTL_CRITICAL_SECTION PCRITICAL_SECTION;

/* exception */

#define EXCEPTION_MAXIMUM_PARAMETERS 15

typedef struct _exception_record
{
  DWORD ExceptionCode;
  DWORD ExceptionFlags;
  struct _exception_record *ExceptionRecord;

  LPVOID ExceptionAddress;
  DWORD NumberParameters;
  DWORD ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];
} EXCEPTION_RECORD, *PEXCEPTION_RECORD;

typedef struct _exception_pointers {
  PEXCEPTION_RECORD ExceptionRecord;
  //PCONTEXT ContextRecord;
} EXCEPTION_POINTERS, *PEXCEPTION_POINTERS;

/* environment */

typedef struct _startupinfoa {
  DWORD cb;               /* 00: size of struct */
  LPSTR lpReserved;       /* 04: */
  LPSTR lpDesktop;        /* 08: */
  LPSTR lpTitle;          /* 0c: */
  DWORD dwX;              /* 10: */
  DWORD dwY;              /* 14: */
  DWORD dwXSize;          /* 18: */
  DWORD dwYSize;          /* 1c: */
  DWORD dwXCountChars;    /* 20: */
  DWORD dwYCountChars;    /* 24: */
  DWORD dwFillAttribute;  /* 28: */
  DWORD dwFlags;          /* 2c: */
  WORD wShowWindow;       /* 30: */
  WORD cbReserved2;       /* 32: */
  BYTE *lpReserved2;      /* 34: */
  HANDLE hStdInput;       /* 38: */
  HANDLE hStdOutput;      /* 3c: */
  HANDLE hStdError;       /* 40: */
} STARTUPINFOA, *LPSTARTUPINFOA;

/* codepage */

#define MAX_LEADBYTES     12
#define MAX_DEFAULTCHAR   2

typedef struct _cpinfo {
  UINT MaxCharSize;
  BYTE DefaultChar[MAX_DEFAULTCHAR];
  BYTE LeadByte[MAX_LEADBYTES];
} CPINFO, *LPCPINFO;

/* date and time */

typedef struct _systemtime {
  WORD wYear;
  WORD wMonth;
  WORD wDayOfWeek;
  WORD wDay;
  WORD wHour;
  WORD wMinute;
  WORD wSecond;
  WORD wMilliseconds;
} SYSTEMTIME, *LPSYSTEMTIME;

/* misc */

#define VER_PLATFORM_WIN32s        0
#define VER_PLATFORM_WIN32_WINDOWS 1
#define VER_PLATFORM_WIN32_NT      2

typedef struct _osversioninfoa {
  DWORD dwOSVersionInfoSize;
  DWORD dwMajorVersion;
  DWORD dwMinorVersion;
  DWORD dwBuildNumber;
  DWORD dwPlatformId;
  CHAR szCSDVersion[128];
} OSVERSIONINFOA;

#endif
