/*
 * convert.c -- Convert UI plugin
 * (C)Copyright 2000-2005 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Nov 18 23:03:48 2011.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include <stdlib.h>

#include <fcntl.h>

#define REQUIRE_STRING_H
#define REQUIRE_UNISTD_H
#include "compat.h"

#include "common.h"

#include "utils/libstring.h"
#include "utils/misc.h"
#include "enfle/ui-plugin.h"
#include "enfle/loader.h"
#include "enfle/player.h"
#include "enfle/saver.h"
#include "enfle/streamer.h"
#include "enfle/archiver.h"
#include "enfle/identify.h"

static int ui_main(UIData *);

static UIPlugin plugin = {
  .type = ENFLE_PLUGIN_UI,
  .name = "Convert",
  .description = "Convert UI plugin version 0.1.5",
  .author = "Hiroshi Takekawa",

  .ui_main = ui_main,
};

ENFLE_PLUGIN_ENTRY(ui_convert)
{
  UIPlugin *uip;

  if ((uip = (UIPlugin *)calloc(1, sizeof(UIPlugin))) == NULL)
    return NULL;
  memcpy(uip, &plugin, sizeof(UIPlugin));

  return (void *)uip;
}

ENFLE_PLUGIN_EXIT(ui_convert, p)
{
  free(p);
}

/* for internal use */

static int
save_image(UIData *uidata, Image *p, char *format, char *path)
{
  EnflePlugins *eps = get_enfle_plugins();
  char *outpath;
  char *ext;
  FILE *fp;
  int fd;

  if (strcasecmp(format, "test") == 0 ||
      strcasecmp(format, "null") == 0)
    return 1;

  if ((ext = saver_get_ext(eps, format, uidata->c)) == NULL)
    return 0;
  if ((outpath = misc_replace_ext(path, ext)) == NULL) {
    show_message_fnc("No enough memory.\n");
    return 0;
  }
  free(ext);

  if ((fd = open(outpath, O_WRONLY)) >= 0) {
    close(fd);
    show_message_fnc("file %s exists\n", outpath);
    free(outpath);
    return 0;
  }

  if ((fp = fopen(outpath, "wb+")) == NULL) {
    show_message_fnc("Cannot open %s for writing.\n", outpath);
    free(outpath);
    return 0;
  }

  config_set_str(uidata->c, (char *)"/enfle/plugins/ui/convert/source_path", outpath);
  if (!saver_save(eps, format, p, fp, uidata->c, NULL)) {
    show_message("Save failed.\n");
    unlink(outpath);
  }
  fclose(fp);
  /* outpath must not be free'd here */

  return 1;
}

static int
process_files_of_archive(UIData *uidata, Archive *a)
{
  EnflePlugins *eps = get_enfle_plugins();
  Config *c = uidata->c;
  Archive *arc;
  Stream *s;
  Image *p;
  char *path, *format;
  int ret = 0, r;

  s = stream_create();
  p = image_create();
  format = config_get_str(c, "/enfle/plugins/ui/convert/format");

  path = NULL;
  for (;;) {
    if ((path = (path == NULL) ? archive_iteration_start(a) : archive_iteration_next(a)) == NULL)
      break;

    r = identify_file(eps, path, s, a, c);
    switch (r) {
    case IDENTIFY_FILE_DIRECTORY:
      arc = archive_create(a);
      debug_message("Reading %s.\n", path);
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
      continue;
    case IDENTIFY_FILE_STREAM:
      arc = archive_create(a);
      if (archiver_identify(eps, arc, s, c)) {
	debug_message("Archiver identified as %s\n", arc->format);
	if (archiver_open(eps, arc, arc->format, s)) {
	  ret = process_files_of_archive(uidata, arc);
	  archive_destroy(arc);
	  continue;
	} else {
	  show_message("Archive %s [%s] cannot open\n", arc->format, path);
	  archive_destroy(arc);
	  continue;
	}
      }
      archive_destroy(arc);
      break;
    case IDENTIFY_FILE_NOTREG:
    case IDENTIFY_FILE_SOPEN_FAILED:
    case IDENTIFY_FILE_AOPEN_FAILED:
    case IDENTIFY_FILE_STAT_FAILED:
    default:
      continue;
    }

    r = identify_stream(eps, p, NULL, s, NULL, c);
    stream_close(s);
    switch (r) {
    case IDENTIFY_STREAM_MOVIE_FAILED:
    case IDENTIFY_STREAM_IMAGE_FAILED:
      show_message("%s load failed\n", path);
      continue;
    case IDENTIFY_STREAM_FAILED:
      show_message("%s identification failed\n", path);
      continue;
    case IDENTIFY_STREAM_IMAGE:
      debug_message("%s: (%d, %d) %s\n", path, image_width(p), image_height(p), image_type_to_string(p->type));
      if (p->comment) {
	show_message("comment:\n%s\n", p->comment);
	free(p->comment);
	p->comment = NULL;
      }
      {
	char *fullpath = archive_getpathname(a, path);
	save_image(uidata, p, format, fullpath);
	free(fullpath);
      }
      memory_destroy(image_image(p));
      image_image(p) = NULL;
      continue;
    case IDENTIFY_STREAM_MOVIE:
      show_message("BUG... cannot reach here.\n");
      continue;
    default:
      continue;
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
  debug_message("Convert: %s()\n", __FUNCTION__);

  process_files_of_archive(uidata, uidata->a);

  return 1;
}
