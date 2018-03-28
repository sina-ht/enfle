/*
 * gifbitio.h -- bit based I/O package
 * (C)Copyright 1998, 99, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Jun 21 21:17:54 2004.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include <stdio.h>

void gif_bitio_init(Stream *);
int getbit(void);
int getbits(int);
