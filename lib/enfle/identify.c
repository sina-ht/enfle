/*
 * identify.c -- File or stream identify utility functions
 * (C)Copyright 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Mar 22 16:34:49 2008.
 * $Id: identify.c,v 1.18 2008/04/19 08:58:11 sian Exp $
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

#include <stdlib.h>
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
identify_file(EnflePlugins *eps, char *path, Stream *s, Archive *a, Config *c)
{
  struct stat statbuf;

  if (!a || strcmp(a->format, "NORMAL") == 0) {
    char *fullpath, *tmp;

    if (strcmp(path, "-") == 0) {
      stream_make_fdstream(s, dup(0));
      return IDENTIFY_FILE_STREAM;
    }

    if ((fullpath = archive_getpathname(a, path)) == NULL)
      return IDENTIFY_FILE_MEMORY_ERROR;

    if (stat(fullpath, &statbuf)) {
      err_message("%s: %s.\n", fullpath, strerror(errno));
      free(fullpath);
      return IDENTIFY_FILE_STAT_FAILED;
    }
    if (S_ISDIR(statbuf.st_mode)) {
      free(fullpath);
      return IDENTIFY_FILE_DIRECTORY;
    }
    if (!S_ISREG(statbuf.st_mode)) {
      free(fullpath);
      return IDENTIFY_FILE_NOTREG;
    }
    if (statbuf.st_size == 0) {
      free(fullpath);
      return IDENTIFY_FILE_ZERO_SIZE;
    }

    if ((tmp = config_get_str(c, "/enfle/identify/streamer/disabled")) == NULL ||
	strcasecmp(tmp, "yes") != 0) {
      if (streamer_identify(eps, s, fullpath, c)) {
	debug_message("Stream identified as %s.\n", s->format);
	if (!streamer_open(eps, s, s->format, fullpath)) {
	  show_message("Stream %s[%s] cannot open.\n", s->format, fullpath);
	  free(fullpath);
	  return IDENTIFY_FILE_SOPEN_FAILED;
	}
	return IDENTIFY_FILE_STREAM;
      }
    }
    if (!stream_make_filestream(s, fullpath)) {
      show_message("Stream NORMAL[%s] cannot open.\n", fullpath);
      free(fullpath);
      return IDENTIFY_FILE_SOPEN_FAILED;
    }
    free(fullpath);
  } else if (archive_open(a, s, path) != OPEN_OK) {
    show_message("File %s in Archive %s[%s] cannot open.\n", path, a->format, a->path);
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
    if (!image_image(p))
      image_image(p) = memory_create();
    if ((f = loader_load(eps, p->format, p, s, vw, c)) == LOAD_OK)
      return IDENTIFY_STREAM_IMAGE;
    if (f != LOAD_NOT)
      return IDENTIFY_STREAM_IMAGE_FAILED;
  }

  /* for generic player */
  if (m && demultiplexer_identify(eps, m, s, c) == DEMULTIPLEX_OK) {
    debug_message_fnc("demultiplexer_identify() OK.\n");
    m->channels = m->samplerate = m->bitrate = m->block_align = m->timer_offset_set = 0;
    if ((m->demux = demultiplexer_examine(eps, m->format, m, s, c)) == NULL) 
      return IDENTIFY_STREAM_MOVIE_FAILED;
    debug_message_fnc("demultiplexer_examine() OK.\n");
    if (player_load(eps, vw, (char *)"generic", m, s, c) != PLAY_OK) {
      demultiplexer_destroy(m->demux);
      return IDENTIFY_STREAM_MOVIE_FAILED;
    }
    debug_message_fnc("player_load() OK.\n");
    return IDENTIFY_STREAM_MOVIE;
  }

  /* for mng, ungif player */
  if (m && player_identify(eps, m, s, c)) {
    if (player_load(eps, vw, m->player_name, m, s, c) != PLAY_OK)
      return IDENTIFY_STREAM_MOVIE_FAILED;
    return IDENTIFY_STREAM_MOVIE;
  }

  return IDENTIFY_STREAM_FAILED;
}
