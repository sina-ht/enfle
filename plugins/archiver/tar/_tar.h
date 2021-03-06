/*
 * tar.h -- tar archiver plugin header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Sep 29 06:16:07 2000.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

typedef struct _tar_header {
  char name[100];     /*   0 */
  char mode[8];       /* 100 */
  char uid[8];        /* 108 */
  char gid[8];        /* 116 */
  char size[12];      /* 124 */
  char mtime[12];     /* 136 */
  char checksum[8];   /* 148 */
  char linkflag;      /* 156 */
  char linkname[100]; /* 157 */
  char magic[8];      /* 257 */
  char uname[32];     /* 265 */
  char gname[32];     /* 297 */
  char devmajor[8];   /* 329 */
  char devminor[8];   /* 337 */
  char padding[167];  /* 345 */
} TarHeader;

typedef struct _tar_info {
  char *path;
  unsigned int size;
  long offset;
} TarInfo;
