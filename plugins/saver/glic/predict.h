/*
 * predict.h -- Prediction modules header
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * Last Modified: Wed May 23 14:01:13 2001.
 * $Id: predict.h,v 1.1 2001/05/23 12:13:15 sian Exp $
 */

#ifndef _PREDICT_H
#define _PREDICT_H

int predict_get_id_by_name(char *);
unsigned char *predict(unsigned char *, int, int, int);

#endif
