/*
 * spi.c -- spi to enfle bridge
 */

#include <stdlib.h>
#include <string.h>

#include "pe_image.h"
#include "spi-private.h"
#include "spi.h"
#include "loader-plugin.h"
#include "loader-extra.h"
#include "archiver-plugin.h"
#include "archiver-extra.h"

#include "common.h"

static LoaderStatus loader_identify(Image *, Stream *, void *);
static LoaderStatus loader_load(Image *, Stream *, void *);
static ArchiverStatus archiver_identify(Archive *, Stream *, void *);
static ArchiverStatus archiver_open(Archive *, Stream *, void *);

typedef struct _susie_loader {
  PE_image *pe;
  GetPictureInfoFunc get_pic_info;
  GetPictureFunc get_pic;
} SusieLoader;
  
typedef struct _susie_archiver {
  PE_image *pe;
  GetArchiveInfoFunc get_archive_info;
  GetFileInfoFunc get_file_info;
  GetFileFunc get_file;
} SusieArchiver;

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
  PictureInfo info;
  int err;

  debug_message("loader_identify() called\n");
  if ((err = sl->get_pic_info(st->path, 0, 0, &info)) == SPI_SUCCESS) {
    if (info.width <= 0 || info.height <= 0) {
      debug_message("Invalid dimension (%ld, %ld)\n", info.width, info.height);
      return LOAD_ERROR;
    } else if (info.colorDepth > 32) {
      debug_message("Invalid depth %d\n", info.colorDepth);
      return LOAD_ERROR;
    }
    debug_message("(%ld, %ld) depth %d\n", info.width, info.height, info.colorDepth);
    return LOAD_OK;
  }

  debug_message("loader_identify(): %s\n", spi_errormsg[err]);

  return LOAD_ERROR;
}

static LoaderStatus
loader_load(Image *p, Stream *st, void *priv)
{
  SusieLoader *sl = priv;
  unsigned char *image;
  BITMAPINFOHEADER *bih;
  int i, err, bpl;

  debug_message("loader_load() called\n");
  if ((err = sl->get_pic(st->path, 0, 0, (HANDLE *)&bih, (HANDLE *)&image, NULL, 0)) == SPI_SUCCESS) {
    p->type = _BGR24;
    p->width = bih->biWidth;
    p->height = bih->biHeight;
    p->depth = p->bits_per_pixel = bih->biBitCount;
    p->bytes_per_line = p->width * 3;
    bpl = (p->bytes_per_line + 3) & ~3;
    p->image_size = p->bytes_per_line * p->height;
    if ((p->image = malloc(p->image_size)) == NULL) {
      free(image);
      free(bih);
      show_message("No enough memory for image\n");
      return LOAD_ERROR;
    }
    for (i = p->height - 1; i >= 0; i--)
      memcpy(p->image + p->bytes_per_line * (p->height - 1 - i), image + bpl * i, p->bytes_per_line);
    free(image);
    free(bih);
    return LOAD_OK;
  }

  debug_message("loader_load(): %s\n", spi_errormsg[err]);

  return LOAD_ERROR;
}

static ArchiverStatus
archiver_identify(Archive *p, Stream *st, void *priv)
{
  show_message("archiver_identify() called\n");
  return OPEN_ERROR;
}

static ArchiverStatus
archiver_open(Archive *p, Stream *st, void *priv)
{
  show_message("archiver_open() called\n");
  return OPEN_ERROR;
}

static void
spi_plugin_exit(void *p)
{
  free(p);
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

  pe = peimage_create();
  if (!peimage_load(pe, path)) {
    show_message("peimage_load() failed: %s\n", path);
    return NULL;
  }

  if ((get_plugin_info = peimage_resolve(pe, "GetPluginInfo")) == NULL) {
    show_message("Cannot resolve GetPluginInfo.\n");
    goto error;
  }

  if ((err = get_plugin_info(0, buf, 256)) == 0) {
    show_message("GetPluginInfo returns 0\n");
    goto error;
  }

  switch (buf[2]) {
  case 'I':
    *type_return = ENFLE_PLUGIN_LOADER;
    if ((sl = calloc(1, sizeof(SusieLoader))) == NULL) {
      show_message("No enough memory for SusieLoader\n");
      goto error;
    }
    sl->pe = pe;
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

  if ((err = get_plugin_info(1, buf, 256)) == 0) {
    show_message("GetPluginInfo returns 0\n");
    exit(-1);
  }

  ep->name = strdup(path);
  ep->description = strdup(buf);
  ep->author = "SPI author";

  p = plugin_create();
  p->filepath = strdup(path);
  p->substance = ep;
  p->substance_unload = spi_plugin_exit;

  pl = eps->pls[ep->type];
  if (!pluginlist_add(pl, p ,ep->name)) {
    show_message("pluginlist_add failed: %s\n", ep->name);
    goto error;
  }

  return ep->name;

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
