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

#include "common.h"

static void unknown_symbol(void);
static int msvcrt_initterm(void *, void *);
static void *msvcrt_malloc(int);
static void *msvcrt_new(int);
static void msvcrt_free(void *);
static void msvcrt_delete(void *);

static Symbol_info symbol_infos[] = {
  { "_initterm", msvcrt_initterm },
  { "malloc", msvcrt_malloc },
  { "free", msvcrt_free },
  { "??2@YAPAXI@Z", msvcrt_new }, /* Gee, what the hell is this fu**ing symbol names? */
  { "??3@YAXPAX@Z", msvcrt_delete },
  { NULL, unknown_symbol }
};

static int
msvcrt_initterm(void *arg1, void *arg2)
{
  /* debug_message("_initterm(%p, %p) called\n", arg1, arg2); */
  return 0;
}

static void *
msvcrt_malloc(int size)
{
  debug_message("malloc(%d) called\n", size);
  return w32api_mem_alloc(size);
}

static void *
msvcrt_new(int size)
{
  debug_message("new(%d) called\n", size);
  return w32api_mem_alloc(size);
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
