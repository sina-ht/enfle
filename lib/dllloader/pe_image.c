/*
 * pe_image.c -- PE_image implementation
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Apr 18 13:31:00 2004.
 * $Id: pe_image.c,v 1.27 2004/04/18 06:25:02 sian Exp $
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

#include <stdlib.h>
#include <string.h>

/* for LDT */
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

#ifndef __i386__
# error i386 or later required
#endif

#ifdef __linux__
# include <asm/ldt.h>
# include <asm/unistd.h>
// tls-2.5.31-D9 by Ingo Molnar <mingo@elte.hu> changed the name...
# include <linux/version.h>
# if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,32)
#  define modify_ldt_ldt_s user_desc
# endif
#elif defined(__FreeBSD__)
# include <machine/segments.h>
# include <machine/sysarch.h>
#else
# error Sorry, LDT is not supported. Please submit a patch.
#endif

#include "pe_image.h"
#include "utils/misc.h"
#include "utils/libstring.h"

#include "w32api.h"
#include "module.h"

/* import dlls */
#include "kernel32.h"
#include "user32.h"
#include "advapi32.h"
#include "msvcrt.h"
#include "borlndmm.h"
#include "msdmo.h"

#include "common.h"

static int load(PE_image *, char *);
static void *resolve(PE_image *, const char *);
static void destroy(PE_image *);

static PE_image template = {
  .filepath = NULL,
  .pe_header = IMAGE_FILE_HEADER_INITIALIZER,
  .sect_headers = NULL,
  .export_symbols = NULL,
  .image = NULL,
  .load = load,
  .resolve = resolve,
  .destroy = destroy
};

PE_image *
peimage_create(void)
{
  PE_image *p;

  if ((p = calloc(1, sizeof(PE_image))) == NULL)
    return NULL;
  memcpy(p, &template, sizeof(PE_image));
  debug_message("PE_image = %p\n", p);

  return p;
}

/* for internal use */

DEFINE_W32API(void, unknown_symbol, (void))
{
  show_message("unknown symbol called\n");
}

static Symbol_info *
get_dll_symbols(char *dllname)
{
  int i;

  static struct {
    const char *name;
    Symbol_info *(*get_symbols)(void);
  } name_to_func[] = {
    { "kernel32.dll", kernel32_get_export_symbols },
    { "user32.dll",   user32_get_export_symbols },
    { "advapi32.dll", advapi32_get_export_symbols },
    { "msvcrt.dll",   msvcrt_get_export_symbols },
    { "borlndmm.dll", borlndmm_get_export_symbols },
    { "msdmo.dll", msdmo_get_export_symbols },
    { NULL, NULL }
  };

  for (i = 0; name_to_func[i].name != NULL; i++) {
    char *trimmed = misc_trim_ext(name_to_func[i].name, "dll");

    if (strcasecmp(name_to_func[i].name, dllname) == 0 ||
	strcasecmp(trimmed, dllname) == 0) {
      free(trimmed);
      return name_to_func[i].get_symbols();
    }
    free(trimmed);
  }

  return NULL;
}

/* LDT related */
#define LDT_INDEX 1
#define LDT_TABLE_IDENT 1
#define LDT_RPL 3
#define LDT_SELECTOR(i, t, rpl) ((i << 3) | (t << 2) | rpl)

#if defined(__linux__)
#ifdef PIC
static int
modify_ldt(int func, struct modify_ldt_ldt_s *p, unsigned long c)
{
  int res;
  __asm__ __volatile__("pushl %%ebx\n\t"
		       "movl %2,%%ebx\n\t"
		       "int $0x80\n\t"
		       "popl %%ebx"
		       : "=a" (res)
		       : "0" (__NR_modify_ldt),
		         "r" (func),
		         "c" (p),
		         "d" (c));
  return res;
}
#else
static _syscall3(int, modify_ldt, int, func, struct modify_ldt_ldt_s *, p, unsigned long, c);
#endif
#endif /* defined(linux) */

static int fs_installed = 0;
static char *fs_seg = NULL;

#if defined(__linux__)
void
setup_fs(void)
{
  unsigned int fs = LDT_SELECTOR(LDT_INDEX, LDT_TABLE_IDENT, LDT_RPL);
  __asm__ __volatile__ (
			"movl %0, %%eax; movw %%ax, %%fs\n\t" : : "r" (fs)
			);
}

static int
install_fs(void)
{
  struct modify_ldt_ldt_s ldt;
  int fd, ret;
  void *prev;

  if (fs_installed)
    return 0;

  fd = open("/dev/zero", O_RDWR);
  if ((fs_seg = mmap(NULL, getpagesize(), PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0)) == NULL) {
    show_message("No enough memory for fs. Probably raising SIGSEGV...\n");
    return -1;
  }
  ldt.base_addr = (int)fs_seg;
  ldt.entry_number = LDT_INDEX;
  ldt.limit = ldt.base_addr + getpagesize() - 1;
  ldt.seg_32bit = 1;
  ldt.read_exec_only = 0;
  ldt.seg_not_present = 0;
  ldt.contents = MODIFY_LDT_CONTENTS_DATA;
  ldt.limit_in_pages = 0;
  if ((ret = modify_ldt(1, &ldt, sizeof(ldt))) < 0) {
    perror("install_fs");
    show_message("modify_ldt() failed. Probably raising SIGSEGV...\n");
  }
  setup_fs();
  prev = malloc(8);
  *(void **)ldt.base_addr = prev;
  close(fd);
  fs_installed = 1;
  return 0;
}
#elif defined(__FreeBSD__)
static int
install_fs(void)
{
  union descriptor d;
  int              ret;
  caddr_t          addr;
  size_t           psize;

  psize = getpagesize();
  addr = fs_seg = (caddr_t)mmap(0, psize * 2,
				PROT_READ|PROT_WRITE | PROT_EXEC,
				MAP_ANON | MAP_PRIVATE,
				-1, 0);
  if (addr == (void *)-1)
    return -1;

  memset(addr, 0x0, psize * 2);
  *(uint32_t *)(addr + 0) = (uint32_t)addr + psize;
  *(uint32_t *)(addr + 4) = (uint32_t)addr + psize * 2;
  d.sd.sd_lolimit = 2;
  d.sd.sd_hilimit = 0;
  d.sd.sd_lobase = ((uint32_t)addr & 0xffffff);
  d.sd.sd_hibase = ((uint32_t)addr >> 24) & 0xff;
  d.sd.sd_type = 0x12;
  d.sd.sd_dpl = SEL_UPL;
  d.sd.sd_p = 1;
  d.sd.sd_def32 = 1;
  d.sd.sd_gran = 1;
#define LDT_SEL_START	0xb
  ret = i386_set_ldt(LDT_SEL_START, &d, 1);
  if (ret < 0) {
    perror("i386_set_ldt");
    return ret;
  }
  __asm__("movl $0xf,%eax\n\t"
          "movw %ax,%fs\n\t");

  return 0;
}
#else
#error No install_fs()
#endif

#if 0
static int
uninstall_fs(void)
{
  if (fs_seg == NULL)
    return -1;
  munmap(fs_seg, getpagesize());
  fs_seg = 0;
  fs_installed = 0;

  return 0;
}
#endif

/*
0x0001 = Cursor
0x0002 = Bitmap
0x0003 = Icon
0x0004 = Menu
0x0005 = Dialog
0x0006 = String Table
0x0007 = Font Directory
0x0008 = Font
0x0009 = Accelerators Table
0x000A = RC Data (custom binary data)
0x000B = Message table
0x000C = Group Cursor
0x000E = Group Icon
0x0010 = Version Information
0x0011 = Dialog Include
0x0013 = Plug'n'Play
0x0014 = VXD
0x0015 = Animated Cursor
0x2002 = Bitmap (new version)
0x2004 = Menu (new version)
0x2005 = Dialog (new version)
*/
static void
traverse_directory(PE_image *p, IMAGE_RESOURCE_DIRECTORY *ird, String *s)
{
  unsigned int ne = ird->NumberOfNamedEntries;
  unsigned int ie = ird->NumberOfIdEntries;
  unsigned int i;

  debug_message(">>>%s(%d named, %d id)\n", string_get(s), ne, ie);

  for (i = 0; i < ne + ie; i++) {
    IMAGE_RESOURCE_DIRECTORY_ENTRY *irde = (IMAGE_RESOURCE_DIRECTORY_ENTRY *)((char *)ird + sizeof(*ird) + sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY) * i);
    String *new_s = string_dup(s);

    if (irde->Id & 0x80000000) {
      debug_message("Named Id: %d offset: %x\n", irde->Id & 0x7fffffff, irde->OffsetToData);
    } else {
      debug_message("Id: %d offset: %x\n", irde->Id, irde->OffsetToData);
    }

    /* XXX: should extract resource name */
    string_catf(new_s, "/0x%X", irde->Id);

    if (irde->OffsetToData & 0x80000000) {
      IMAGE_RESOURCE_DIRECTORY *new_ird;

      new_ird = (IMAGE_RESOURCE_DIRECTORY *)&p->image[p->opt_header.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress + (irde->OffsetToData & 0x7fffffff)];
      traverse_directory(p, new_ird, new_s);
    } else {
      IMAGE_RESOURCE_DATA_ENTRY *de = (IMAGE_RESOURCE_DATA_ENTRY *)&p->image[p->opt_header.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress + irde->OffsetToData];
      unsigned char *data;

      debug_message("%X %d\n", de->OffsetToData, de->Size);
      if ((data = (unsigned char *)malloc(4 + de->Size)) == NULL) {
	show_message("No enough memory for resource.\n");
	exit(-1);
      }
      data[0] =  de->Size >> 24;
      data[1] = (de->Size >> 16) & 0xff;
      data[2] = (de->Size >>  8) & 0xff;
      data[3] =  de->Size        & 0xff;
      memcpy(data + 4, &p->image[de->OffsetToData], de->Size);
      hash_define_str(p->resource, string_get(new_s), data);
    }
    string_destroy(new_s);
  }

  debug_message("<<<%s\n", string_get(s));
}

/* methods */

static int
load(PE_image *p, char *path)
{
  FILE *fp;
  unsigned char dos_header[DOS_HEADER_SIZE];
  unsigned int pe_signature;
  unsigned int pe_header_start;
  unsigned int i;
#ifdef DEBUG
  static const char *data_directory_names[] = {
    "Export", "Import", "Resource", "Exception", "Security", "BaseReloc",
    "Debug", "Copyright", "GlobalPtr", "TLS", "Load config", "Bound import", "IAT"
  };
#endif

  if ((fp = fopen(path, "rb")) == NULL) {
    show_message("PE_image: %s: Cannot open %s\n", __FUNCTION__, path);
    return 0;
  }
  p->filepath = strdup(path);

  fread(dos_header, 1, DOS_HEADER_SIZE, fp);
  pe_header_start = PE_HEADER_START(dos_header);
  debug_message("PE header found at %d\n", pe_header_start);

  fseek(fp, pe_header_start, SEEK_SET);
  fread(&pe_signature, 1, 4, fp);
  if (pe_signature != PE_SIGNATURE) {
    show_message("PE_image: %s: Not PE file.\n", __FUNCTION__);
    return 0;
  }

  fread(&p->pe_header, 1, PE_HEADER_SIZE, fp);
  switch (p->pe_header.Machine) {
  case 0x14c:
  case 0x14d:
  case 0x14e:
    break;
  default:
    show_message("PE_image: %s: unsupported architecture %03X\n", __FUNCTION__, p->pe_header.Machine);
    return 0;
  }

  debug_message("Section: %d sections\n", p->pe_header.NumberOfSections);
  if (p->pe_header.SizeOfOptionalHeader != OPTIONAL_HEADER_SIZE) {
    show_message("PE_image: %s: Optional Header Size %d != %d\n", __FUNCTION__, p->pe_header.SizeOfOptionalHeader, OPTIONAL_HEADER_SIZE);
    return 0;
  }

  fread(&p->opt_header, 1, OPTIONAL_HEADER_SIZE, fp);
  if (p->opt_header.Magic != OPTIONAL_SIGNATURE) {
    show_message("PE_image: %s: PE file but corrupted optional header.\n", __FUNCTION__);
    return 0;
  }
#if 0
  debug_message("-- Optional header\n");
  debug_message("Code:              %d\n", p->opt_header.SizeOfCode);
  debug_message("InitializedData:   %d\n", p->opt_header.SizeOfInitializedData);
  debug_message("UninitializedData: %d\n", p->opt_header.SizeOfUninitializedData);
  debug_message("EntryPoint:        0x%08X\n", p->opt_header.AddressOfEntryPoint);
  debug_message("BaseOfCode:        0x%08X\n", p->opt_header.BaseOfCode);
  debug_message("BaseOfData:        0x%08X\n", p->opt_header.BaseOfData);
  debug_message("ImageBase:         0x%08X\n", p->opt_header.ImageBase);
  debug_message("SectionAlignment:  0x%08X\n", p->opt_header.SectionAlignment);
  debug_message("FileAlignment:     0x%08X\n", p->opt_header.FileAlignment);
  debug_message("SizeOfImage:       0x%08X\n", p->opt_header.SizeOfImage);
  debug_message("SizeOfHeaders:     0x%08X\n", p->opt_header.SizeOfHeaders);
  debug_message("DataDirectories:\n");
#endif
#ifdef DEBUG
  for (i = 0; i < 12; i++)
    if (p->opt_header.DataDirectory[i].Size)
      debug_message(" %s: 0x%08X %d\n",
	      data_directory_names[i],
	      p->opt_header.DataDirectory[i].VirtualAddress,
	      p->opt_header.DataDirectory[i].Size);
#endif

  if ((p->image = calloc(1, p->opt_header.SizeOfHeaders + p->opt_header.SizeOfImage)) == NULL) {
    show_message("No enough memory for image (%d bytes)\n",
		 p->opt_header.SizeOfImage + p->opt_header.SizeOfHeaders);
    return 0;
  }
  debug_message("Image base %p size 0x%X\n", p->image, p->opt_header.SizeOfHeaders + p->opt_header.SizeOfImage);
  fseek(fp, 0, SEEK_SET);
  fread(p->image, 1, p->opt_header.SizeOfHeaders, fp);

  p->sect_headers = (IMAGE_SECTION_HEADER *)(p->image + pe_header_start + 4 + PE_HEADER_SIZE + OPTIONAL_HEADER_SIZE);
  for (i = 0; i < p->pe_header.NumberOfSections; i++) {
    debug_message("Section[%s]: RVA: 0x%08X File: 0x%08X Memory %p Size: 0x%X ", p->sect_headers[i].Name, p->sect_headers[i].VirtualAddress, p->sect_headers[i].PointerToRawData, p->image + p->sect_headers[i].VirtualAddress, p->sect_headers[i].SizeOfRawData);
    if (p->sect_headers[i].Characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA) {
      memset(p->image + p->sect_headers[i].VirtualAddress, 0, p->opt_header.SizeOfUninitializedData);
      debug_message("zero-cleared at 0x%X (%d bytes).\n", p->sect_headers[i].VirtualAddress, p->opt_header.SizeOfUninitializedData);
    } else {
      if (p->sect_headers[i].SizeOfRawData > 0) {
	fseek(fp, p->sect_headers[i].PointerToRawData, SEEK_SET);
	fread(p->image + p->sect_headers[i].VirtualAddress, 1, p->sect_headers[i].SizeOfRawData, fp);
	debug_message("loaded at 0x%X.\n", p->sect_headers[i].VirtualAddress);
      } else {
	debug_message("at 0x%X.\n", p->sect_headers[i].VirtualAddress);
      }
    }
  }

  fclose(fp);

  /* export symbols */
  debug_message("-- Exported Symbols\n");
  p->export_symbols = hash_create(PE_EXPORT_HASH_SIZE);
  if (p->opt_header.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size) {
    IMAGE_EXPORT_DIRECTORY *ied = (IMAGE_EXPORT_DIRECTORY *)&p->image[p->opt_header.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress];
    Symbol_info *export_syminfo;

    debug_message("Export DLL Name: %s\n", p->image + ied->Name);
    debug_message("%d function(s) %d name(s)\n", ied->NumberOfFunctions, ied->NumberOfNames);
    debug_message("Exported symbol(s):\n");

    if ((export_syminfo = calloc(ied->NumberOfNames + 1, sizeof(Symbol_info))) == NULL) {
      show_message("No enough memory...\n");
      return 0;
    }

    for (i = 0; i < ied->NumberOfNames; i++) {
      DWORD *functions = (DWORD *)(p->image + ied->AddressOfFunctions);
      DWORD *names = (DWORD *)(p->image + ied->AddressOfNames);
      WORD *name_ordinals = (WORD *)(p->image + ied->AddressOfNameOrdinals);

      hash_set_str_value(p->export_symbols, p->image + names[i], p->image + functions[name_ordinals[i]]);
      debug_message(" %s", p->image + names[i]);

      export_syminfo[i].name = (char *)p->image + names[i];
      export_syminfo[i].value = p->image + functions[name_ordinals[i]];
    }
    debug_message("\n");
    export_syminfo[i].name = NULL;
    module_register(misc_basename(path), export_syminfo);
  }

  /* import symbols */
  debug_message("-- Imported Symbols\n");
  if (p->opt_header.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size) {
    IMAGE_IMPORT_DESCRIPTOR *iid = (IMAGE_IMPORT_DESCRIPTOR *)&p->image[p->opt_header.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress];
    IMAGE_THUNK_DATA *thunk, *othunk;
    IMAGE_IMPORT_BY_NAME *iibn;
    int j;
    Symbol_info *syminfo;

    /* per DLL */
    for (i = 0; iid[i].Name; i++) {
      if (iid[i].u.OriginalFirstThunk) {
	othunk = (IMAGE_THUNK_DATA *)(p->image + (int)iid[i].u.OriginalFirstThunk);
      } else {
	othunk = (IMAGE_THUNK_DATA *)(p->image + (int)iid[i].FirstThunk);
      }
      thunk = (IMAGE_THUNK_DATA *)(p->image + (int)iid[i].FirstThunk);

      debug_message("Import DLL Name: %s\n", p->image + iid[i].Name);
      /* symbols in each DLL */
      for (j = 0; othunk[j].u1.AddressOfData; j++) {
	if (othunk[j].u1.Ordinal & IMAGE_ORDINAL_FLAG) {
#ifdef DEBUG
	  DWORD ordinal = othunk[j].u1.Ordinal & ~IMAGE_ORDINAL_FLAG;
	  debug_message(" #%d", ordinal);
#endif
	} else {
	  iibn = (IMAGE_IMPORT_BY_NAME *)(p->image + (int)othunk[j].u1.AddressOfData);
	  /* import */
	  if ((syminfo = get_dll_symbols((char *)p->image + iid[i].Name)) != NULL) {
	    for (; ; syminfo++) {
	      if (syminfo->name) {
		if (strcmp((char *)iibn->Name, syminfo->name) == 0) {
		  debug_message(" %s", iibn->Name);
		  thunk[j].u1.Function = (FARPROC)syminfo->value;
		  break;
		}
	      } else {
		thunk[j].u1.Function = (FARPROC)syminfo->value;
		debug_message(" **%s**", iibn->Name);
		break;
	      }
	    }
	  } else {
	    thunk[j].u1.Function = (FARPROC)unknown_symbol;
	    debug_message(" %s", iibn->Name);
	  }
	}
      }
      debug_message("\n");
    }
  }

  /* resource */
  debug_message("-- Resource\n");
  p->resource = hash_create(PE_RESOURCE_HASH_SIZE);
  if (p->opt_header.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size) {
    String *s;
    IMAGE_RESOURCE_DIRECTORY *ird = (IMAGE_RESOURCE_DIRECTORY *)&p->image[p->opt_header.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress];

    s = string_create();
    string_set(s, "");
    traverse_directory(p, ird, s);
    string_destroy(s);
  }

  /* relocation */
  debug_message("-- Relocation\n");
  if (p->opt_header.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size) {
    IMAGE_BASE_RELOCATION *ibr = (IMAGE_BASE_RELOCATION *)&p->image[p->opt_header.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress];
    int adjust = (int)(p->image - p->opt_header.ImageBase);

    while (ibr->VirtualAddress) {
      WORD *reloc = ibr->TypeOffset;
      unsigned int nrelocs = (ibr->SizeOfBlock - sizeof(DWORD) * 2) >> 1;

      for (i = 0; i < nrelocs; i++) {
	int type = (reloc[i] >> 12) & 0xf;
	int offset = reloc[i] & 0xfff;

	switch (type) {
	case 0:
	  break;
	case 3:
	  {
	    DWORD *patched = (DWORD *)(p->image + ibr->VirtualAddress + offset);
	    *patched += adjust;
	  }
	  break;
	default:
	  show_message("Unsupported relocation type %d...\n", type);
	  break;
	}
      }
      ibr = (IMAGE_BASE_RELOCATION *)(((unsigned char *)ibr) + ibr->SizeOfBlock);
    }
  }

  debug_message("-- Install fs\n");
  if (!fs_installed)
    install_fs();

  debug_message("-- Call InitDll\n");
  {
    DllEntryProc InitDll;
    int result;

    if ((InitDll = (DllEntryProc)resolve(p, "DllMain")) == NULL) {
      InitDll = (DllEntryProc)(p->image + p->opt_header.AddressOfEntryPoint);
      if (p->opt_header.AddressOfEntryPoint > p->opt_header.SizeOfHeaders + p->opt_header.SizeOfImage) {
	show_message("%s: InitDll %p is bad address\n", path, InitDll);
	InitDll = NULL;
      }
      if (p->opt_header.AddressOfEntryPoint < p->opt_header.SizeOfHeaders) {
	debug_message("EntryPoint %d < %d SizeOfHeaders\n", p->opt_header.AddressOfEntryPoint, p->opt_header.SizeOfHeaders);
	InitDll = NULL;
      }
    } else {
      debug_message("DllMain() found: %p <=> %p EntryPoint\n", InitDll, p->image + p->opt_header.AddressOfEntryPoint);
    }

    if (InitDll) {
      debug_message("InitDll %p\n", InitDll);
      /*
       * some dll (e.g. ir32_32.dll) break %ebx
       * but do as below will cause segmentation fault...
       */
      //__asm__ __volatile__("pushl %ebx\n\t");
      if ((result = InitDll((HMODULE)p, DLL_PROCESS_ATTACH, NULL)) != 1)
	show_message("InitDll returns %d\n", result);
      //__asm__ __volatile__("popl %ebx\n\t");
    }
  }

  debug_message("-- Load completed.\n");

  return 1;
}

static void *
resolve(PE_image *p, const char *symbolname)
{
  return hash_lookup_str(p->export_symbols, (char *)symbolname);
}

static void
destroy(PE_image *p)
{
  module_deregister(misc_basename(p->filepath));
  if (p->resource)
    hash_destroy(p->resource);
  if (p->export_symbols)
    hash_destroy(p->export_symbols);
  if (p->filepath)
    free(p->filepath);
  if (p->image)
    free(p->image);
  free(p);
}
