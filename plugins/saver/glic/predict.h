/*
 * predict.h -- Prediction modules header
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * Last Modified: Fri Jun 29 02:18:50 2001.
 * $Id: predict.h,v 1.2 2001/06/28 17:27:19 sian Exp $
 */

#ifndef _PREDICT_H
#define _PREDICT_H

typedef enum _predicttype {
  _PREDICT_INVALID = 0,
  _PREDICT_NONE = 1,
  _PREDICT_SUB,
  _PREDICT_SUB2,
  _PREDICT_UP,
  _PREDICT_AVG,
  _PREDICT_AVG2,
  _PREDICT_PAETH,
  _PREDICT_JPEGLS
} PredictType;

PredictType predict_get_id_by_name(char *);
unsigned char *predict(unsigned char *, int, int, PredictType);

#endif
