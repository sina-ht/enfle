/*
 * convert.c -- Convert UI plugin
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue May 22 17:50:28 2001.
 * $Id: convert.c,v 1.6 2001/05/23 12:14:02 sian Exp $
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define REQUIRE_STRING_H
#define REQUIRE_UNISTD_H
#include "compat.h"

#include "common.h"

#include "utils/libstring.h"
#include "utils/misc.h"
#include "enfle/ui-plugin.h"

static int ui_main(UIData *);

static UIPlugin plugin = {
  type: ENFLE_PLUGIN_UI,
  name: "Convert",
  description: "Convert UI plugin version 0.1.1",
  author: "Hiroshi Takekawa",

  ui_main: ui_main,
};

void *
plugin_entry(void)
{
  UIPlugin *uip;

  if ((uip = (UIPlugin *)calloc(1, sizeof(UIPlugin))) == NULL)
    return NULL;
  memcpy(uip, &plugin, sizeof(UIPlugin));

  return (void *)uip;
}

void
plugin_exit(void *p)
{
  free(p);
}

/* for internal use */

static int
save_image(UIData *uidata, Image *p, char *format, char *path)
{
  EnflePlugins *eps = uidata->eps;
  Saver *sv = uidata->sv;
  char *outpath;
  char *ext;
  FILE *fp;
  int fd;

  if ((ext = saver_get_ext(uidata->sv, uidata->eps, format, uidata->c)) == NULL)
    return 0;
  if ((outpath = misc_replace_ext(path, ext)) == NULL) {
    show_message(__FUNCTION__ ": No enough memory.\n");
    return 0;
  }
  free(ext);

  if ((fd = open(outpath, O_WRONLY)) >= 0) {
    close(fd);
    show_message(__FUNCTION__ ": file %s exists\n", outpath);
    return 0;
  }
  if ((fp = fopen(outpath, "wb")) == NULL) {
    show_message(__FUNCTION__ ": Cannot open %s for writing.\n", outpath);
    free(outpath);
    return 0;
  } else {
    config_set_str(uidata->c, (char *)"/enfle/plugins/ui/convert/source_path", outpath);
    if (!saver_save(sv, eps, format, p, fp, uidata->c))
      show_message("Save failed.\n");
    fclose(fp);
  }
  free(outpath);

  return 1;
}

static int
process_files_of_archive(UIData *uidata, Archive *a)
{
  EnflePlugins *eps = uidata->eps;
  Config *c = uidata->c;
  Loader *ld = uidata->ld;
  Streamer *st = uidata->st;
  Archiver *ar = uidata->ar;
  Archive *arc;
  Stream *s;
  Image *p;
  char *path, *format;
  int f, dir, ret = 0;
  struct stat statbuf;

  s = stream_create();
  p = image_create();
  format = config_get_str(c, "/enfle/plugins/ui/convert/format");

  path = NULL;
  dir = 1;
  for (;;) {
    if ((path = (path == NULL) ? archive_iteration_start(a) : archive_iteration_next(a)) == NULL)
      break;

    if (strcmp(a->format, "NORMAL") == 0) {
      if (strcmp(path, "-") == 0) {
	stream_make_fdstream(s, dup(0));
      } else {
	if (stat(path, &statbuf)) {
	  show_message("stat() failed: %s: %s\n", path, strerror(errno));
	  break;
	}
	if (S_ISDIR(statbuf.st_mode)) {
	  arc = archive_create(a);

	  debug_message("reading %s\n", path);

	  if (!archive_read_directory(arc, path, 1)) {
	    archive_destroy(arc);
	    continue;
	  }
	  ret = process_files_of_archive(uidata, arc);
	  if (arc->nfiles == 0) {
	    /* Now that all paths are deleted in this archive, should be deleted wholly. */
	    archive_iteration_delete(a);
	  }
	  archive_destroy(arc);
	  dir = 1;
	  continue;
	} else if (!S_ISREG(statbuf.st_mode)) {
	  continue;
	}

	if (streamer_identify(st, eps, s, path)) {

	  debug_message("Stream identified as %s\n", s->format);

	  if (!streamer_open(st, eps, s, s->format, path)) {
	    show_message("Stream %s [%s] cannot open\n", s->format, path);
	    continue;
	  }
	} else if (!stream_make_filestream(s, path)) {
	  show_message("Stream NORMAL [%s] cannot open\n", path);
	  continue;
	}
      }

      arc = archive_create(a);
      if (archiver_identify(ar, eps, arc, s)) {

	debug_message("Archiver identified as %s\n", arc->format);

	if (archiver_open(ar, eps, arc, arc->format, s)) {
	  ret = process_files_of_archive(uidata, arc);
	  archive_destroy(arc);
	  dir = 1;
	  continue;
	} else {
	  show_message("Archive %s [%s] cannot open\n", arc->format, path);
	}
      }
      archive_destroy(arc);
    } else if (!archive_open(a, s, path)) {
      show_message("File %s in %s archive cannot open\n", path, a->format);
      continue;
    }

    f = LOAD_NOT;
    debug_message("Image identifying...\n");
    if (loader_identify(ld, eps, p, s)) {

      debug_message("Image identified as %s\n", p->format);

      p->image = memory_create();
      if ((f = loader_load_image(ld, eps, p->format, p, s)) == LOAD_OK)
	stream_close(s);
    }

    if (f != LOAD_OK) {
	stream_close(s);
	show_message("%s identification failed\n", path);
	continue;
    } else {

      debug_message("%s: (%d, %d) %s\n", path, p->width, p->height, image_type_to_string(p->type));

      if (p->comment) {
	show_message("comment:\n%s\n", p->comment);
	free(p->comment);
	p->comment = NULL;
      }

      save_image(uidata, p, format, path);

      memory_destroy(p->image);
      p->image = NULL;
    }
  }

  image_destroy(p);
  stream_destroy(s);

  return ret;
}

/* methods */

static int
ui_main(UIData *uidata)
{
  debug_message("Convert: " __FUNCTION__ "()\n");

  process_files_of_archive(uidata, uidata->a);

  return 1;
}
