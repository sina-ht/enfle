/*
 * libavi.c -- AVI file read library
 * (C)Copyright 2001 by Hiroshi Takekawa
 * Last Modified: Sun Jan 21 04:36:13 2001.
 */

#include <stdio.h>
#include <stdlib.h>
#include "libavi.h"

static int open_avi(AviFileReader *, RIFF_File *);
static int close_avi(AviFileReader *);
static void destroy(AviFileReader *);

static AviFileReader template = {
  open_avi: open_avi,
  close_avi: close_avi,
  destroy: destroy
};

/* constructor */

AviFileReader *
avifilereader_create(void)
{
  AviFileReader *afr;

  if ((afr = calloc(1, sizeof(AviFileReader))) == NULL)
    return NULL;
  memcpy(afr, &template, sizeof(AviFileReader));

  return afr;
}

/* class method */

int
avifilereader_identify(char *path)
{
  FILE *fp;
  char buffer[12];

  if ((fp = fopen(path, "rb")) == NULL)
    goto ERROR;
  if (fread(buffer, 1, 12, fp) != 12)
    goto ERROR;
  if (memcmp(buffer, "RIFF", 4))
    goto NOT_AVI;
  if (memcmp(buffer + 8, "AVI ", 4))
    goto NOT_AVI;

  fclose(fp);
  return 1;

 ERROR:
  fclose(fp);
  return -1;
 NOT_AVI:
  fclose(fp);
  return 0;
}

/* methods */

static int
open_avi(AviFileReader *afr, RIFF_File *rf)
{
  RIFF_Chunk *rc;
  int nstream;
  int n, n1, n2, vnchunk, anchunk;
  char *chunk_name;

  afr->rf = rf;
  riff_file_iter_reset(rf);
  nstream = 0;
  for (;;) {
    if (!riff_file_iter_next_chunk(rf, &rc)) {
      fprintf(stderr, __FUNCTION__ ": riff_file_iter_next_chunk() failed: %s\n", riff_file_get_errmsg(rf));
      return 0;
    }
    if (!rc) {
      if (!riff_file_iter_pop(rf))
	break;
      else
	continue;
    }
    
    if (!rc->is_list) {
      chunk_name = riff_chunk_get_name(rc);
      if (!strcmp(chunk_name, "avih")) {
	if (!riff_file_read_data(rf, rc)) {
	  fprintf(stderr, __FUNCTION__ ": riff_file_read_data() failed: %s\n", riff_file_get_errmsg(rf));
	  return 0;
	}
	/* XXX: Little endian only */
	memcpy(&afr->header, riff_chunk_get_data(rc), sizeof(MainAVIHeader));
	riff_chunk_destroy(rc);
      } else if (!strcmp(chunk_name, "strh")) {
	if (!riff_file_read_data(rf, rc)) {
	  fprintf(stderr, __FUNCTION__ ": riff_file_read_data() failed: %s\n", riff_file_get_errmsg(rf));
	  return 0;
	}
	switch (nstream) {
	case 0:
	  memcpy(&afr->vsheader, riff_chunk_get_data(rc), sizeof(AVIStreamHeader));
	  break;
	case 1:
	  memcpy(&afr->asheader, riff_chunk_get_data(rc), sizeof(AVIStreamHeader));
	  break;
	default:
	  break;
	}
	riff_chunk_destroy(rc);
      } else if (!strcmp(chunk_name, "strf")) {
	if (!riff_file_read_data(rf, rc)) {
	  fprintf(stderr, __FUNCTION__ ": riff_file_read_data() failed: %s\n", riff_file_get_errmsg(rf));
	  return 0;
	}
	switch (nstream) {
	case 0:
	  memcpy(&afr->vformat, riff_chunk_get_data(rc), sizeof(BITMAPINFOHEADER));
	  if ((afr->vchunks = calloc(afr->vsheader.dwLength, sizeof(RIFF_Chunk *))) == NULL) {
	    fprintf(stderr, __FUNCTION__ ": No enough memory\n");
	    return 0;
	  }
	  vnchunk = 0;
	  break;
	case 1:
	  memcpy(&afr->aformat, riff_chunk_get_data(rc), sizeof(WAVEFORMATEX));
	  /* use video frame length... */
	  if ((afr->achunks = calloc(afr->vsheader.dwLength, sizeof(RIFF_Chunk *))) == NULL) {
	    fprintf(stderr, __FUNCTION__ ": No enough memory\n");
	    return 0;
	  }
	  anchunk = 0;
	  break;
	default:
	  fprintf(stderr, __FUNCTION__ ": stream %d is ignored\n", nstream);
	  break;
	}
	riff_chunk_destroy(rc);
	nstream++;
      } else if (!strcmp(chunk_name, "idx1")) {
	/* XXX: process idx1 chunk */
	break;
      } else if (!strcmp(chunk_name + 2, "wb") &&
		 ((n1 = chunk_name[0] - '0') >= 0 && n1 <= 9) &&
		 ((n2 = chunk_name[1] - '0') >= 0 && n2 <= 9)) {
	n = n1 * 10 + n2;
	if (n >= nstream) {
	  fprintf(stderr, "invalid chunk (%d > %d) %s\n", n, nstream, chunk_name);
	  return 0;
	}
	if (n > 1) {
	  fprintf(stderr, "unsupported %d( > 1) stream\n", n);
	  continue;
	}
	/* XXX: should check overrun */
	afr->achunks[anchunk++] = rc;
      } else if ((!strcmp(chunk_name + 2, "db") || !strcmp(chunk_name + 2, "dc")) &&
		 ((n1 = chunk_name[0] - '0') >= 0 && n1 <= 9) &&
		 ((n2 = chunk_name[1] - '0') >= 0 && n2 <= 9)) {
	n = n1 * 10 + n2;
	if (n >= nstream) {
	  fprintf(stderr, "invalid chunk (%d > %d) %s\n", n, nstream, chunk_name);
	  return 0;
	}
	if (n != 0) {
	  fprintf(stderr, "unsupported %d( != 0) stream\n", n);
	  continue;
	}
	afr->vchunks[vnchunk++] = rc;
      }
    } else {
      if (!riff_file_iter_push(rf)) {
	fprintf(stderr, __FUNCTION__ ": riff_file_iter_push() failed: %s\n", riff_file_get_errmsg(rf));
	return 0;
      }
    }
  }

  afr->vnchunks = vnchunk;
  afr->anchunks = anchunk;

  return 1;
}

static int
close_avi(AviFileReader *afr)
{
  if (afr->vchunks)
    free(afr->vchunks);
  if (afr->achunks)
    free(afr->achunks);

  return 1;
}

static void
destroy(AviFileReader *afr)
{
  close_avi(afr);
  free(afr);
}
