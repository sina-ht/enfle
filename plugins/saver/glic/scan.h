/*
 * scan.h -- Scanning modules header
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * Last Modified: Wed May 23 13:32:56 2001.
 * $Id: scan.h,v 1.1 2001/05/23 12:13:15 sian Exp $
 */

#ifndef _SCAN_H
#define _SCAN_H

int scan_get_id_by_name(char *);
int scan(unsigned char *, unsigned char *, int, int, int);

#endif
