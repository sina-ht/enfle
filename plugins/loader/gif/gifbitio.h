/*
 * gifbitio.h -- bit based I/O package
 * (C)Copyright 1998, 99, 2002 by Hiroshi Takekawa
 * Last Modified: Wed Jul 10 00:26:32 2002.
 * $Id: gifbitio.h,v 1.1 2002/08/03 05:10:17 sian Exp $
 */

#include <stdio.h>

void gif_bitio_init(Stream *);
int getbit(void);
int getbits(int);
