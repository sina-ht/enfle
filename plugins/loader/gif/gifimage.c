/*
 * gifimage.c -- GIF image block parser and decoder
 * (C)Copyright 1998, 99 by Hiroshi Takekawa
 * Last Modified: Wed Jul 10 22:21:12 2002.
 *
 * NOTE: This code includes LZW decoder
 */

#include "compat.h"
#include "common.h"

#include "giflib.h"
#include "gifbitio.h"
#include "enfle/stream-utils.h"

int
GIFDecodeImage(Stream *st, GIF_info *info)
{
  GIF_image *image = info->image;
  GIF_id *id = image->id;
  int c, i, dataindex, treesize, rootnumber;
  int length; /* bit length */
  int pass = 1; /* interlace pass */
  int width = id->width;
  int height = id->height;
  int w = 0, h = 0; /* interlace width, height keeper */
  unsigned char topdata = 0, *d, *top;

  static LZWtree tree[LZWMAXCODE];
  LZWtree *p, *q;

  static unsigned char data[LZWDATAMAXLEN];
  static int max[13] = {0, 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047, 4095};

  debug_message_fn("() called\n");

  /* allocate memory for image data */
  if ((top = d = image->data = (unsigned char *)malloc(width * (height + 1))) == NULL) {
    info->err = (char *)"No enough memory for image data";
    return PARSE_ERROR;
  }

  /* fill out image with background color */
  memset(image->data, info->sd->back, width * height);

  /* get initial bit length */
  /* NOTE: actual bit length is this value + 1 */
  image->initial_length = stream_getc(st);
  gif_bitio_init(st);

  /* determine number of root */
  rootnumber = 1 << image->initial_length;

  /* initialize tree */
  for (i = 0; i < rootnumber; i++) {
    tree[i].parent = (LZWtree *)NULL;
    tree[i].datum  = (unsigned char)i;
  }
  treesize = rootnumber + 2;
  length = image->initial_length + 1;
  q = NULL;

  /* decode loop */
  while ((c = getbits(length)) >= 0) {
    if (c == rootnumber) {
      /* this is clear code */
      treesize = rootnumber + 2;
      length = image->initial_length + 1;
      q = NULL;
    } else if (c == rootnumber + 1) {
      /* this is end code */
      break;
    } else {
      dataindex = LZWDATAMAXLEN - 1;
      if (treesize < c) {
	/* invalid data stream */
	info->err = (char *)"Invalid data stream";
	break;
      } else if (treesize == c) {
	/* not yet exist in a tree */
	data[dataindex--] = topdata;
        p = q;
      }	else {
	p = &tree[c];
      }

      for (; p != NULL; p = p->parent)
	data[dataindex--] = p->datum;
      topdata = data[++dataindex];

      for (i = dataindex; i < LZWDATAMAXLEN; i++)
	if (!id->interlace)
	  *d++ = data[i];
	else {
	  *d++ = data[i];
	  w++;
	  if (w == width) {
	    w = 0;
	    switch (pass) {
	    case 1:
	      h += 8;
	      if (h >= height) {
		h = 4;
		d = top + width * 4;
		pass++;
	      } else {
		d += 7 * width;
	      }
	      break;
	    case 2:
	      h += 8;
	      if (h >= height) {
		h = 2;
		d = top + width * 2;
		pass++;
	      } else {
		d += 7 * width;
	      }
	      break;
	    case 3:
	      h += 4;
	      if (h >= height) {
		h = 1;
		d = top + width;
		pass++;
	      } else {
		d += 3 * width;
	      }
	      break;
	    case 4:
	      h += 2;
		d += width;
	      break;
	    }
	  }
	}

      if (q != NULL && treesize < LZWMAXCODE) {
	tree[treesize].parent = q;
	tree[treesize].datum = topdata;
	treesize++;
      }
      if (treesize > max[length] && length < LZWMAXLEN)
	length++;
      q = &tree[c];
    }
  }
  if (stream_getc(st) != 0) {
    debug_message("Invalid image terminator at %lX.\n", stream_tell(st));
    warning("Image may be corrupted.\n");
  }

  if (image->gc == NULL) {
    if ((image->gc = (GIF_gc *)calloc(1, sizeof(GIF_gc))) == NULL) {
      info->err = (char *)"No enought memory for GIF_gc";
      return PARSE_ERROR;
    }
  }
  return PARSE_OK;
}

int GIFParseImageBlock(Stream *st, GIF_info *info)
{
  unsigned char c[9];
  GIF_id *id;
  char *tmp;

  if ((tmp = (char *)calloc(1, sizeof(GIF_image))) == NULL) {
    info->err = (char *)"No enough memory for image block";
    return PARSE_ERROR;
  }

  if (info->top == NULL)
    info->top = info->image = (GIF_image *)tmp;
  else {
    info->image->next = (GIF_image *)tmp;
    info->image = info->image->next;
    info->image->next = (GIF_image *)NULL;
  }
  info->image->gc = info->recent_gc;
  if ((info->image->id = id = calloc(1, sizeof(GIF_id))) == NULL) {
    info->err = (char *)"No enough memory for image descriptor";
    return PARSE_ERROR;
  }

  stream_read(st, c, 9);
  id->left   = (c[1] << 8) + c[0];
  id->top    = (c[3] << 8) + c[2];
  id->width  = (c[5] << 8) + c[4];
  id->height = (c[7] << 8) + c[6];
  id->lct_follows = c[8] & (1 << 7);
  id->interlace = c[8] & (1 << 6);
  id->lct_sorted = c[8] & (1 << 5);
  id->depth = (c[8] & 7) + 1;

  debug_message_fnc("(%d, %d) - (%d, %d)\n",
		    id->left, id->top, id->left + id->width, id->top + id->height);

  /* load local color table */
  if (id->lct_follows) {
    int i, ncell;
    GIF_ct *lct;
    GIF_ccell *lccell;

    debug_message_fnc("Load local color table\n");

    if ((id->lct = lct = (GIF_ct *)malloc(sizeof(GIF_ct))) == NULL) {
      info->err = (char *)"No enough memory for local color table";
      return PARSE_ERROR;
    }

    ncell = lct->nentry = 1 << id->depth;
    if ((lccell = (GIF_ccell *)malloc(sizeof(GIF_ccell) * ncell)) == NULL) {
      info->err = (char *)"No enough memory for local color cell";
      return PARSE_ERROR;
    }

    if ((lct->cell = (GIF_ccell **)malloc(sizeof(GIF_ccell *) * ncell)) == NULL) {
      info->err = (char *)"No enough memory for local color cell pointer";
      return PARSE_ERROR;
    }

    for (i = 0; i < lct->nentry; i++)
      lct->cell[i] = lccell + i;

    stream_read(st, (unsigned char *)lccell, sizeof(GIF_ccell) * ncell);
  }

  return GIFDecodeImage(st, info);
}
