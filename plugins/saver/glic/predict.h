/*
 * predict.h -- Prediction modules header
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * Last Modified: Mon Aug  6 00:37:45 2001.
 * $Id: predict.h,v 1.3 2001/08/05 16:17:58 sian Exp $
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
  _PREDICT_JPEGLS,
  _PREDICT_INVALID_END
} PredictType;

PredictType predict_get_id_by_name(char *);
const char *predict_get_name_by_id(PredictType);
unsigned char *predict(unsigned char *, int, int, PredictType);

#endif
