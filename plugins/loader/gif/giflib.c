/*
 * giflib.c -- GIF manipulation library
 * (C)Copyright 1997, 98, 99 by Hiroshi Takekawa
 * Last Modified: Wed Jul 10 22:21:39 2002.
 * $Id: giflib.c,v 1.1 2002/08/03 05:10:17 sian Exp $
 *
 *  This is GIF manipulation library.
 *  This can read GIF file into raster memory image.
 *
 *             The Graphics Interchange Format(c) is
 *       the Copyright property of CompuServe Incorporated.
 * GIF(sm) is a Service Mark property of CompuServe Incorporated.
 *
 */

#include "compat.h"
#include "common.h"

#include "giflib.h"
#include "enfle/stream-utils.h"

/*
 * GIF_sd *GIFReadScreenDescriptor(Stream *st, GIF_info *info)
 * Arguments
 *  Stream *st : opened stream structure
 * Return
 *  pointer to ScreenDescriptor, fail if NULL.
 * Desctiption
 *  This function reads GIF file and stores an info. of GIF file.
 */
int
GIFReadScreenDescriptor(Stream *st, GIF_info *info)
{
  int i;
  unsigned char buf[7];
  GIF_sd *sd;

  if ((info->sd = sd = (GIF_sd *)calloc(1, sizeof(GIF_sd))) == NULL) {
    info->err = (char *)"No enough memory for screen descriptor";
    return NOENOUGHMEM;
  }

  stream_read(st, buf, 7);
  sd->width        = (buf[1] << 8) + buf[0];
  sd->height       = (buf[3] << 8) + buf[2];
  sd->gct_follows  = (buf[4] & (1 << 7));
  sd->resolution   = ((buf[4] >> 4) & 7) + 1;
  sd->gct_sorted   = (buf[4] & (1 << 3));
  sd->depth        = (buf[4] & 7) + 1;
  sd->back         = sd->gct_follows ? buf[5] : 0;
  sd->aspect_ratio = buf[6];

  /* load global color table */
  if (sd->gct_follows) {
    GIF_ct *gct;
    GIF_ccell *gccell;
    int ncell;

    if ((sd->gct = gct = (GIF_ct *)malloc(sizeof(GIF_ct))) == NULL) {
      info->err = (char *)"No enough memory for global color table";
      return NOENOUGHMEM;
    }

    ncell = gct->nentry = 1 << sd->depth;
    if ((gccell = (GIF_ccell *)malloc(sizeof(GIF_ccell) * ncell)) == NULL) {
      info->err = (char *)"No enough memory for global color cell";
      return NOENOUGHMEM;
    }

    if ((gct->cell = (GIF_ccell **)malloc(sizeof(GIF_ccell *) * ncell)) == NULL) {
      info->err = (char *)"No enough memory for global color cell pointer";
      return NOENOUGHMEM;
    }

    for (i = 0; i < ncell; i++)
      gct->cell[i] = gccell + i;

    stream_read(st, (unsigned char *)gccell, sizeof(GIF_ccell) * ncell);
  }

  return SUCCESS;
}

/*
 * GIF_info *GIFReadSignature(Stream *st, int *c);
 * Arguments
 *  Stream *st : GIF stream structure
 * Return
 *  Pointer to GIF_info structure
 *  c:
 *  NOTGIFFILE   : given file is not GIF file
 *  NOTSUPPORTED : given file is not supported format
 *  NOENOUGHMEM  : cannot allocate memory for information
 *       0       : ok this file is supported
 * Description
 *  This function determines whether if a given file is supported or not.
 *  You must give opened stream structure.
 */
GIF_info *
GIFReadSignature(Stream *st, int *c)
{
  GIF_info *info;
  char sig[3];

  stream_read(st, sig, 3);
  if (memcmp(sig, "GIF", 3) != 0) {
    *c = NOTGIFFILE;
    return NULL;
  }

  stream_read(st, sig, 3);
  /* allocate memory for information. */
  info = (GIF_info *)calloc(1, sizeof(GIF_info));
  if (info == NULL) {
    *c = NOENOUGHMEM;
    return NULL;
  }

  if (memcmp(sig, "87a", 3) == 0) {
    *c = SUCCESS;
    info->revision = 87;
  } else if (memcmp(sig, "89a", 3) == 0) {
    *c = SUCCESS;
    info->revision = 89;
  } else {
    *c = NOTSUPPORTED;
    return NULL;
  }

  info->comment = info->applcode = info->err = (char *)NULL;
  info->sd = (GIF_sd *)NULL;
  info->top = info->image = (GIF_image *)NULL;

  return info;
}

static void
gif_free(void *ptr)
{
  if (ptr != NULL)
    free(ptr);
}

void
GIFDestroyData(GIF_info *info)
{
  GIF_image *img, *tmp;

  gif_free(info->comment);
  gif_free(info->applcode);

  if (info->sd->gct != NULL) {
    gif_free(info->sd->gct->cell[0]);
    gif_free(info->sd->gct->cell);
  }

  gif_free(info->sd->gct);
  gif_free(info->sd);
  img = info->top;

  while (img != NULL) {
    gif_free(img->data);
    if (img->id->lct != NULL) {
      gif_free(img->id->lct->cell[0]);
      gif_free(img->id->lct->cell);
    }
    gif_free(img->id->lct);
    gif_free(img->id);
    gif_free(img->gc);
    tmp = img->next;
    gif_free(img);
    img = tmp;
  }

  gif_free(info);
}

int
GIFParseNextBlock(Stream *st, GIF_info *info)
{
  int c;
  char buf[512];

  c = stream_getc(st);
  debug_message_fnc("%02X at %lX\n", c, stream_tell(st));

  switch (c) {
  case IMAGE:
    c = GIFParseImageBlock(st, info);
    info->npics++;
    break;
  case TRAILER:
    debug_message_fnc("got trailer\n");
    return PARSE_END;
  case EXTENSION:
    c = GIFParseExtensionBlock(st, info);
    break;
  case EOF:
    debug_message_fnc("got EOF before trailer\n");
    return PARSE_END;
  default:
    if (c) {
      sprintf(buf, "Unknown code %02X: Image, extension or trailer expected", (unsigned char)c);
      info->err = buf;
      return PARSE_ERROR;
    }
    return PARSE_OK;
  }
  return c;
}
