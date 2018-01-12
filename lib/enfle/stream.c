/*
 * stream.c -- stream interface
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Nov  3 16:08:56 2007.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
static int make_tmpfilestream(Stream *, char *, char *);
static void destroy(Stream *);

static int dummy_read(Stream *, unsigned char *, int);
static int dummy_seek(Stream *, long, StreamWhence);
static long dummy_tell(Stream *);
static int dummy_close(Stream *);

static Stream template = {
  .data = NULL,
  .buffer = NULL,
  .ptr = NULL,

  .transfer = transfer,
  .make_memorystream = make_memorystream,
  .make_fdstream = make_fdstream,
  .make_filestream = make_filestream,
  .make_tmpfilestream = make_tmpfilestream,
  .read = dummy_read,
  .seek = dummy_seek,
  .tell = dummy_tell,
  .close = dummy_close,
  .destroy = destroy
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
      err_message_fnc("No enough memory (tried to allocate %d bytes)\n", s->buffer_size);
      return -1;
    }
    s->ptr = s->buffer + ptr_offset;
  }

  /* Should I use select()? */
  if ((have_read = read((long)s->data, s->buffer + s->buffer_used, to_read)) < 0) {
    err_message_fnc("read failed\n");
    return -1;
  }
  s->buffer_used += have_read;

  return size - (to_read - have_read);
}

/* read functions */

static int
dummy_read(Stream *s __attribute__((unused)), unsigned char *p, int size)
{
  warning_fnc("This function only fills the buffer with 0.\n");
  memset(p, 0, size);
  return size;
}

static int
memorystream_read(Stream *s, unsigned char *p, int size)
{
  int remain = s->buffer_size - (s->ptr - s->buffer);
  int read_size;

  read_size = remain > size ? size : remain;
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
dummy_seek(Stream *s __attribute__((unused)), long offset __attribute__((unused)), StreamWhence whence __attribute__((unused)))
{
  warning_fnc("This function always returns 1.\n");
  return 1;
}

static int
memorystream_seek(Stream *s, long offset, StreamWhence whence)
{
  switch (whence) {
  case _SET:
    if (offset < 0 || (unsigned long)offset > s->buffer_size) {
      err_message_fnc("_SET: invalid offset %ld\n", offset);
      return 0;
    }
    s->ptr = s->buffer + offset;
    return 1;
  case _CUR:
    if (s->ptr - s->buffer + offset < 0) {
      err_message_fnc("_CUR: underflow (offset = %ld)\n", offset);
      return 0;
    }
    if ((unsigned long)(s->ptr - s->buffer + offset) > s->buffer_size) {
      err_message_fnc("_CUR: overflow (offset = %ld)\n", offset);
      return 0;
    }
    s->ptr += offset;
    return 1;
  case _END:
    if (offset > 0) {
      err_message_fnc("_END: overflow (offset = %ld)\n", offset);
      return 0;
    }
    if (s->buffer_size < (unsigned long)-offset) {
      err_message_fnc("_END: underflow (offset = %ld)\n", offset);
      return 0;
    }
    s->ptr = s->buffer + s->buffer_size + offset;
    return 1;
  }

  debug_message_fnc("Invalid whence %d\n", whence);
  return 0;
}

static int
fdstream_seek(Stream *s, long offset, StreamWhence whence)
{
  switch (whence) {
  case _SET:
    if (offset < 0) {
      err_message_fnc("_SET: underflow (offset = %ld)\n", offset);
      return 0;
    }
    if (fdstream_grow(s, offset - (s->ptr - s->buffer)) < 0)
      return 0;
    s->ptr = s->buffer + offset;
    return 1;
  case _CUR:
    if (s->ptr - s->buffer + offset < 0) {
      err_message_fnc("_CUR: underflow (offset = %ld)\n", offset);
      return 0;
    }
    if (fdstream_grow(s, offset) < 0)
      return 0;
    s->ptr += offset;
    return 1;
  case _END:
    /* cannot be implemented, since we don't know end. */
    err_message_fnc("_END: cannot be implemented.\n");
    return 0;
  }

  debug_message_fnc("Invalid whence %d\n", whence);
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

  debug_message_fnc("Invalid whence %d\n", whence);
  return -1;
}

/* tell functions */

static long
dummy_tell(Stream *s __attribute__((unused)))
{
  warning_fnc("This function always returns 0\n");
  return 0;
}

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
dummy_close(Stream *s __attribute__((unused)))
{
  return 1;
}

static void
free_stream_data(Stream *s)
{
  if (s->buffer) {
    free(s->buffer);
    s->buffer = NULL;
  }
  if (s->path) {
    free(s->path);
    s->path = NULL;
  }
  if (s->format) {
    free(s->format);
    s->format = NULL;
  }
}

static int
memorystream_close(Stream *s)
{
  free_stream_data(s);
  return 1;
}

static int
fdstream_close(Stream *s)
{
  int f;

  free_stream_data(s);

  f = (close((int)(long)s->data) == 0) ? 1 : 0;
  s->data = NULL;

  return f;
}

static int
filestream_close(Stream *s)
{
  int f = 1;

  if (s->data == NULL)
    goto exit;

  f = (fclose((FILE *)s->data) == 0) ? 1 : 0;
  s->data = NULL;
  if (s->tmppath) {
    unlink(s->tmppath);
    free(s->tmppath);
    s->tmppath = NULL;
  }

 exit:
  free_stream_data(s);
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

  s->format = strdup("MEMORY");
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
  s->path = strdup("");

  s->format = strdup("FD");
  s->read = fdstream_read;
  s->seek = fdstream_seek;
  s->tell = fdstream_tell;
  s->close = fdstream_close;

  s->data = (void *)(long)fd;

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

  s->format = strdup("FILE");
  s->read = filestream_read;
  s->seek = filestream_seek;
  s->tell = filestream_tell;
  s->close = filestream_close;
  s->tmppath = NULL;

  s->data = (void *)fp;

  return 1;
}

/* Just for delete on close and set real name */
static int
make_tmpfilestream(Stream *s, char *path, char *name)
{
  FILE *fp;

  if ((fp = fopen(path, "rb")) == NULL)
    return 0;

  if ((s->path = strdup(name)) == NULL) {
    fclose(fp);
    return 0;
  }

  s->format = strdup("TMPFILE");
  s->read = filestream_read;
  s->seek = filestream_seek;
  s->tell = filestream_tell;
  s->close = filestream_close;
  s->tmppath = strdup(path);

  s->data = (void *)fp;

  return 1;
}

static void
destroy(Stream *s)
{
  if (s->close)
    stream_close(s);
  if (s->format)
    free(s->format);
  if (s->path)
    free(s->path);
  free(s);
}
