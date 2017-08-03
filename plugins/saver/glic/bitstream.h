/*
 * bitstream.h -- bitwise I/O stream header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * Last Modified: Tue Apr  3 18:08:07 2001.
 */

#ifndef _BITSTREAM_H
#define _BITSTREAM_H

#include <stdio.h>

typedef unsigned char Bits;

#define BUFFER_SIZE (sizeof(Bits))
#define BUFFER_BITS (sizeof(Bits) << 3)
#define BUFFER_MSB (1U << (BUFFER_BITS - 1))

typedef struct _bitstream BitStream;
struct _bitstream {
  FILE *fp;
  Bits buffer;
  unsigned int buffer_left;

  void (*init)(BitStream *, FILE *);
  int (*getbit)(BitStream *);
  unsigned int (*getbits)(BitStream *, unsigned int, int *);
  int (*putbit)(BitStream *, int);
  int (*putbits)(BitStream *, unsigned int, int);
  int (*flush)(BitStream *);
  void (*destroy)(BitStream *);
};

#define bitstream_init(bs, fp) (bs)->init((bs), (fp))
#define bitstream_getbit(bs) (bs)->getbit((bs))
#define bitstream_getbits(bs, n, er) (bs)->getbits((bs), (n), (er))
#define bitstream_putbit(bs, b) (bs)->putbit((bs), (b))
#define bitstream_putbits(bs, b, n) (bs)->putbits((bs), (b), (n))
#define bitstream_flush(bs) (bs)->flush((bs))
#define bitstream_destroy(bs) (bs)->destroy((bs))

BitStream *bitstream_create(void);

#endif
