/*
 * stream.h -- stream interface header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Nov  3 16:08:46 2007.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _STREAM_H
#define _STREAM_H

typedef enum _stream_whence {
  _SET,
  _CUR,
  _END
} StreamWhence;

typedef struct _stream Stream;
struct _stream {
  char *path;
  char *format;

  /* You need not use all these variables */
  void *data;
  unsigned char *buffer;
  unsigned char *ptr;
  long ptr_offset;
  unsigned int buffer_size;
  unsigned int buffer_used;
  char *tmppath;

  /* transfer() transfers all data to new created object. The old object will be initialized */
  Stream *(*transfer)(Stream *);
  /* fundamental stream implementations */
  int (*make_memorystream)(Stream *, unsigned char *, int);
  int (*make_fdstream)(Stream *, int);
  int (*make_filestream)(Stream *, char *);
  int (*make_tmpfilestream)(Stream *, char *, char *);
  /* must be overridden by stream implementor */
  int (*read)(Stream *, unsigned char *, int);
  int (*seek)(Stream *, long, StreamWhence);
  long (*tell)(Stream *);
  int (*close)(Stream *);
  /* can be overridden by specific stream implementor */
  void (*destroy)(Stream *);
};

#define stream_transfer(st) (st)->transfer((st))
#define stream_make_memorystream(st, p, size) (st)->make_memorystream((st), (p), (size))
#define stream_make_fdstream(st, fd) (st)->make_fdstream((st), (fd))
#define stream_make_filestream(st, path) (st)->make_filestream((st), (path))
#define stream_make_tmpfilestream(st, path, name) (st)->make_tmpfilestream((st), (path), (name))
#define stream_read(st, p, s) (st)->read((st), (p), (s))
#define stream_seek(st, o, w) (st)->seek((st), (o), (w))
#define stream_rewind(st) (st)->seek((st), 0, _SET)
#define stream_tell(st) (st)->tell((st))
#define stream_close(st) (st)->close((st))
#define stream_destroy(st) (st)->destroy((st))

Stream *stream_create(void);

#endif
