/*
 * scan.c -- Scanning modules
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * Last Modified: Wed May 23 14:09:44 2001.
 * $Id: scan.c,v 1.1 2001/05/23 12:13:15 sian Exp $
 */

#include <stdlib.h>

#define REQUIRE_STRING_H
#include "compat.h"

#include "scan.h"
#include "floorlog2.h"

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

int
scan_get_id_by_name(char *name)
{
  if (strcasecmp(name, "normal") == 0)
    return 1;
  if (strcasecmp(name, "quad") == 0)
    return 2;
  if (strcasecmp(name, "hilbert") == 0)
    return 3;
  return 0;
}

int
scan(unsigned char *d, unsigned char *s, int width, int height, int id)
{
  switch (id) {
  case 1:
    memcpy(d, s, width * height);
    return 1;
  case 2:
    scan_quad(d, s, width, width, height, 0, 0);
    return 1;
  case 3:
    scan_hilbert(d, s, width, width, height, 0, 0);
    return 1;
  default:
    return 0;
  }
}
