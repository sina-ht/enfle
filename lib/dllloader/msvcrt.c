/*
 * msvcrt.c -- implementation of routines in msvcrt.dll
 */

#define W32API_REQUEST_MEM_ALLOC
#undef W32API_REQUEST_MEM_REALLOC
#define W32API_REQUEST_MEM_FREE
#include "mm.h"
#include "w32api.h"
#include "module.h"

#include "msvcrt.h"

#define REQUIRE_STRING_H
#include "compat.h"

#include "common.h"

static void unknown_symbol(void);
static void msvcrt_dllonexit(void *, void *);
static int msvcrt_initterm(void **, void **);
static void msvcrt_onexit(void *);
static int msvcrt_strcmpi(char *, char *);
static char *msvcrt_strrchr(char *, int);
static void *msvcrt_malloc(int);
static void msvcrt_free(void *);
static void *msvcrt_new(int);
static void msvcrt_delete(void *);

static Symbol_info symbol_infos[] = {
  { "__dllonexit", msvcrt_dllonexit },
  { "_initterm", msvcrt_initterm },
  { "_onexit", msvcrt_onexit },
  { "_strcmpi", msvcrt_strcmpi },
  { "strrchr", msvcrt_strrchr },
  { "malloc", msvcrt_malloc },
  { "free", msvcrt_free },
  { "??2@YAPAXI@Z", msvcrt_new }, /* Gee, what the hell is this fu**ing symbol names? */
  { "??3@YAXPAX@Z", msvcrt_delete },
  { NULL, unknown_symbol }
};

static void
msvcrt_dllonexit(void *arg1, void *arg2)
{
  debug_message("__dllonexit() called\n");
}

static int
msvcrt_initterm(void **arg1, void **arg2)
{
  BOOL (*initterm_func)(void);

  debug_message("_initterm(%p, %p) called\n", arg1, arg2);
  for (; arg1 < arg2; arg1++)
    if ((initterm_func = *arg1))
      initterm_func();

  return 0;
}

static void
msvcrt_onexit(void *arg)
{
  debug_message("_onexit() called\n");
}

static int
msvcrt_strcmpi(char *str1, char *str2)
{
  return strcasecmp(str1, str2);
}

static char *
msvcrt_strrchr(char *str, int c)
{
  debug_message("strrchr(%s, %c)\n", str, (char)c);
  /* XXX: hack hack hack */
  if (c == '\\')
    return strrchr(str, '/');
  return strrchr(str, c);
}

static void *
msvcrt_malloc(int size)
{
  void *p;

  p = w32api_mem_alloc(size);
  debug_message("malloc(%d) -> %p\n", size, p);

  return p;
}

static void *
msvcrt_new(int size)
{
  void *p;

  p = w32api_mem_alloc(size);
  debug_message("new(%d) -> %p\n", size, p);

  return p;
}

static void
msvcrt_free(void *p)
{
  debug_message("free(%p) called\n", p);
  w32api_mem_free(p);
}

static void
msvcrt_delete(void *p)
{
  debug_message("delete(%p) called\n", p);
  w32api_mem_free(p);
}

/* unimplemened */

static void
unknown_symbol(void)
{
  show_message("unknown symbol in msvcrt called\n");
}

/* export */

Symbol_info *
msvcrt_get_export_symbols(void)
{
  module_register("msvcrt.dll", symbol_infos);
  return symbol_infos;
}
