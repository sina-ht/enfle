/*
 * stream.c -- stream interface
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Oct 14 04:53:46 2000.
 * $Id: stream.c,v 1.3 2000/10/15 07:51:10 sian Exp $
 *
 * Enfle is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Enfle is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <stdio.h>
#include <stdlib.h>

#define REQUIRE_STRING_H
#define REQUIRE_UNISTD_H
#include "compat.h"

#include "common.h"

#include "stream.h"

static Stream *transfer(Stream *);
static int make_memorystream(Stream *, unsigned char *, int);
static int make_fdstream(Stream *, int);
static int make_filestream(Stream *, char *);
static void destroy(Stream *);

static Stream template = {
  data: NULL,
  buffer: NULL,
  ptr: NULL,
  buffer_size: 0,
  transfer: transfer,
  make_memorystream: make_memorystream,
  make_fdstream: make_fdstream,
  make_filestream: make_filestream,
  read: NULL,
  seek: NULL,
  tell: NULL,
  close: NULL,
  destroy: destroy
};

Stream *
stream_create(void)
{
  Stream *s;

  if ((s = calloc(1, sizeof(Stream))) == NULL)
    return NULL;
  memcpy(s, &template, sizeof(Stream));

  return s;
}

/* for internal use */

static int
fdstream_grow(Stream *s , int size)
{
  int to_read, grow_size, have_read;

  if (size < 0)
    return 0;

  to_read = s->ptr - s->buffer + size - s->buffer_used;

  if (to_read <= 0)
    return size;

  grow_size = s->buffer_used + to_read - s->buffer_size;
  if (grow_size > 0) {
    int ptr_offset;

    if (grow_size < 1024)
      grow_size = 1024;

    ptr_offset = s->ptr - s->buffer;
    s->buffer_size += grow_size;
    if ((s->buffer = realloc(s->buffer, s->buffer_size)) == NULL) {
      show_message("fdstream_grow: No enough memory (tried to allocate %d bytes)\n", s->buffer_size);
      return -1;
    }
    s->ptr = s->buffer + ptr_offset;
  }

  /* Should I use select()? */
  if ((have_read = read((int)s->data, s->buffer + s->buffer_used, to_read)) < 0) {
    show_message("fdstream_grow: read failed\n");
    return -1;
  }
  s->buffer_used += have_read;

  return size - (to_read - have_read);
}

/* read functions */

static int
memorystream_read(Stream *s, unsigned char *p, int size)
{
  unsigned int read_size;

  read_size = s->buffer_size > size ? size : s->buffer_size;
  if (read_size > 0) {
    memcpy(p, s->ptr, read_size);
    s->ptr += read_size;
  }

  return read_size;
}

static int
fdstream_read(Stream *s, unsigned char *p, int size)
{
  if ((size = fdstream_grow(s, size)) < 0)
    return size;

  memcpy(p, s->ptr, size);
  s->ptr += size;

  return size;
}

static int
filestream_read(Stream *s, unsigned char *p, int size)
{
  int have_read;

  if ((have_read = fread(p, 1, size, (FILE *)s->data)) == 0)
    return feof((FILE *)s->data) ? 0 : -1;

  return have_read;
}

/* seek functions */

static int
memorystream_seek(Stream *s, long offset, StreamWhence whence)
{
  switch (whence) {
  case _SET:
    if (offset < 0 || offset > s->buffer_size) {
      show_message("memorystream_seek: _SET: invalid offset %ld\n", offset);
      return 0;
    }
    s->ptr = s->buffer + offset;
    return 1;
  case _CUR:
    if (s->ptr - s->buffer + offset < 0) {
      show_message("memorystream_seek: _CUR: underflow (offset = %ld)\n", offset);
      return 0;
    }
    if (s->ptr - s->buffer + offset > s->buffer_size) {
      show_message("memorystream_seek: _CUR: overflow (offset = %ld)\n", offset);
      return 0;
    }
    s->ptr += offset;
    return 1;
  case _END:
    if (s->buffer_size + offset < 0) {
      show_message("memorystream_seek: _END: underflow (offset = %ld)\n", offset);
      return 0;
    }
    if (offset > 0) {
      show_message("memorystream_seek: _END: overflow (offset = %ld)\n", offset);
      return 0;
    }
    s->ptr = s->buffer + s->buffer_size - offset;
    return 1;
  }

  fprintf(stderr, "memorystream_seek: Invalid whence %d\n", whence);
  return 0;
}

static int
fdstream_seek(Stream *s, long offset, StreamWhence whence)
{
  switch (whence) {
  case _SET:
    if (offset < 0) {
      show_message("fdstream_seek: _SET: underflow (offset = %ld)\n", offset);
      return 0;
    }
    if (fdstream_grow(s, offset - (s->ptr - s->buffer)) < 0)
      return 0;
    s->ptr = s->buffer + offset;
    return 1;
  case _CUR:
    if (s->ptr - s->buffer + offset < 0) {
      show_message("fdstream_seek: _CUR: underflow (offset = %ld)\n", offset);
      return 0;
    }
    if (fdstream_grow(s, offset) < 0)
      return 0;
    s->ptr += offset;
    return 1;
  case _END:
    /* cannot be implemented, since we don't know end. */
    show_message("fdstream_seek: _END: cannot be implemented.\n");
    return 0;
  }

  fprintf(stderr, "fdstream_seek: Invalid whence %d\n", whence);
  return 0;
}

static int
filestream_seek(Stream *s, long offset, StreamWhence whence)
{
  switch (whence) {
  case _SET:
    return (fseek((FILE *)s->data, offset, SEEK_SET) == -1) ? 0 : 1;
  case _CUR:
    return (fseek((FILE *)s->data, offset, SEEK_CUR) == -1) ? 0 : 1;
  case _END:
    return (fseek((FILE *)s->data, offset, SEEK_END) == -1) ? 0 : 1;
  }

  fprintf(stderr, "filestream_seek: Invalid whence %d\n", whence);
  return -1;
}

/* tell functions */

static long
memorystream_tell(Stream *s)
{
  return s->ptr - s->buffer;
}

static long
fdstream_tell(Stream *s)
{
  return s->ptr - s->buffer;
}

static long
filestream_tell(Stream *s)
{
  return ftell((FILE *)s->data);
}

/* close functions */

static int
memorystream_close(Stream *s)
{
  if (s->buffer) {
    free(s->buffer);
    s->buffer = NULL;
  }

  return 1;
}

static int
fdstream_close(Stream *s)
{
  int f;

  if (s->buffer) {
    free(s->buffer);
    s->buffer = NULL;
  }

  f =  (close((int)s->data) == 0) ? 1 : 0;
  s->data = NULL;

  return f;
}

static int
filestream_close(Stream *s)
{
  int f;

  if (s->data == NULL)
    return 1;

  f = (fclose((FILE *)s->data) == 0) ? 1 : 0;
  s->data = NULL;

  return f;
}

/* methods */

static Stream *
transfer(Stream *s)
{
  Stream *new;

  if ((new = stream_create()) == NULL)
    return NULL;
  memcpy(new, s, sizeof(Stream));
  memcpy(s, &template, sizeof(Stream));

  return new;
}

static int
make_memorystream(Stream *s, unsigned char *p, int size)
{
  s->buffer = p;
  s->buffer_size = size;
  s->read = memorystream_read;
  s->seek = memorystream_seek;
  s->tell = memorystream_tell;
  s->close = memorystream_close;

  s->ptr = s->buffer;

  return 1;
}

static int
make_fdstream(Stream *s, int fd)
{
  s->buffer_size = 1024;
  if ((s->buffer = calloc(1, s->buffer_size)) == NULL)
    return 0;
  s->ptr = s->buffer;
  s->buffer_used = 0;

  s->read = fdstream_read;
  s->seek = fdstream_seek;
  s->tell = fdstream_tell;
  s->close = fdstream_close;

  s->data = (void *)fd;

  return 1;
}

static int
make_filestream(Stream *s, char *path)
{
  FILE *fp;

  if ((fp = fopen(path, "rb")) == NULL)
    return 0;

  if ((s->path = strdup(path)) == NULL) {
    fclose(fp);
    return 0;
  }

  s->read = filestream_read;
  s->seek = filestream_seek;
  s->tell = filestream_tell;
  s->close = filestream_close;

  s->data = (void *)fp;

  return 1;
}

static void
destroy(Stream *s)
{
  if (s->close)
    stream_close(s);
  if (s->path)
    free(s->path);
  free(s);
}
