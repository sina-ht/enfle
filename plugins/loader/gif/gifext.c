/*
 * gifext.c -- parse GIF extension block
 * (C)Copyright 1998, 2002 by Hiroshi Takekawa
 * Last Modified: Sat Oct 19 11:26:47 2002.
 */

#include "compat.h"
#include "common.h"

#include "giflib.h"
#include "enfle/stream-utils.h"

#define GC_BLOCK_SIZE   4
#define APPL_BLOCK_SIZE 11

int
GIFSkipExtensionBlock(Stream *st, GIF_info *info)
{
  unsigned char size;

  debug_message_fn("() called\n");

  while ((size = stream_getc(st)) > 0)
    stream_seek(st, (long)size, SEEK_CUR);
  return PARSE_OK;
}

int
GIFParseGraphicControlBlock(Stream *st, GIF_info *info)
{
  unsigned char c[5];
  GIF_gc *gc;

  if (stream_getc(st) != GC_BLOCK_SIZE) {
    info->err = (char *)"Illegal block size";
    return PARSE_ERROR;
  }

  stream_read(st, c, 5);
  if ((gc = info->recent_gc = calloc(1, sizeof(GIF_gc))) == NULL) {
    info->err = (char *)"No enough memory for graphic control block";
    return PARSE_ERROR;
  }

  gc->disposal = (c[0] >> 2) & 7;
  gc->user_input = (c[0] >> 1) & 1;
  gc->transparent = c[0] & 1;
  gc->delay = (c[2] << 8) + c[1];
  gc->transparent_index = c[3];

  debug_message_fnc("disposal = %d user_input = %d\n", gc->disposal, gc->user_input);
  debug_message_fnc("trans = %d delay = %d transindex = %d\n",
		    gc->transparent, gc->delay, gc->transparent_index);

  return (c[4] == 0) ? PARSE_OK : PARSE_ERROR;
}

int
GIFParseCommentBlock(Stream *st, GIF_info *info)
{
  unsigned char size;
  int len;

  debug_message_fn("() called\n");

  len = stream_getc(st);
  if (len > 0) {
    if ((info->comment = malloc(len + 1)) == NULL) {
      info->err = (char *)"No enough memory for comment";
      return PARSE_ERROR;
    }

    if (stream_read(st, info->comment, len) > 0) {
      *(info->comment + len) = '\0';
      while ((size = stream_getc(st)) > 0) {
	if ((info->comment = realloc(info->comment, size + len + 1)) == NULL) {
	  info->err = (char *)"No enough memory for comment (reallocation)";
	  return PARSE_ERROR;
	}
	if (stream_read(st, info->comment + len, size) > 0) {
	  *(info->comment + len + size) = '\0';
	  len += size;
	} else
	  break;
      }
    }
  }
  return PARSE_OK;
}

int
GIFParsePlainTextBlock(Stream *st, GIF_info *info)
{
  debug_message_fn("() called\n");

  return GIFSkipExtensionBlock(st, info);
}

int
GIFParseApplicationBlock(Stream *st, GIF_info *info)
{
  unsigned char authcode[3];

  debug_message_fn("() called\n");

  if (stream_getc(st) != APPL_BLOCK_SIZE) {
    info->err = (char *)"Illegal application block size";
    return PARSE_ERROR;
  }

  if ((info->applcode = (unsigned char *)malloc(8 + 1)) == NULL) {
    info->err = (char *)"No enough memory for application code";
    return PARSE_ERROR;
  }
  stream_read(st, info->applcode, 8);
  *(info->applcode + 8) = '\0';
  stream_read(st, authcode, 3);
  return GIFSkipExtensionBlock(st, info);
}

int
GIFParseExtensionBlock(Stream *st, GIF_info *info)
{
  int c = stream_getc(st);

  debug_message_fn("() called\n");

  switch (c) {
  case GRAPHIC_CONTROL:
    c = GIFParseGraphicControlBlock(st, info);
    break;
  case COMMENT:
    c = GIFParseCommentBlock(st, info);
    break;
  case PLAINTEXT:
    c = GIFParsePlainTextBlock(st, info);
    break;
  case APPLICATION:
    c = GIFParseApplicationBlock(st, info);
    break;
  default:
    c = GIFSkipExtensionBlock(st, info);
    break;
  }
  return c;
}
