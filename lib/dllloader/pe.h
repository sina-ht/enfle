/*
 * pe.h
 */

#include "windef.h"

#define e_lfanew 0x3c
#define DOS_HEADER_SIZE (e_lfanew + 4)
#define PE_HEADER_START(p) (*((DWORD *)((BYTE *)p + e_lfanew)))
#define PE_HEADER_SIZE (sizeof(IMAGE_FILE_HEADER))
#define PE_SIGNATURE 0x00004550

typedef struct _image_file_header {
  WORD Machine;
  WORD NumberOfSections;
  DWORD TimeDateStamp;
  DWORD PointerToSymbolTable;
  DWORD NumberOfSymbols;
  WORD SizeOfOptionalHeader;
  WORD Characteristics;
} IMAGE_FILE_HEADER;

#define IMAGE_DIRECTORY_ENTRY_EXPORT 0
#define IMAGE_DIRECTORY_ENTRY_IMPORT 1
#define IMAGE_DIRECTORY_ENTRY_RESOURCE 2
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION 3
#define IMAGE_DIRECTORY_ENTRY_SECURITY 4
#define IMAGE_DIRECTORY_ENTRY_BASERELOC 5
#define IMAGE_DIRECTORY_ENTRY_DEBUG 6
#define IMAGE_DIRECTORY_ENTRY_COPYRIGHT 7
#define IMAGE_DIRECTORY_ENTRY_GLOBALPTR 8
#define IMAGE_DIRECTORY_ENTRY_TLS 9
#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG 10
#define IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT 11
#define IMAGE_DIRECTORY_ENTRY_IAT 12

typedef struct _image_data_directory {
  DWORD VirtualAddress;
  DWORD Size;
} IMAGE_DATA_DIRECTORY;

#define OPTIONAL_HEADER_SIZE (sizeof(IMAGE_OPTIONAL_HEADER))
#define OPTIONAL_SIGNATURE 0x010b
#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES 16

typedef struct _image_optional_header {
  /* Standard fields */
  WORD Magic;
  BYTE MajorLinkerVersion;
  BYTE MinorLinkerVersion;
  DWORD SizeOfCode;
  DWORD SizeOfInitializedData;
  DWORD SizeOfUninitializedData;
  DWORD AddressOfEntryPoint;
  DWORD BaseOfCode;
  DWORD BaseOfData;
  /* NT additional fields */
  DWORD ImageBase;
  DWORD SectionAlignment;
  DWORD FileAlignment;
  WORD MajorOperatingSystemVersion;
  WORD MinorOperatingSystemVersion;
  WORD MajorImageVersion;
  WORD MinorImageVersion;
  WORD MajorSubsystemVersion;
  WORD MinorSubsystemVersion;
  DWORD Win32VersionValue;
  DWORD SizeOfImage;
  DWORD SizeOfHeaders;
  DWORD CheckSum;
  WORD Subsystem;
  WORD DllCharacteristics;
  DWORD SizeOfStackReserve;
  DWORD SizeOfStackCommit;
  DWORD SizeOfHeapReserve;
  DWORD SizeOfHeapCommit;
  DWORD LoaderFlags;
  DWORD NumberOfRvaAndSizes;
  IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER;

#define SECTION_HEADER_SIZE (sizeof(IMAGE_SECTION_HEADER))
#define IMAGE_SIZEOF_SHORT_NAME 8

typedef struct _image_section_header {
  BYTE Name[IMAGE_SIZEOF_SHORT_NAME];
  union {
    DWORD PhysicalAddress;
    DWORD VirtualSize;
  } Misc;
  DWORD VirtualAddress;
  DWORD SizeOfRawData;
  DWORD PointerToRawData;
  DWORD PointerToRelocations;
  DWORD PointerToLinenumbers;
  WORD NumberOfRelocations;
  WORD NumberOfLinenumbers;
  DWORD Characteristics;
} IMAGE_SECTION_HEADER;

typedef struct _image_export_directory {
  DWORD Characteristics;
  DWORD TimeDateStamp;
  WORD MajorVersion;
  WORD MinorVersion;
  DWORD Name;
  DWORD Base;
  DWORD NumberOfFunctions;
  DWORD NumberOfNames;
  DWORD AddressOfFunctions;
  DWORD AddressOfNames;
  DWORD AddressOfNameOrdinals;
} IMAGE_EXPORT_DIRECTORY;

typedef struct _image_import_by_name {
  WORD Hint;
  BYTE Name[1];
} IMAGE_IMPORT_BY_NAME;

typedef struct _image_thunk_data {
  union { 
    LPBYTE ForwarderString;
    FARPROC Function;
    DWORD Ordinal;
    IMAGE_IMPORT_BY_NAME *AddressOfData;
  } u1;
} IMAGE_THUNK_DATA;

typedef struct _image_import_descriptor {
  union {
    DWORD Characteristics;
    IMAGE_THUNK_DATA *OriginalFirstThunk;
  } u;
  DWORD TimeDateStamp;
  DWORD ForwarderChain;
  DWORD Name;
  IMAGE_THUNK_DATA *FirstThunk;   
} IMAGE_IMPORT_DESCRIPTOR;

typedef struct _image_base_relocation {
  DWORD VirtualAddress;
  DWORD SizeOfBlock;
  WORD TypeOffset[1];
} IMAGE_BASE_RELOCATION;
