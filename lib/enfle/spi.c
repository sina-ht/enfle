/*
 * spi.c -- spi to enfle bridge
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Jan 15 04:12:12 2001.
 * $Id: spi.c,v 1.11 2001/01/14 19:13:07 sian Exp $
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

#ifdef HAVE_CONFIG_H
# ifndef CONFIG_H_INCLUDED
#  include "enfle-config.h"
#  define CONFIG_H_INCLUDED
# endif
#endif

#ifdef USE_SPI

#include <stdlib.h>

#include "dllloader/pe_image.h"
#include "spi-private.h"
#include "spi.h"
#include "loader-plugin.h"
#include "loader-extra.h"
#include "archiver-plugin.h"
#include "archiver-extra.h"
#include "utils/misc.h"

#define REQUIRE_STRING_H
#include "compat.h"

#define REQUIRE_FATAL
#include "common.h"

static LoaderStatus loader_identify(Image *, Stream *, void *);
static LoaderStatus loader_load(Image *, Stream *, void *);
static ArchiverStatus archiver_identify(Archive *, Stream *, void *);
static ArchiverStatus archiver_open(Archive *, Stream *, void *);

typedef struct _susie_loader {
  PE_image *pe;
  IsSupportedFunc is_supported;
  GetPictureInfoFunc get_pic_info;
  GetPictureFunc get_pic;
} SusieLoader;

typedef struct _susie_archiver {
  PE_image *pe;
  IsSupportedFunc is_supported;
  GetArchiveInfoFunc get_archive_info;
  GetFileInfoFunc get_file_info;
  GetFileFunc get_file;
} SusieArchiver;

typedef struct _susie_archiver_info {
  GetFileFunc get_file;
  unsigned long position;
  unsigned long filesize;
} Susie_archiver_info;

static LoaderPlugin loader_template = {
  type: ENFLE_PLUGIN_LOADER,
  identify: loader_identify,
  load: loader_load
};

static ArchiverPlugin archiver_template = {
  type: ENFLE_PLUGIN_ARCHIVER,
  identify: archiver_identify,
  open: archiver_open
};

static LoaderStatus
loader_identify(Image *p, Stream *st, void *priv)
{
  SusieLoader *sl = priv;
#if 0
  PictureInfo info;
#endif
  int err;
  unsigned char buf[2048];

  memset(buf, 0, 2048);
  stream_read(st, buf, 2048);
  debug_message("loader_identify() called\n");
  if ((err = sl->is_supported(st->path, (DWORD)buf)) != SPI_SUCCESS)
    return LOAD_ERROR;

#if 0
  if (st->path)
    err = sl->get_pic_info(st->path, 0, 0, &info);
  else
    err = sl->get_pic_info(st->buffer, st->buffer_size, 1, &info);
  if (err == SPI_SUCCESS) {
    if (info.width <= 0 || info.height <= 0) {
      debug_message("Invalid dimension (%ld, %ld)\n", info.width, info.height);
      return LOAD_ERROR;
    } else if (info.colorDepth > 32 || info.colorDepth <= 0) {
      debug_message("Invalid depth %d\n", info.colorDepth);
      return LOAD_ERROR;
    }
    debug_message("(%ld, %ld) depth %d\n", info.width, info.height, info.colorDepth);
    return LOAD_OK;
  }
  debug_message("loader_identify(): %s\n", spi_errormsg[err]);

  return LOAD_ERROR;
#else
  return LOAD_OK;
#endif
}

static int PASCAL
susie_loader_progress_callback(int nNum, int nDenom, long lData)
{
  return 0;
}

static LoaderStatus
loader_load(Image *p, Stream *st, void *priv)
{
  SusieLoader *sl = priv;
  unsigned char *image, *d;
  BITMAPINFOHEADER *bih;
  int i, err, bpl;

  debug_message("loader_load() called\n");
  if ((err = sl->get_pic(st->path, 0, 0, (HANDLE *)&bih, (HANDLE *)&image,
			 susie_loader_progress_callback, 0)) == SPI_SUCCESS) {
    p->type = _BGR24;
    p->width = bih->biWidth;
    p->height = bih->biHeight;
    p->depth = p->bits_per_pixel = bih->biBitCount;
    p->bytes_per_line = p->width * 3;
    bpl = (p->bytes_per_line + 3) & ~3;

    if ((d = memory_alloc(p->image, p->bytes_per_line * p->height)) == NULL) {
      free(image);
      free(bih);
      show_message("No enough memory for image\n");
      return LOAD_ERROR;
    }
    for (i = p->height - 1; i >= 0; i--)
      memcpy(d + p->bytes_per_line * (p->height - 1 - i), image + bpl * i, p->bytes_per_line);
    free(image);
    free(bih);
    return LOAD_OK;
  }

  debug_message("loader_load(): %s\n", spi_errormsg[err]);

  return LOAD_ERROR;
}

static ArchiverStatus
archiver_identify(Archive *a, Stream *st, void *priv)
{
  SusieArchiver *sa = priv;
  unsigned char buffer[2048];

  debug_message("archiver_identify() called\n");

  memset(buffer, 0, 2048);
  stream_read(st, buffer, 2048);

  if (sa->is_supported(st->path, (DWORD)buffer))
    return OPEN_OK;

  return OPEN_ERROR;
}

static int PASCAL
susie_archive_progress_callback(int nNum, int nDenom, long lData)
{
    return 0;
}

static int
susie_archive_open(Archive *a, Stream *st, char *path)
{
  Susie_archiver_info *sai;
  unsigned char *dest;

  debug_message("susie_archive_open: %s: %s: %s\n", a->format, a->st->path, path);

  if ((sai = (Susie_archiver_info *)archive_get(a, path)) == NULL)
    return 0;

  if ((sai->get_file(a->st->path, sai->position, (LPSTR)&dest, 0x100,
		     susie_archive_progress_callback, 0)) != SPI_SUCCESS) {
    show_message("archive_open: GetFile() failed.\n");
    return 0;
  }

  return stream_make_memorystream(st, dest, sai->filesize);
}

static void
susie_archive_destroy(Archive *a)
{
  stream_destroy(a->st);
  hash_destroy(a->filehash, 1);
  free(a);
}

static ArchiverStatus
archiver_open(Archive *a, Stream *st, void *priv)
{
  int i, err;
  SusieArchiver *sa = priv;
  Susie_archiver_info *sai;
  fileInfo *info;

  if ((err = sa->get_archive_info(st->path, 0, 0, (HLOCAL *)&info)) != SPI_SUCCESS) {
    show_message("archive_open: %s: %s\n", st->path, spi_errormsg[err]);
    return OPEN_ERROR;
  }

  for (i = 0; info[i].method[0]; i++) {
    if ((sai = malloc(sizeof(Susie_archiver_info))) == NULL) {
      show_message("archiver_open: No enough memory.\n");
      free(info);
      return OPEN_ERROR;
    }
    sai->get_file = sa->get_file;
    sai->position = info[i].position;
    sai->filesize = info[i].filesize;
    archive_add(a, info[i].filename, (void *)sai);
  }

  free(info);

  a->st = stream_transfer(st);
  a->open = susie_archive_open;
  a->destroy = susie_archive_destroy;

  return OPEN_OK;
}

static void
spi_plugin_exit(void *p)
{
  EnflePlugin *ep = (EnflePlugin *)p;

  if (ep->name)
    free((void *)ep->name);
  if (ep->description)
    free((void *)ep->description);

  switch (ep->type) {
  case ENFLE_PLUGIN_LOADER:
    {
      LoaderPlugin *l = (LoaderPlugin *)p;
      SusieLoader *sl = l->private;

      peimage_destroy(sl->pe);
      free(sl);
    }
    break;
  case ENFLE_PLUGIN_ARCHIVER:
    {
      ArchiverPlugin *arp = (ArchiverPlugin *)p;
      SusieArchiver *sa = arp->private;

      peimage_destroy(sa->pe);
      free(sa);
    }
    break;
  default:
    show_message("spi_plugin_exit: inappropriate type %d\n", ep->type);
    break;
  }

  free(ep);
}

char *
spi_load(EnflePlugins *eps, char *path, PluginType *type_return)
{
  PE_image *pe;
  SusieLoader *sl = NULL;
  SusieArchiver *sa = NULL;
  GetPluginInfoFunc get_plugin_info;
  EnflePlugin *ep = NULL;
  LoaderPlugin *lp = NULL;
  ArchiverPlugin *ap = NULL;
  Plugin *p = NULL;
  PluginList *pl;
  char buf[256];
  int err;

  debug_message("spi_load %s...\n", path);
  pe = peimage_create();
  if (!peimage_load(pe, path)) {
    show_message("peimage_load() failed: %s\n", path);
    return NULL;
  }
  debug_message("OK\n");

  if ((get_plugin_info = peimage_resolve(pe, "GetPluginInfo")) == NULL) {
    show_message("Cannot resolve GetPluginInfo.\n");
    goto error;
  }

  debug_message("GetPluginInfo 0 ");
  if ((err = get_plugin_info(0, buf, 256)) == 0) {
    show_message("GetPluginInfo returns 0\n");
    goto error;
  }
  debug_message("OK\n");

  switch (buf[2]) {
  case 'I':
    *type_return = ENFLE_PLUGIN_LOADER;
    if ((sl = calloc(1, sizeof(SusieLoader))) == NULL) {
      show_message("No enough memory for SusieLoader\n");
      goto error;
    }
    sl->pe = pe;
    if ((sl->is_supported = peimage_resolve(pe, "IsSupported")) == NULL) {
      show_message("Cannot resolve IsSupported.\n");
      goto error;
    }
    if ((sl->get_pic_info = peimage_resolve(pe, "GetPictureInfo")) == NULL) {
      show_message("Cannot resolve GetPictureInfo.\n");
      goto error;
    }
    if ((sl->get_pic = peimage_resolve(pe, "GetPicture")) == NULL) {
      show_message("Cannot resolve GetPicture.\n");
      goto error;
    }
    if ((lp = calloc(1, sizeof(LoaderPlugin))) == NULL) {
      show_message("No enough memory for LoaderPlugin\n");
      goto error;
    }
    memcpy(lp, &loader_template, sizeof(LoaderPlugin));
    lp->private = sl;
    ep = (EnflePlugin *)lp;
    break;
  case 'X':
    *type_return = ENFLE_PLUGIN_SAVER;
    show_message("Export filter is not supported yet.\n");
    return NULL;
  case 'A':
    *type_return = ENFLE_PLUGIN_ARCHIVER;
    if ((sa = calloc(1, sizeof(SusieArchiver))) == NULL) {
      show_message("No enough memory for SusieArchiver\n");
      return NULL;
    }
    sa->pe = pe;
    if ((sa->is_supported = peimage_resolve(pe, "IsSupported")) == NULL) {
      show_message("Cannot resolve IsSupported.\n");
      goto error;
    }
    if ((sa->get_archive_info = peimage_resolve(pe, "GetArchiveInfo")) == NULL) {
      show_message("Cannot resolve GetArchiveInfo.\n");
      return NULL;
    }
    if ((sa->get_file_info = peimage_resolve(pe, "GetFileInfo")) == NULL) {
      show_message("Cannot resolve GetFileInfo.\n");
      return NULL;
    }
    if ((sa->get_file = peimage_resolve(pe, "GetFile")) == NULL) {
      show_message("Cannot resolve GetFile.\n");
      return NULL;
    }
    if ((ap = calloc(1, sizeof(ArchiverPlugin))) == NULL) {
      show_message("No enough memory for ArchiverPlugin\n");
      goto error;
    }
    memcpy(ap, &archiver_template, sizeof(ArchiverPlugin));
    ap->private = sa;
    ep = (EnflePlugin *)ap;
    break;
  default:
    show_message("Unknown susie plugin type %c.\n", buf[2]);
    break;
  }

  debug_message("GetPluginInfo 1 ");
  if ((err = get_plugin_info(1, buf, 256)) == 0)
    fatal(1, "GetPluginInfo returns 0\n");
  debug_message("OK\n");

  ep->name = strdup(misc_basename(path));
  ep->description = strdup(buf);
  ep->author = (char *)"SPI author";

  p = plugin_create();
  p->filepath = strdup(path);
  p->substance = ep;
  p->substance_unload = spi_plugin_exit;

  pl = eps->pls[ep->type];
  if (!pluginlist_add(pl, p ,ep->name)) {
    show_message("pluginlist_add failed: %s\n", ep->name);
    goto error;
  }

  return (char *)ep->name;

 error:
  peimage_destroy(pe);
  if (p)
    plugin_destroy(p);
  if (sl)
    free(sl);
  if (sa)
    free(sa);
  if (lp)
    free(lp);
  if (ap)
    free(ap);

  return NULL;
}

#endif
