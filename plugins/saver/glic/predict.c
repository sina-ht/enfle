/*
 * predict.c -- Prediction modules
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * Last Modified: Wed May 23 17:46:39 2001.
 * $Id: predict.c,v 1.1 2001/05/23 12:13:15 sian Exp $
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

static unsigned char *
predict_sub(unsigned char *s, int w, int h)
{
  int i, j;
  unsigned char *d;
  unsigned int size = w * h;

  if ((d = malloc(size)) == NULL)
    return NULL;
  for (i = 0; i < h; i++) {
    d[i * w] = s[i * w];
    for (j = 1; j < w; j++)
      d[i * w + j] = s[i * w + j] - s[i * w + j - 1];
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
  int i;
  unsigned char *d;
  unsigned int size = w * h;

  if ((d = malloc(size)) == NULL)
    return NULL;
  d[0] = s[0];
  for (j = 1; j < w; j++)
    d[j] = s[j] - (s[j - 1] >> 1);
  for (i = 1; i < h; i++) {
    d[i * w] = s[j] - (s[(i - 1) * w] >> 1);
    for (j = 1; j < w; j++)
      d[i * w + j] = s[j] - ((s[(i - 1) * w + j] + s[i * w + j - 1]) >> 1);
  }

  return d;
}

static unsigned char *
predict_paeth(unsigned char *s, int w, int h)
{
  int i;
  unsigned char *d;
  unsigned int size = w * h;

  if ((d = malloc(size)) == NULL)
    return NULL;
  d[0] = s[0];
  for (j = 1; j < w; j++)
    d[j] = s[j] - (s[j - 1] >> 1);
  for (i = 1; i < h; i++) {
    d[i * w] = s[j] - (s[(i - 1) * w] >> 1);
    for (j = 1; j < w; j++)
      d[i * w + j] = s[j] - ((s[(i - 1) * w + j] + s[i * w + j - 1]) >> 1);
  }

  return d;
}

int
predict_get_id_by_name(char *name)
{
  if (strcasecmp(name, "none") == 0)
    return 1;
  if (strcasecmp(name, "sub") == 0)
    return 2;
  if (strcasecmp(name, "up") == 0)
    return 3;
  if (strcasecmp(name, "avg") == 0)
    return 4;
  if (strcasecmp(name, "paeth") == 0)
    return 5;
  return 0;
}

unsigned char *
predict(unsigned char *s, int w, int h, int id)
{
  switch (id) {
  case 1:
    return predict_none(s, w, h);
  case 2:
    return predict_sub(s, w, h);
  case 3:
    return predict_up(s, w, h);
  case 4:
    return predict_avg(s, w, h);
  case 5:
    return predict_paeth(s, w, h);
  default:
    return NULL;
  }
}
