/*
 * predict.c -- Prediction modules
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * Last Modified: Fri Jun 29 02:25:53 2001.
 * $Id: predict.c,v 1.3 2001/06/28 17:27:19 sian Exp $
 */

#include <stdlib.h>

#define REQUIRE_STRING_H
#include "compat.h"

#include "predict.h"

static unsigned char *
predict_none(unsigned char *s, int w, int h)
{
  unsigned char *d;
  unsigned int size = w * h;

  if ((d = malloc(size)) == NULL)
    return NULL;
  memcpy(d, s, size);

  return d;
}

#define P2I(x,y,w) ((y) * (w) + (x))

static unsigned char *
predict_sub(unsigned char *s, int w, int h)
{
  int i, j;
  unsigned char *d;

  if ((d = malloc(w * h)) == NULL)
    return NULL;

  for (i = 0; i < h; i++) {
    d[P2I(0,i,w)] = s[P2I(0,i,w)];
    for (j = 1; j < w; j++)
      d[P2I(j,i,w)] = s[P2I(j,i,w)] - s[P2I(j-1,i,w)];
  }

  return d;
}

static unsigned char *
predict_sub2(unsigned char *s, int w, int h)
{
  int i, j;
  unsigned char *d;

  if ((d = malloc(w * h)) == NULL)
    return NULL;

  d[P2I(0,0,w)] = s[P2I(0,0,w)];
  for (j = 1; j < w; j++)
    d[P2I(j,0,w)] = s[P2I(j,0,w)] - s[P2I(j-1,0,w)];

  for (i = 1; i < h; i++) {
    d[P2I(0,i,w)] = s[P2I(0,i,w)] - s[P2I(0,i-1,w)];
    for (j = 1; j < w; j++)
      d[P2I(j,i,w)] = s[P2I(j,i,w)] - s[P2I(j-1,i,w)];
  }

  return d;
}

static unsigned char *
predict_up(unsigned char *s, int w, int h)
{
  int i;
  unsigned char *d;
  unsigned int size = w * h;

  if ((d = malloc(size)) == NULL)
    return NULL;
  memcpy(d, s, w);
  for (i = 0; i < size - w; i++)
    d[w + i] = s[w + i] - s[i];

  return d;
}

static unsigned char *
predict_avg(unsigned char *s, int w, int h)
{
  int i, j;
  unsigned char *d;

  if ((d = malloc(w * h)) == NULL)
    return NULL;

  d[P2I(0,0,w)] = s[P2I(0,0,w)];
  for (j = 1; j < w; j++)
    d[P2I(j,0,w)] = s[P2I(j,0,w)] - (s[P2I(j-1,0,w)] >> 1);

  for (i = 1; i < h; i++) {
    d[P2I(0,i,w)] = s[P2I(0,i,w)] - (s[P2I(0,i-1,w)] >> 1);
    for (j = 1; j < w; j++)
      d[P2I(j,i,w)] = s[P2I(j,i,w)] - ((s[P2I(j,i-1,w)] + s[P2I(j-1,i,w)]) >> 1);
  }

  return d;
}

static unsigned char *
predict_avg2(unsigned char *s, int w, int h)
{
  int i, j;
  unsigned char *d;

  if ((d = malloc(w * h)) == NULL)
    return NULL;

  d[P2I(0,0,w)] = s[P2I(0,0,w)];
  for (j = 1; j < w; j++)
    d[P2I(j,0,w)] = s[P2I(j,0,w)] - (s[P2I(j-1,0,w)] >> 1);

  for (i = 1; i < h; i++) {
    d[P2I(0,i,w)] = s[P2I(0,i,w)] - (s[P2I(0,i-1,w)] >> 1);
    for (j = 1; j < w; j++)
      d[P2I(j,i,w)] = s[P2I(j,i,w)] - (s[P2I(j,i-1,w)] + s[P2I(j-1,i,w)] - s[P2I(j-1,i-1,w)]);
  }

  return d;
}

static unsigned char *
predict_paeth(unsigned char *s, int w, int h)
{
  int i, j;
  unsigned char p, t, *d;
  int pa, pb, pc;

  if ((d = malloc(w * h)) == NULL)
    return NULL;
  d[0] = s[0];
  for (j = 1; j < w; j++)
    d[j] = s[j] - s[j - 1];
  for (i = 1; i < h; i++) {
    d[i * w] = s[i * w] - s[(i - 1) * w];
    for (j = 1; j < w; j++) {
      unsigned char a, b, c;

      a = s[i * w + j - 1];
      b = s[(i - 1) * w + j];
      c = s[(i - 1) * w + j - 1];
      t = a + b - c;
      pa = abs((int)t - a);
      pb = abs((int)t - b);
      pc = abs((int)t - c);
      if (pa <= pb && pa <= pc)
	p = a;
      else if (pb <= pc)
	p = b;
      else
	p = c;
      d[i * w + j] = s[i * w + j] - p;
    }
  }

  return d;
}

#define PREDICT_JLS_T1 2
#define PREDICT_JLS_T2 4
#define PREDICT_JLS_T3 6

static unsigned char *
predict_jpegls(unsigned char *s, int w, int h)
{
#if 0
  int i, j, d1, d2, d3;
  unsigned char *dd, a, b, c, d;
  int pa, pb, pc;

  if ((dd = malloc(w * h)) == NULL)
    return NULL;

  for (i = 0; i < h; i++)
    for (j = 0; j < w; j++) {
      a = P2I(j-1,i  ,w);
      b = P2I(j  ,i-1,w);
      c = P2I(j-1,i-1,w);
      d = P2I(j+1,i-1,w);
      d1 = d - b;
      d2 = b - c;
      d3 = c - a;
    }
#endif
  return NULL;
}

/* methods */

PredictType
predict_get_id_by_name(char *name)
{
  if (strcasecmp(name, "none") == 0 || strcasecmp(name, "no") == 0)
    return _PREDICT_NONE;
  if (strcasecmp(name, "sub") == 0)
    return _PREDICT_SUB;
  if (strcasecmp(name, "sub2") == 0)
    return _PREDICT_SUB2;
  if (strcasecmp(name, "up") == 0)
    return _PREDICT_UP;
  if (strcasecmp(name, "avg") == 0)
    return _PREDICT_AVG;
  if (strcasecmp(name, "avg2") == 0)
    return _PREDICT_AVG2;
  if (strcasecmp(name, "paeth") == 0)
    return _PREDICT_PAETH;
  if (strcasecmp(name, "jpegls") == 0 || strcasecmp(name, "jls") == 0)
    return _PREDICT_JPEGLS;
  return _PREDICT_INVALID;
}

unsigned char *
predict(unsigned char *s, int w, int h, PredictType id)
{
  switch (id) {
  case _PREDICT_NONE:
    return predict_none(s, w, h);
  case _PREDICT_SUB:
    return predict_sub(s, w, h);
  case _PREDICT_SUB2:
    return predict_sub2(s, w, h);
  case _PREDICT_UP:
    return predict_up(s, w, h);
  case _PREDICT_AVG:
    return predict_avg(s, w, h);
  case _PREDICT_AVG2:
    return predict_avg2(s, w, h);
  case _PREDICT_PAETH:
    return predict_paeth(s, w, h);
  case _PREDICT_JPEGLS:
    return predict_jpegls(s, w, h);
  default:
    return NULL;
  }
}
