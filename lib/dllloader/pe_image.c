/*
 * pe_image.c -- PE_image implementation
 */

#include <stdlib.h>
#include <string.h>

/* for LDT */
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

#ifdef linux
# include <asm/ldt.h>
# include <asm/unistd.h>
#elif defined(__FreeBSD__)
# include <machine/segments.h>
# include <machine/sysarch.h>
#else
# error Sorry, LDT is not supported.
#endif

#include "pe_image.h"
#include "misc.h"

#include "w32api.h"
#include "module.h"

/* import dlls */
#include "kernel32.h"
#include "user32.h"
#include "advapi32.h"
#include "msvcrt.h"
#include "borlndmm.h"

#include "common.h"

static int load(PE_image *, char *);
static void *resolve(PE_image *, char *symbolname);
static void destroy(PE_image *);

static PE_image template = {
  load: load,
  resolve: resolve,
  destroy: destroy
};

PE_image *
peimage_create(void)
{
  PE_image *p;

  if ((p = calloc(1, sizeof(PE_image))) == NULL)
    return NULL;
  memcpy(p, &template, sizeof(PE_image));

  return p;
}

/* for internal use */

DEFINE_W32API(void, unknown_symbol, ())
{
  show_message("unknown symbol called\n");
}

static Symbol_info *
get_dll_symbols(char *dllname)
{
  int i;

  static struct {
    char *name;
    Symbol_info *(*get_symbols)(void);
  } name_to_func[] = {
    { "kernel32.dll", kernel32_get_export_symbols },
    { "user32.dll",   user32_get_export_symbols },
    { "advapi32.dll", advapi32_get_export_symbols },
    { "msvcrt.dll",   msvcrt_get_export_symbols },
    { "borlndmm.dll", borlndmm_get_export_symbols },
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

#if defined(linux)
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

#if defined(linux)
static int
install_fs(void)
{
  struct modify_ldt_ldt_s ldt;
  int fd, ret;
  void *prev;

  if (fs_installed)
    return 0;

  fd = open("/dev/zero", O_RDWR);
  if ((fs_seg = mmap((void *)0xbf000000, 0x30000, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0)) == NULL) {
    show_message("No enough memory for fs. Probably raising SIGSEGV...\n");
    return -1;
  }
  ldt.base_addr = ((int)fs_seg + 0xffff) & 0xffff0000;
  ldt.entry_number = 1;
  ldt.limit = ldt.base_addr + getpagesize() - 1;
  ldt.seg_32bit = 1;
  ldt.read_exec_only = 0;
  ldt.seg_not_present = 0;
  ldt.contents = MODIFY_LDT_CONTENTS_DATA;
  ldt.limit_in_pages = 0;
  if ((ret = modify_ldt(1, &ldt, sizeof(struct modify_ldt_ldt_s))) < 0) {
    perror("install_fs");
    show_message("modify_ldt() failed. Probably raising SIGSEGV...\n");
  }
  __asm__("movl $0xf,%eax\n\t"
	  "movw %ax,%fs\n\t");
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
#endif

#if 0
static int
uninstall_fs(void)
{
  if (fs_seg == NULL)
    return -1;
  munmap(fs_seg, 0x30000);
  fs_installed = 0;

  return 0;
}
#endif

/* methods */

static int
load(PE_image *p, char *path)
{
  FILE *fp;
  unsigned char dos_header[DOS_HEADER_SIZE];
  unsigned int pe_signature;
  unsigned int pe_header_start;
  int i;
#ifdef DEBUG
  static char *data_directory_names[] = {
    "Export", "Import", "Resource", "Exception", "Security", "BaseReloc",
    "Debug", "Copyright", "GlobalPtr", "TLS", "Load config", "Bound import", "IAT"
  };
#endif

  if ((fp = fopen(path, "rb")) == NULL) {
    show_message("PE_image: load: Cannot open %s\n", path);
    return 0;
  }
  p->filepath = path;

  fread(dos_header, 1, DOS_HEADER_SIZE, fp);
  pe_header_start = PE_HEADER_START(dos_header);
  debug_message("PE header found at %d\n", pe_header_start);

  fseek(fp, pe_header_start, SEEK_SET);
  fread(&pe_signature, 1, 4, fp);
  if (pe_signature != PE_SIGNATURE) {
    show_message("PE_image: load: Not PE file.\n");
    return 0;
  }

  fread(&p->pe_header, 1, PE_HEADER_SIZE, fp);
  switch (p->pe_header.Machine) {
  case 0x14c:
  case 0x14d:
  case 0x14e:
    break;
  default:
    show_message("PE_image: load: unsupported architecture %03X\n", p->pe_header.Machine);
    return 0;
  }

  debug_message("Section: %d sections\n", p->pe_header.NumberOfSections);
  if (p->pe_header.SizeOfOptionalHeader != OPTIONAL_HEADER_SIZE) {
    show_message("PE_image: load: Optional Header Size %d != %d\n", p->pe_header.SizeOfOptionalHeader, OPTIONAL_HEADER_SIZE);
    return 0;
  }

  fread(&p->opt_header, 1, OPTIONAL_HEADER_SIZE, fp);
  if (p->opt_header.Magic != OPTIONAL_SIGNATURE) {
    show_message("PE_image: load: PE file but corrupted optional header.\n");
    return 0;
  }
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
#ifdef DEBUG
  for (i = 0; i < 12; i++)
    if (p->opt_header.DataDirectory[i].Size)
      debug_message(" %s: 0x%08X %d\n",
	      data_directory_names[i],
	      p->opt_header.DataDirectory[i].VirtualAddress,
	      p->opt_header.DataDirectory[i].Size);
#endif

  if ((p->image = malloc(p->opt_header.SizeOfHeaders + p->opt_header.SizeOfImage)) == NULL) {
    show_message("No enough memory for image (%d bytes)\n",
		 p->opt_header.SizeOfImage + p->opt_header.SizeOfHeaders);
    return 0;
  }
  fseek(fp, 0, SEEK_SET);
  fread(p->image, 1, p->opt_header.SizeOfHeaders, fp);

  p->sect_headers = (IMAGE_SECTION_HEADER *)(p->image + pe_header_start + 4 + PE_HEADER_SIZE + OPTIONAL_HEADER_SIZE);
  for (i = 0; i < p->pe_header.NumberOfSections; i++) {
    debug_message("Section[%s]: RVA: 0x%08X File: 0x%08X Size: %d ", p->sect_headers[i].Name, p->sect_headers[i].VirtualAddress, p->sect_headers[i].PointerToRawData, p->sect_headers[i].SizeOfRawData);
    fseek(fp, p->sect_headers[i].PointerToRawData, SEEK_SET);
    fread(p->image + p->sect_headers[i].VirtualAddress, 1, p->sect_headers[i].SizeOfRawData, fp);
    debug_message("loaded at %d.\n", p->sect_headers[i].VirtualAddress);
  }

  /* export symbols */
  debug_message("-- Exported Symbols\n");
  p->export_symbols = hash_create(1024);
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

      hash_set(p->export_symbols, p->image + names[i], p->image + functions[name_ordinals[i]]);
      debug_message(" %s", p->image + names[i]);

      export_syminfo[i].name = p->image + names[i];
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
    for (i = 0; iid[i].u.OriginalFirstThunk; i++) {
      othunk = (IMAGE_THUNK_DATA *)(p->image + (int)iid[i].u.OriginalFirstThunk);
      thunk = (IMAGE_THUNK_DATA *)(p->image + (int)iid[i].FirstThunk);

      debug_message("Import DLL Name: %s\n", p->image + iid[i].Name);
      /* symbols in each DLL */
      for (j = 0; othunk[j].u1.AddressOfData; j++) {
	iibn = (IMAGE_IMPORT_BY_NAME *)(p->image + (int)othunk[j].u1.AddressOfData);
	/* import */
	if ((syminfo = get_dll_symbols(p->image + iid[i].Name)) != NULL) {
	  for (; ; syminfo++) {
	    if (syminfo->name) {
	      if (strcmp(iibn->Name, syminfo->name) == 0) {
		debug_message(" [%s]", iibn->Name);
		thunk[j].u1.Function = (FARPROC)syminfo->value;
		break;
	      }
	    } else {
	      thunk[j].u1.Function = (FARPROC)syminfo->value;
	      debug_message(" %s", iibn->Name);
	      break;
	    }
	  }
	} else {
	  thunk[j].u1.Function = (FARPROC)unknown_symbol;
	  debug_message(" %s", iibn->Name);
	}
      }
      debug_message("\n");
    }
  }

  /* relocation */
  debug_message("-- Relocation\n");
  if (p->opt_header.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size) {
    IMAGE_BASE_RELOCATION *ibr = (IMAGE_BASE_RELOCATION *)&p->image[p->opt_header.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress];
    int adjust = (int)(p->image - p->opt_header.ImageBase);

    while (ibr->VirtualAddress) {
      WORD *reloc = ibr->TypeOffset;
      int nrelocs = (ibr->SizeOfBlock - sizeof(DWORD) * 2) >> 1;

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
    DllEntryProc InitDll = (DllEntryProc)(p->image + p->opt_header.AddressOfEntryPoint);
    int result;

    debug_message("InitDll %p\n", InitDll);
    if ((result = InitDll((HMODULE)p->image, DLL_PROCESS_ATTACH, NULL)) != 1)
      show_message("InitDll returns %d\n", result);
  }

  debug_message("-- Load completed.\n");

  fclose(fp);

  return 1;
}

static void *
resolve(PE_image *p, char *symbolname)
{
  return hash_lookup(p->export_symbols, symbolname);
}

static void
destroy(PE_image *p)
{
  module_deregister(misc_basename(p->filepath));
  if (p->export_symbols)
    hash_destroy(p->export_symbols, 0);
  if (p->image)
    free(p->image);
  free(p);
}
