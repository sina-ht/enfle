/*
 * scan.c -- Scanning modules
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * Last Modified: Mon Aug  6 02:01:07 2001.
 * $Id: scan.c,v 1.3 2001/08/06 04:58:23 sian Exp $
 */

#include <stdlib.h>

#define REQUIRE_STRING_H
#include "compat.h"

#include "scan.h"
#include "floorlog2.h"

struct scanner {
  const char *name;
  void (*scanner)(unsigned char *, unsigned char *, int, int, int, int, int);
  ScanType id;
};

static void scan_normal(unsigned char *, unsigned char *, int, int, int, int, int);
static void scan_quad(unsigned char *, unsigned char *, int, int, int, int, int);
static void scan_hilbert(unsigned char *, unsigned char *, int, int, int, int, int);

static struct scanner scanners[] = {
  { NULL, NULL, _SCAN_INVALID },
  { "normal",  scan_normal,  _SCAN_NORMAL },
  { "quad",    scan_quad,    _SCAN_QUAD },
  { "hilbert", scan_hilbert, _SCAN_HILBERT },
  { NULL, NULL, _SCAN_INVALID_END }
};

static void
scan_normal(unsigned char *d, unsigned char *s, int width, int w, int h, int x, int y)
{
  memcpy(d, s, width * h);
}

static void
scan_quad(unsigned char *d, unsigned char *s, int width, int w, int h, int x, int y)
{
  if (w > 2) {
    int hw = w >> 1;
    int hh = h >> 1;

    scan_quad(d              , s, width, hw, hh, x     , y     );
    scan_quad(d + hw * hh    , s, width, hw, hh, x + hw, y     );
    scan_quad(d + hw * hh * 2, s, width, hw, hh, x     , y + hh);
    scan_quad(d + hw * hh * 3, s, width, hw, hh, x + hw, y + hh);
  } else {
    *d++ = s[ y      * width + x    ];
    *d++ = s[ y      * width + x + 1];
    *d++ = s[(y + 1) * width + x    ];
    *d++ = s[(y + 1) * width + x + 1];
  }
}

static void scan_hilbert_rs(unsigned char *, unsigned char *, int, int, int, int, int);
static void scan_hilbert_ls(unsigned char *, unsigned char *, int, int, int, int, int);
static void scan_hilbert_us(unsigned char *, unsigned char *, int, int, int, int, int);
static void scan_hilbert_ds(unsigned char *, unsigned char *, int, int, int, int, int);

static void
scan_hilbert_rs(unsigned char *d, unsigned char *s, int width, int w, int h, int x, int y)
{
  if (w > 2) {
    int hw = w >> 1;
    int hh = h >> 1;

    scan_hilbert_ds(d              , s, width, hw, hh, x     , y     );
    scan_hilbert_rs(d + hw * hh    , s, width, hw, hh, x + hw, y     );
    scan_hilbert_rs(d + hw * hh * 2, s, width, hw, hh, x + hw, y + hh);
    scan_hilbert_us(d + hw * hh * 3, s, width, hw, hh, x     , y + hh);
  } else {
    *d++ = s[ y      * width + x    ];
    *d++ = s[ y      * width + x + 1];
    *d++ = s[(y + 1) * width + x + 1];
    *d++ = s[(y + 1) * width + x    ];
 }
}

static void
scan_hilbert_ls(unsigned char *d, unsigned char *s, int width, int w, int h, int x, int y)
{
  if (w > 2) {
    int hw = w >> 1;
    int hh = h >> 1;

    scan_hilbert_us(d              , s, width, hw, hh, x + hw, y + hh);
    scan_hilbert_ls(d + hw * hh    , s, width, hw, hh, x     , y + hh);
    scan_hilbert_ls(d + hw * hh * 2, s, width, hw, hh, x     , y     );
    scan_hilbert_ds(d + hw * hh * 3, s, width, hw, hh, x + hw, y     );
  } else {
    *d++ = s[(y + 1) * width + x + 1];
    *d++ = s[(y + 1) * width + x    ];
    *d++ = s[ y      * width + x    ];
    *d++ = s[ y      * width + x + 1];
  }
}

static void
scan_hilbert_us(unsigned char *d, unsigned char *s, int width, int w, int h, int x, int y)
{
  if (w > 2) {
    int hw = w >> 1;
    int hh = h >> 1;

    scan_hilbert_ls(d              , s, width, hw, hh, x + hw, y + hh);
    scan_hilbert_us(d + hw * hh    , s, width, hw, hh, x + hw, y     );
    scan_hilbert_us(d + hw * hh * 2, s, width, hw, hh, x     , y     );
    scan_hilbert_rs(d + hw * hh * 3, s, width, hw, hh, x     , y + hh);
  } else {
    *d++ = s[(y + 1) * width + x + 1];
    *d++ = s[ y      * width + x + 1];
    *d++ = s[ y      * width + x    ];
    *d++ = s[(y + 1) * width + x    ];
  }
}

static void
scan_hilbert_ds(unsigned char *d, unsigned char *s, int width, int w, int h, int x, int y)
{
  if (w > 2) {
    int hw = w >> 1;
    int hh = h >> 1;

    scan_hilbert_rs(d              , s, width, hw, hh, x     , y     );
    scan_hilbert_ds(d + hw * hh    , s, width, hw, hh, x     , y + hh);
    scan_hilbert_ds(d + hw * hh * 2, s, width, hw, hh, x + hw, y + hh);
    scan_hilbert_ls(d + hw * hh * 3, s, width, hw, hh, x + hw, y     );
  } else {
    *d++ = s[ y      * width + x    ];
    *d++ = s[(y + 1) * width + x    ];
    *d++ = s[(y + 1) * width + x + 1];
    *d++ = s[ y      * width + x + 1];
  }
}

static void
scan_hilbert(unsigned char *d, unsigned char *s, int width, int w, int h, int x, int y)
{
  int b;

  b = floorlog2(width);
  if (b % 2 == 0)
    scan_hilbert_ds(d, s, width, w, h, x, y);
  else
    scan_hilbert_rs(d, s, width, w, h, x, y);
}

/* methods */

ScanType
scan_get_id_by_name(char *name)
{
  int i;

  for (i = 1; scanners[i].id != _SCAN_INVALID_END; i++)
    if (strcasecmp(scanners[i].name, name) == 0)
      return scanners[i].id;
  return _SCAN_INVALID;
}

const char *
scan_get_name_by_id(ScanType id)
{
  if (id < 0 || id >= _SCAN_INVALID_END)
    return NULL;
  return scanners[id].name;
}

int
scan(unsigned char *d, unsigned char *s, int width, int height, ScanType id)
{
  if (id < 0 || id >= _SCAN_INVALID_END)
    return 0;
  if (scanners[id].scanner) {
    scanners[id].scanner(d, s, width, width, height, 0, 0);
    return 1;
  }

  return 0;
}
