/*
 * identify.c -- File or stream identify utility functions
 * (C)Copyright 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Aug 11 04:45:19 2001.
 * $Id: identify.c,v 1.3 2001/08/15 06:38:50 sian Exp $
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

#include <sys/stat.h>
#include <errno.h>

#define REQUIRE_STRING_H
#define REQUIRE_UNISTD_H
#include "compat.h"
#include "common.h"

#include "enfle-plugins.h"
#include "streamer.h"
#include "archiver.h"
#include "image.h"
#include "movie.h"
#include "video.h"
#include "loader.h"
#include "player.h"
#include "utils/libconfig.h"
#include "identify.h"

/* The fourth argument 'a' can be NULL. */
int
identify_file(EnflePlugins *eps, char *path, Stream *s, Archive *a)
{
  struct stat statbuf;

  if (!a || strcmp(a->format, "NORMAL") == 0) {
    if (strcmp(path, "-") == 0) {
      stream_make_fdstream(s, dup(0));
      return IDENTIFY_FILE_STREAM;
    }
    if (stat(path, &statbuf)) {
      show_message("ERROR: %s: %s.\n", path, strerror(errno));
      return IDENTIFY_FILE_STAT_FAILED;
    }
    if (S_ISDIR(statbuf.st_mode))
      return IDENTIFY_FILE_DIRECTORY;
    if (!S_ISREG(statbuf.st_mode))
      return IDENTIFY_FILE_NOTREG;
    if (streamer_identify(eps, s, path)) {
      debug_message("Stream identified as %s.\n", s->format);
      if (!streamer_open(eps, s, s->format, path)) {
	show_message("Stream %s[%s] cannot open.\n", s->format, path);
	return IDENTIFY_FILE_SOPEN_FAILED;
      }
    } else if (!stream_make_filestream(s, path)) {
      show_message("Stream NORMAL[%s] cannot open.\n", path);
      return IDENTIFY_FILE_SOPEN_FAILED;
    }
  } else if (!archive_open(a, s, path)) {
    show_message("File %s in Archive %s [%s] cannot open.\n", path, a->format, a->path);
    return IDENTIFY_FILE_AOPEN_FAILED;
  }
  return IDENTIFY_FILE_STREAM;
}

int
identify_stream(EnflePlugins *eps, Image *p, Movie *m, Stream *s, VideoWindow *vw, Config *c)
{
  int f;

  f = LOAD_NOT;
  if (p && loader_identify(eps, p, s, vw, c)) {
#ifdef DEBUG
    if (p->format_detail)
      debug_message("Image identified as %s: %s\n", p->format, p->format_detail);
    else
      debug_message("Image identified as %s\n", p->format);
#endif
    if (!p->image) {
      debug_message("*** p->image == NULL ***\n");
      p->image = memory_create();
      debug_message("*** memory object p->image created (no problem) ***\n");
    }
    if ((f = loader_load(eps, p->format, p, s, vw, c)) == LOAD_OK)
      return IDENTIFY_STREAM_IMAGE;
    if (f != LOAD_NOT)
      return IDENTIFY_STREAM_IMAGE_FAILED;
  }

  if (m && player_identify(eps, m, s, c)) {
    debug_message("Movie identified as %s\n", m->format);
    if (player_load(eps, vw, m->format, m, s, c) == PLAY_OK)
      return IDENTIFY_STREAM_MOVIE;
    return IDENTIFY_STREAM_MOVIE_FAILED;
  }

  return IDENTIFY_STREAM_FAILED;
}
