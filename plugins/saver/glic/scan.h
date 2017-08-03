/*
 * scan.h -- Scanning modules header
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * Last Modified: Mon Aug  6 00:37:57 2001.
 */

#ifndef _SCAN_H
#define _SCAN_H

typedef enum _scantype {
  _SCAN_INVALID = 0,
  _SCAN_NORMAL = 1,
  _SCAN_QUAD,
  _SCAN_HILBERT,
  _SCAN_INVALID_END
} ScanType;

ScanType scan_get_id_by_name(char *);
const char *scan_get_name_by_id(ScanType);
int scan(unsigned char *, unsigned char *, int, int, ScanType);

#endif
