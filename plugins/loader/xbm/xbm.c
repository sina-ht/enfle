/*
 * xbm.c -- xbm loader plugin
 * (C)Copyright 1999, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Jul  1 21:48:11 2002.
 * $Id: xbm.c,v 1.2 2002/08/03 05:08:39 sian Exp $
 *
 * NOTE: This plugin is not optimized for speed.
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
#include <ctype.h>

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "enfle/stream-utils.h"
#include "enfle/loader-plugin.h"

#define XBM_BUFFER_SIZE 65536

DECLARE_LOADER_PLUGIN_METHODS;

static LoaderPlugin plugin = {
  type: ENFLE_PLUGIN_LOADER,
  name: "XBM",
  description: "XBM Loader plugin version 0.1",
  author: "Hiroshi Takekawa",
  image_private: NULL,

  identify: identify,
  load: load
};

ENFLE_PLUGIN_ENTRY(loader_xbm)
{
  LoaderPlugin *lp;

  if ((lp = (LoaderPlugin *)calloc(1, sizeof(LoaderPlugin))) == NULL)
    return NULL;
  memcpy(lp, &plugin, sizeof(LoaderPlugin));

  return (void *)lp;
}

ENFLE_PLUGIN_EXIT(loader_xbm, p)
{
  free(p);
}

/* for internal use */

static unsigned int
parse_define(char *s)
{
  char *p;

  if (strncmp(s, "#define", 7) != 0)
    return 0;

  p = s + 7;
  while (*p && isspace(*p))
    p++;
  if (!*p)
    return 0;
  while (*p && !isspace(*p))
    p++;
  if (!*p)
    return 0;
  while (*p && isspace(*p))
    p++;
  if (!*p)
    return 0;
  if (!isdigit(*p))
    return 0;

  return atoi(p);
}

static unsigned char
reverse_bit(unsigned char d)
{
  unsigned char r = 0;
  int i;

  for (i = 0; i < 8; i++) {
    r <<= 1;
    r |= (d & 1) ? 1 : 0;
    d >>= 1;
  }

  return r;
}

static int
load_image(Image *p, Stream *st)
{
  char s[XBM_BUFFER_SIZE];
  int c, c1;
  unsigned int i;
  unsigned char d, *ptr;

  for (;;) {
    if (stream_ngets(st, s, XBM_BUFFER_SIZE) == NULL) {
      debug_message("xbm: error(read1)\n");
      return 0;
    }
    if (s[0] == '#')
      break;
    if (!(s[0] == '/' || s[0] == '*' || isspace(s[0]))) {
      //debug_message("xbm: error(read1.1 %c)\n", s[0]);
      return 0;
    }
  }

  if ((image_width(p) = parse_define(s)) == 0) {
    debug_message("xbm: error(width)\n");
    return 0;
  }

  if (stream_ngets(st, s, XBM_BUFFER_SIZE) == NULL) {
    debug_message("xbm: error(read2)\n");
    return 0;
  }

  if ((image_height(p) = parse_define(s)) == 0) {
    debug_message("xbm: error(height)\n");
    return 0;
  }

  debug_message("xbm: (%dx%d)\n", image_width(p), image_height(p));

  while (stream_getc(st) != '{') ;

  p->depth = p->bits_per_pixel = 1;
  p->ncolors = 2;
  p->type = _BITMAP_MSBFirst;

  image_bpl(p) = (image_width(p) * p->bits_per_pixel + 7) >> 3;
  if (!image_image(p))
    if ((image_image(p) = memory_create()) == NULL) {
      show_message("xbm: No enough memory.\n");
      return 0;
    }
  if ((ptr = memory_alloc(image_image(p), image_bpl(p) * image_height(p))) == NULL) {
    show_message("xbm: No enough memory for image.\n");
    return 0;
  }

  for (i = 0; i < image_bpl(p) * image_height(p); i++) {
    while ((c = stream_getc(st)) >= 0 && c != '0') ;

    if (c < 0)
      return 0;

    if ((c = stream_getc(st)) != 'x') {
      show_message("not 0x. corrupted xbm file\n");
      return 0;
    }

    if ((c = stream_getc(st)) < 0) {
      show_message("got EOF. corrupted xbm file\n");
      return 0;
    }
    c = toupper(c);

    if ((c1 = stream_getc(st)) < 0) {
      show_message("got EOF. corrupted xbm file\n");
      return 0;
    }
    c1 = toupper(c1);

    if (isxdigit(c1)) {
      if (isxdigit(c)) {
	d = ((isdigit(c) ? c - '0' : c - 'A' + 10) << 4) +
	  (isdigit(c1) ? c1 - '0' : c1 - 'A' + 10);
      } else {
	show_message("Illegal hexadecimal. corrupted xbm file\n");
	return 0;
      }
    } else if (!isxdigit(c)) {
      show_message("Illegal hexadecimal. corrupted xbm file\n");
      return 0;
    } else
      d = isdigit(c) ? c - '0' : c - 'A' + 10;
    *ptr++ = reverse_bit(d);
  }

  return 1;
}

DEFINE_LOADER_PLUGIN_IDENTIFY(p, st, vw, c, priv)
{
  if (!load_image(p, st))
    return LOAD_NOT;

  return LOAD_OK;
}

DEFINE_LOADER_PLUGIN_LOAD(p, st, vw, c, priv)
{
  debug_message("xbm loader: load() called\n");

#ifdef IDENTIFY_BEFORE_LOAD
  {
    LoaderStatus status;

    if ((status = identify(p, st, vw, c, priv)) != LOAD_OK)
      return status;
    stream_rewind(st);
  }
#endif

  if (!load_image(p, st))
    return LOAD_ERROR;

  return LOAD_OK;
}
