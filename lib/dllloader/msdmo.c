/*
 * msdmo.c -- implementation of routines in msdmo.dll
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Apr 16 00:50:00 2004.
 * $Id: msdmo.c,v 1.1 2004/04/18 06:22:48 sian Exp $
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
#include <stdarg.h>

#include "w32api.h"
#include "module.h"

#include "utils/converter.h"
#include "pe_image.h"
#include "msdmo.h"

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

typedef struct __attribute__((__packed__)) {
    char unused[0x40];
    unsigned long  cbFormat; // 0x40
    char          *pbFormat; // 0x44
} MediaType;

DECLARE_W32API(HRESULT, MoInitMediaType, (MediaType *, DWORD));
DECLARE_W32API(HRESULT, MoFreeMediaType, (MediaType *));
DECLARE_W32API(HRESULT, MoCopyMediaType, (MediaType *, const MediaType *));
DECLARE_W32API(void, unknown_symbol, (void));

static Symbol_info symbol_infos[] = {
  { "MoInitMediaType", MoInitMediaType },
  { "MoFreeMediaType", MoFreeMediaType },
  { "MoCopyMediaType", MoCopyMediaType },
  { NULL, unknown_symbol }
};

DEFINE_W32API(HRESULT, MoInitMediaType,
	      (MediaType *dest, DWORD cbFormat))
{
  debug_message_fn("(%p, %d)\n", dest, cbFormat);

  if (!dest)
    return E_POINTER;
  memset(dest, 0, sizeof(*dest));
  if (cbFormat) {
    if ((dest->pbFormat = calloc(1, cbFormat)) == NULL)
      return E_OUTOFMEMORY;
  }

  return S_OK;
}

DEFINE_W32API(HRESULT, MoFreeMediaType,
	      (MediaType *dest))
{
  debug_message_fn("(%p)\n", dest);

  if (!dest)
    return E_POINTER;
  if (dest->pbFormat)
    free(dest->pbFormat);

  return S_OK;
}

DEFINE_W32API(HRESULT, MoCopyMediaType,
	      (MediaType *dest, const MediaType *src))
{
  debug_message_fn("(%p, %p)\n", dest, src);

  if (!dest || !src)
    return E_POINTER;
  memcpy(dest, src, sizeof(*dest));
  if (dest->cbFormat) {
    if ((dest->pbFormat = calloc(1, dest->cbFormat)) == NULL)
      return E_OUTOFMEMORY;
    memcpy(dest->pbFormat, src->pbFormat, dest->cbFormat);
  }

  return S_OK;
}

/* unimplemened */

DEFINE_W32API(void, unknown_symbol, (void))
{
  show_message("unknown symbol in msdmo called\n");
}

/* export */

Symbol_info *
msdmo_get_export_symbols(void)
{
  module_register("msdmo.dll", symbol_infos);
  return symbol_infos;
}
