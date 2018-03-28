/*
 * advapi32.c -- implementation of routines in advapi32.dll
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Apr 18 14:44:53 2004.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#define REQUIRE_STRING_H
#define REQUIRE_UNISTD_H
#include "compat.h"
#include "common.h"

#include <sys/stat.h>
#include <fcntl.h>

#include "w32api.h"
#include "module.h"

#include "advapi32.h"

#define HKEY_CLASSES_ROOT       ((HKEY)0x80000000)
#define HKEY_CURRENT_USER       ((HKEY)0x80000001)
#define HKEY_LOCAL_MACHINE      ((HKEY)0x80000002)
#define HKEY_USERS              ((HKEY)0x80000003)
#define HKEY_PERFORMANCE_DATA   ((HKEY)0x80000004)
#define HKEY_CURRENT_CONFIG     ((HKEY)0x80000005)
#define HKEY_DYN_DATA           ((HKEY)0x80000006)

#define ERROR_MORE_DATA         234

#define REG_CREATED_NEW_KEY     0x00000001

#define REG_NONE                0    /* no type */
#define REG_SZ                  1    /* string type (ASCII) */
#define REG_EXPAND_SZ           2    /* string, includes %ENVVAR% (expanded by caller) (ASCII) */
#define REG_BINARY              3    /* binary format, callerspecific */
#define REG_DWORD               4    /* DWORD in little endian format */
#define REG_DWORD_LITTLE_ENDIAN 4    /* DWORD in little endian format */
#define REG_DWORD_BIG_ENDIAN    5    /* DWORD in big endian format  */
#define REG_LINK                6    /* symbolic link (UNICODE) */
#define REG_MULTI_SZ            7    /* multiple strings, delimited by \0, terminated by \0\0 (ASCII) */
#define REG_RESOURCE_LIST       8    /* resource list? huh? */
#define REG_FULL_RESOURCE_DESCRIPTOR    9       /* full resource descriptor */
#define REG_RESOURCE_REQUIREMENTS_LIST  10

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

/* registry implementation ported from avifile */

//char *regpathname = NULL;
static char *localregpathname = NULL;

typedef struct reg_handle_s {
  HKEY handle;
  char *name;
  struct reg_handle_s *next;
  struct reg_handle_s *prev;
} reg_handle_t;

struct reg_value {
  int type;
  char *name;
  int len;
  char *value;
};

static struct reg_value *regs = NULL;
static int reg_size;
static reg_handle_t *head = NULL;

#define DIR -25

static void open_registry(void);
static void save_registry(void);
static void init_registry(void);

static void
create_registry(void)
{
  if (regs) {
    err_message_fnc("Logic error: create_registry() called with existing registry\n");
    save_registry();
    return;
  }

  regs = (struct reg_value *)malloc(3 * sizeof(*regs));

  regs[0].type = DIR;
  regs[0].name = (char *)malloc(5);
  regs[0].value = NULL;
  regs[0].len = 0;
  strcpy(regs[0].name, "HKLM");

  regs[1].type = DIR;
  regs[1].name = (char *)malloc(5);
  regs[1].value = NULL;
  regs[1].len = 0;
  strcpy(regs[1].name, "HKCU");

  reg_size = 2;
  head = 0;
  save_registry();
}

static void
open_registry(void) {
  int i, fd;
  unsigned int len;

  if (regs) {
    err_message_fnc("Registry has already been opened.\n");
    return;
  }

  fd = open(localregpathname, O_RDONLY);
  if (fd == -1) {
    debug_message_fnc("Creating new registry\n");
    create_registry();
    return;
  }

  read(fd, &reg_size, 4);
  regs = (struct reg_value *)malloc(reg_size * sizeof(*regs));
  head = 0;
  for (i = 0; i < reg_size; i++) {
    read(fd, &regs[i].type, 4);
    read(fd, &len, 4);
    regs[i].name = (char *)malloc(len + 1);
    if (regs[i].name == 0) {
      reg_size = i + 1;
      goto error;
    }
    read(fd, regs[i].name, len);
    regs[i].name[len] = 0;
    read(fd, &regs[i].len, 4);
    regs[i].value = (char *)malloc(regs[i].len + 1);
    if (regs[i].value == 0) {
      free(regs[i].name);
      reg_size = i + 1;
      goto error;
    }
    read(fd, regs[i].value, regs[i].len);
    regs[i].value[regs[i].len] = 0;
  }
 error:
  close(fd);
  return;
}

static void
save_registry(void)
{
  int i, fd;

  if (!regs)
    init_registry();

  fd = open(localregpathname, O_WRONLY | O_CREAT, 00666);
  if (fd == -1)	{
    err_message_fnc("Failed to open registry file '%s' for writing.\n", localregpathname);
    return;
  }

  write(fd, &reg_size, 4);
  for (i = 0; i < reg_size; i++) {
    unsigned len = strlen(regs[i].name);

    write(fd, &regs[i].type, 4);
    write(fd, &len, 4);
    write(fd, regs[i].name, len);
    write(fd, &regs[i].len, 4);
    write(fd, regs[i].value, regs[i].len);
  }
  close(fd);
}

#if 0
static void
free_registry(void)
{
  reg_handle_t *t = head;

  while (t) {
    reg_handle_t *f = t;

    if (t->name)
      free(t->name);
    t = t->prev;
    free(f);
  }

  head = 0;
  if (regs) {
    int i;

    for (i = 0; i<reg_size; i++) {
      free(regs[i].name);
      free(regs[i].value);
    }
    free(regs);
    regs = 0;
  }

  if (localregpathname && localregpathname != regpathname)
    free(localregpathname);
  localregpathname = 0;
}
#endif

static struct reg_value *
find_value_by_name(const char *name)
{
  int i;

  for(i=0; i<reg_size; i++)
    if(!strcmp(regs[i].name, name))
      return regs+i;

  return 0;
}

static reg_handle_t *
find_handle(HKEY handle)
{
  reg_handle_t* t;

  for (t = head; t; t = t->prev) {
    if (t->handle == handle)
      return t;
  }

  return 0;
}

static HKEY
generate_handle(void)
{
  static int zz = 249;

  zz++;
  while ((zz == (int)HKEY_LOCAL_MACHINE) || (zz == (int)HKEY_CURRENT_USER))
    zz++;

  return (HKEY)zz;
}

static reg_handle_t *
insert_handle(HKEY handle, const char *name)
{
  reg_handle_t *t;

  t = (reg_handle_t *)malloc(sizeof(*t));
  if (head == 0)
    t->prev = 0;
  else {
    head->next = t;
    t->prev = head;
  }
  t->next = 0;
  t->name = (char *)malloc(strlen(name) + 1);
  strcpy(t->name, name);
  t->handle = handle;
  head = t;

  return t;
}

static char *
build_keyname(HKEY key, const char *subkey)
{
  char *full_name;
  reg_handle_t *t;

  if ((t = find_handle(key)) == 0) {
    debug_message_fnc("Invalid key\n");
    return NULL;
  }
  if (subkey == NULL)
    subkey = "<default>";
  full_name = (char *)malloc(strlen(t->name) + strlen(subkey) + 10);
  strcpy(full_name, t->name);
  strcat(full_name, "\\");
  strcat(full_name, subkey);

  return full_name;
}

static struct reg_value *
insert_reg_value(HKEY handle, const char* name, int type, const void* value, int len)
{
  struct reg_value *v;
  char* fullname;

  if ((fullname = build_keyname(handle, name)) == NULL) {
    debug_message_fnc("Invalid handle\n");
    return NULL;
  }

  if ((v = find_value_by_name(fullname)) == 0) {
    // creating new value in registry
    if (regs == 0)
      create_registry();
    regs = (struct reg_value *)realloc(regs, sizeof(struct reg_value) * (reg_size + 1));
    v = regs + reg_size;
    reg_size++;
  } else {
    // replacing old one
    free(v->value);
    free(v->name);
  }
  v->type = type;
  v->len = len;
  v->value = (char *)malloc(len);
  memcpy(v->value, value, len);
  v->name = (char*)malloc(strlen(fullname) + 1);
  strcpy(v->name, fullname);
  free(fullname);
  save_registry();

  return v;
}

static void
init_registry(void)
{
  debug_message_fnc("Initializing registry\n");
#if 1
  localregpathname = (char *)malloc(strlen(getenv("HOME")) + 22);
  sprintf(localregpathname, "%s/.enfle/win32registry", getenv("HOME"));
#else
  if (localregpathname == 0) {
    const char *pthn = regpathname;
    if (!regpathname) {
      // avifile - for now reading data from user's home
      struct passwd *pwent;
      pwent = getpwuid(geteuid());
      pthn = pwent->pw_dir;
    }
    localregpathname = (char*)malloc(strlen(pthn) + 20);
    strcpy(localregpathname, pthn);
    strcat(localregpathname, "/.registry");
  }
#endif

  open_registry();
  insert_handle(HKEY_LOCAL_MACHINE, "HKLM");
  insert_handle(HKEY_CURRENT_USER, "HKCU");
}

DEFINE_W32API(long, RegOpenKeyA,
	      (long handle, LPCSTR name, int *key_return))
{
  //debug_message_fn("(%p, %s) called -> RegOpenKeyExA\n", (void *)handle, name);
  return RegOpenKeyExA((HKEY)handle, name, 0, 0, (LPHKEY)key_return);
}

DEFINE_W32API(DWORD, RegOpenKeyExA,
	      (HKEY key, LPCSTR subkey, DWORD reserved, REGSAM access, LPHKEY newkey))
{
  char *full_name;
  reg_handle_t *t;
  struct reg_value *v;

  //debug_message_fn("(%p, %s, %p) called\n", key, subkey, newkey);

  if (!regs)
    init_registry();
  full_name = build_keyname(key, subkey);
  if (!full_name)
    return -1;
  //debug_message_fnc("Opening key Fullname %s\n", full_name);
  v = find_value_by_name(full_name);

  t = insert_handle(generate_handle(), full_name);
  *newkey = t->handle;
  free(full_name);

  return 0;
}

DEFINE_W32API(DWORD, RegCloseKey,
	      (HKEY key))
{
  reg_handle_t *handle;

  //debug_message_fn("(%p) called\n", handle);

  if (key == HKEY_LOCAL_MACHINE)
    return 0;
  if (key == HKEY_CURRENT_USER)
    return 0;
  handle = find_handle(key);
  if (handle == 0)
    return 0;
  if (handle->prev)
    handle->prev->next = handle->next;
  if (handle->next)
    handle->next->prev = handle->prev;
  if (handle->name)
    free(handle->name);
  if (handle == head)
    head = head->prev;
  free(handle);

  return 1;
}

DEFINE_W32API(DWORD, RegQueryValueExA,
	      (HKEY key, LPCSTR value, LPDWORD reserved, LPDWORD type,
	       LPBYTE data, LPDWORD count))
{
  struct reg_value* t;
  char* c;

  //debug_message_fn("(%p, %s, %p, %p, %p) called\n", key, value, type, data, count);

  if (!regs)
    init_registry();

  c = build_keyname(key, value);
  if (!c)
    return 1;

  t = find_value_by_name(c);
  free(c);
  if (t == 0)
    return 2;

  if (type)
    *type = t->type;

  if (data) {
    memcpy(data, t->value, (t->len < *count) ? t->len : *count);
    //debug_message_fnc("returning %d bytes: %d\n", t->len, *(int*)data);
  }

  if (*count < t->len) {
    *count = t->len;
    return ERROR_MORE_DATA;
  }
  *count = t->len;

  return 0;
}

DEFINE_W32API(DWORD, RegCreateKeyExA,
	      (HKEY key, LPCSTR name, DWORD reserved, LPSTR class, DWORD options,
	       REGSAM access, SECURITY_ATTRIBUTES *sa, LPHKEY newkey, LPDWORD status))
{
  reg_handle_t *t;
  char *fullname;
  struct reg_value *v;

  //debug_message_fn("(%p, %s) called\n", key, name);

  if (!regs)
    init_registry();

  fullname = build_keyname(key, name);
  if (!fullname)
    return 1;

  //debug_message_fnc("Creating/Opening key %s\n", fullname);

  v = find_value_by_name(fullname);
  if (v == 0) {
    int qw = 45708;
    v = insert_reg_value(key, name, DIR, &qw, 4);
    if (status)
      *status = REG_CREATED_NEW_KEY;
    // return 0;
  }

  t = insert_handle(generate_handle(), fullname);
  *newkey = t->handle;
  free(fullname);

  return 0;
}

DEFINE_W32API(DWORD, RegSetValueExA,
	      (HKEY key, LPCSTR name, DWORD reserved, DWORD type,
	       CONST BYTE *data, DWORD count))
{
  char *c;

  //debug_message_fn("(%p, %s) called\n", key, name);

  if (!regs)
    init_registry();

  c = build_keyname(key, name);
  if (c == NULL)
    return 1;
  insert_reg_value(key, name, type, data, count);
  free(c);

  return 0;
}

DEFINE_W32API(DWORD, RegDeleteKeyA, (HKEY handle, LPCSTR name))
{
  //debug_message_fn("(%p, %s) called\n", handle, name);
  return 1;
}

DEFINE_W32API(DWORD, GetUserNameA,
	      (LPSTR a, LPDWORD b))
{
  debug_message_fn("(%p) called\n", a);
  return 0;
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
