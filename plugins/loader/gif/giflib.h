/*
 * giflib.h -- GIF manipulation library header
 * (C)Copyright 1997,98 by Hiroshi Takekawa <t80679@hongo.ecc.u-tokyo.ac.jp>
 * Last Modified: Sat Oct 19 11:26:15 2002.
 * $Id: giflib.h,v 1.2 2002/11/06 14:11:34 sian Exp $
 */

#include "enfle/stream.h"

#ifndef SEEK_CUR
#  define SEEK_CUR 1
#endif

#define IMAGE     0x2c
#define TRAILER   0x3b
#define EXTENSION 0x21

/* Extension codes */
#define GRAPHIC_CONTROL 0xf9
#define COMMENT         0xfe
#define PLAINTEXT       0x01
#define APPLICATION     0xff

#define PARSE_ERROR     0
#define PARSE_OK        1
#define PARSE_END       (-1)

/* LZW parameter */
#define LZWMAXLEN     12
#define LZWMAXCODE    4096
#define LZWDATAMAXLEN (4096 - 256 + 1)

typedef struct lzwtree LZWtree;
struct lzwtree {
  LZWtree *parent;
  unsigned char datum;
};

typedef union GIFColorCell {
  unsigned char value[3];
  struct _rgb {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
  } rgb;
} GIF_ccell;

typedef struct GIFColorTable {
  int nentry; /* num. of entries */
  GIF_ccell **cell;
} GIF_ct;

typedef struct GIFScreenDescriptor {
  int width;
  int height;
  int gct_follows;
  int resolution;
  int gct_sorted;
  int depth;
  int back;
  int aspect_ratio;
  GIF_ct *gct;
} GIF_sd;

typedef struct GIFImageDescriptor {
  int left;
  int top;
  int width;
  int height;
  int lct_follows;
  int interlace;
  int lct_sorted;
  int depth;
  GIF_ct *lct;
} GIF_id;

typedef struct GIFGraphicControl {
  int disposal;
  int user_input;
  int transparent;
  int delay;
  int transparent_index;
} GIF_gc;

typedef struct GIFImage GIF_image;
struct GIFImage {
  unsigned char *data;
  int initial_length;
  GIF_id *id;
  GIF_gc *gc;
  GIF_image *next;
};

typedef struct GIFFileInfomation {
  int revision; /* 87 or 89 */
  int npics;
  unsigned char *comment;
  unsigned char *applcode;
  char *err;
  void *pixels;
  GIF_sd *sd;
  GIF_gc *recent_gc;
  GIF_image *image;
  GIF_image *top;
} GIF_info;

#define SUCCESS      0
#define NOTGIFFILE   (-1)
#define NOTSUPPORTED (-2)
#define NOENOUGHMEM  (-3)

int       GIFReadScreenDescriptor(Stream *, GIF_info *);
GIF_info *GIFReadSignature(Stream *, int *);
void      GIFDestroyData(GIF_info *);
int       GIFParseNextBlock(Stream *, GIF_info *);
int       GIFParseImageBlock(Stream *, GIF_info *);
int       GIFParseExtensionBlock(Stream *, GIF_info *);
int       GIFParseGraphicControlBlock(Stream *, GIF_info *);
int       GIFParseCommentBlock(Stream *, GIF_info *);
int       GIFParsePlainTextBlock(Stream *, GIF_info *);
int       GIFParseApplicationBlock(Stream *, GIF_info *);
int       GIFSkipExtensionBlock(Stream *, GIF_info *);
