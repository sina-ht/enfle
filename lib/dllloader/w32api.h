/*
 * w32api.h
 */

#ifndef _W32API_H
#define _W32API_H

#include "windef.h"

#define DECLARE_W32API(type, api_name, param) static type CALLBACK api_name param
#define DEFINE_W32API(type, api_name, param) static type CALLBACK api_name param

/* File creation flags */
#define FILE_FLAG_WRITE_THROUGH    0x80000000UL
#define FILE_FLAG_OVERLAPPED       0x40000000L
#define FILE_FLAG_NO_BUFFERING     0x20000000L
#define FILE_FLAG_RANDOM_ACCESS    0x10000000L
#define FILE_FLAG_SEQUENTIAL_SCAN  0x08000000L
#define FILE_FLAG_DELETE_ON_CLOSE  0x04000000L
#define FILE_FLAG_BACKUP_SEMANTICS 0x02000000L
#define FILE_FLAG_POSIX_SEMANTICS  0x01000000L
#define CREATE_NEW        1
#define CREATE_ALWAYS     2
#define OPEN_EXISTING     3
#define OPEN_ALWAYS       4
#define TRUNCATE_EXISTING 5

typedef struct _overlapped {
  DWORD Internal;
  DWORD InternalHigh;
  DWORD Offset;
  DWORD OffsetHigh;
  HANDLE hEvent;
} OVERLAPPED, *LPOVERLAPPED;

typedef struct _security_attributes {
  DWORD nLength;
  LPVOID lpSecurityDescriptor;
  BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

#endif
