/*
 * predict.c -- Prediction modules
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * Last Modified: Mon Aug  6 00:37:34 2001.
 * $Id: predict.c,v 1.4 2001/08/05 16:17:58 sian Exp $
 */

#include <stdlib.h>

#define REQUIRE_STRING_H
#include "compat.h"

#include "predict.h"

struct predictor {
  const char *name;
  unsigned char *(*predictor)(unsigned char *, int, int);
  PredictType id;
};

static unsigned char *predict_none(unsigned char *, int, int);
static unsigned char *predict_sub(unsigned char *, int, int);
static unsigned char *predict_sub2(unsigned char *, int, int);
static unsigned char *predict_up(unsigned char *, int, int);
static unsigned char *predict_avg(unsigned char *, int, int);
static unsigned char *predict_avg2(unsigned char *, int, int);
static unsigned char *predict_paeth(unsigned char *, int, int);
static unsigned char *predict_jpegls(unsigned char *, int, int);

static struct predictor predictors[] = {
  { "none",   predict_none,   _PREDICT_NONE },
  { "sub",    predict_sub,    _PREDICT_SUB },
  { "sub2",   predict_sub2,   _PREDICT_SUB2 },
  { "up",     predict_up,     _PREDICT_UP },
  { "avg",    predict_avg,    _PREDICT_AVG },
  { "avg2",   predict_avg2,   _PREDICT_AVG2 },
  { "paeth",  predict_paeth,  _PREDICT_PAETH },
  { "jpegls", predict_jpegls, _PREDICT_JPEGLS },
  { NULL, NULL, _PREDICT_INVALID }
};

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
  int i;

  for (i = 0; predictors[i].name; i++)
    if (strcasecmp(predictors[i].name, name) == 0)
      return predictors[i].id;
  return _PREDICT_INVALID;
}

const char *
predict_get_name_by_id(PredictType id)
{
  if (id < 0 || id >= _PREDICT_INVALID_END)
    return NULL;
  return (char *)predictors[id].name;
}

unsigned char *
predict(unsigned char *s, int w, int h, PredictType id)
{
  if (id < 0 || id >= _PREDICT_INVALID_END)
    return NULL;
  if (predictors[id].predictor)
    return predictors[id].predictor(s, w, h);
  return NULL;
}
