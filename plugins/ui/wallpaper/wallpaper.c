/*
 * wallpaper.c -- Wallpaper UI plugin
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Mar  6 12:06:08 2004.
 * $Id: wallpaper.c,v 1.15 2004/03/06 03:43:36 sian Exp $
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

#define REQUIRE_STRING_H
#define REQUIRE_UNISTD_H
#include "compat.h"

#include "common.h"

#include "utils/libstring.h"
#include "enfle/ui-plugin.h"
#include "enfle/loader.h"
#include "enfle/player.h"
#include "enfle/streamer.h"
#include "enfle/archiver.h"
#include "enfle/identify.h"

static int ui_main(UIData *);

static UIPlugin plugin = {
  .type = ENFLE_PLUGIN_UI,
  .name = "Wallpaper",
  .description = "Wallpaper UI plugin version 0.1.2",
  .author = "Hiroshi Takekawa",

  .ui_main = ui_main,
};

ENFLE_PLUGIN_ENTRY(ui_wallpaper)
{
  UIPlugin *uip;

  if ((uip = (UIPlugin *)calloc(1, sizeof(UIPlugin))) == NULL)
    return NULL;
  memcpy(uip, &plugin, sizeof(UIPlugin));

  return (void *)uip;
}

ENFLE_PLUGIN_EXIT(ui_wallpaper, p)
{
  free(p);
}

/* for internal use */

static int
initialize_screen(VideoWindow *vw, Movie *m, int w, int h)
{
  /* do nothing */

  return 1;
}

static int
render_frame(VideoWindow *vw, Movie *m, Image *p)
{
  video_window_set_background(vw, p);

  return 1;
}

static int
main_loop(UIData *uidata, VideoWindow *vw, Movie *m, Image *p, char *path)
{
  int loop = 1;

  if (p) {
    vw->if_direct = 0;
    video_window_set_background(vw, p);
  } else if (m) {
    vw->if_direct = 1;
    vw->render_width  = m->width;
    vw->render_height = m->height;

    while (loop) {
      switch (m->status) {
      case _PLAY:
      case _RESIZING:
	if (movie_play_main(m, vw) != PLAY_OK) {
	  show_message_fnc("movie_play_main() failed.\n");
	  return 0;
	}
	break;
      case _PAUSE:
	break;
      case _STOP:
	/* loop */
	movie_play(m);
	break;
      case _UNLOADED:
	show_message("Movie has been already unloaded.\n");
	return 0;
      }
    }
  }

  return 0;
}

static int
process_files_of_archive(UIData *uidata, Archive *a)
{
  EnflePlugins *eps = get_enfle_plugins();
  VideoWindow *vw = uidata->vw;
  Config *c = uidata->c;
  Archive *arc;
  Stream *s;
  Image *p;
  Movie *m;
  char *path;
  int ret, r;

  s = stream_create();
  p = image_create();
  m = movie_create();

  m->initialize_screen = initialize_screen;
  m->render_frame = render_frame;
  m->ap = uidata->ap;

  path = NULL;
  if (!(path = archive_iteration_start(a)))
    return 1;

  r = identify_file(eps, path, s, a, c);
  switch (r) {
  case IDENTIFY_FILE_DIRECTORY:
    arc = archive_create(a);
    debug_message("Reading %s.\n", path);
    if (!archive_read_directory(arc, path, 1)) {
      archive_destroy(arc);
      return 0;
    }
    ret = process_files_of_archive(uidata, arc);
    archive_destroy(arc);
    return ret;
  case IDENTIFY_FILE_STREAM:
    arc = archive_create(a);
    if (archiver_identify(eps, arc, s, c)) {
      debug_message("Archiver identified as %s\n", arc->format);
      if (archiver_open(eps, arc, arc->format, s)) {
	ret = process_files_of_archive(uidata, arc);
	archive_destroy(arc);
	return ret;
      } else {
	show_message("Archive %s [%s] cannot open\n", arc->format, path);
	archive_destroy(arc);
	return 0;
      }
    }
    archive_destroy(arc);
    break;
  case IDENTIFY_FILE_NOTREG:
  case IDENTIFY_FILE_SOPEN_FAILED:
  case IDENTIFY_FILE_AOPEN_FAILED:
  case IDENTIFY_FILE_STAT_FAILED:
  default:
    return 0;
  }

  r = identify_stream(eps, p, m, s, vw, c);
  switch (r) {
  case IDENTIFY_STREAM_MOVIE_FAILED:
  case IDENTIFY_STREAM_IMAGE_FAILED:
    stream_close(s);
    show_message("%s load failed\n", path);
    return 0;
  case IDENTIFY_STREAM_FAILED:
    stream_close(s);
    show_message("%s identification failed\n", path);
    return 0;
  case IDENTIFY_STREAM_IMAGE:
    debug_message("%s: (%d, %d) %s\n", path, image_width(p), image_height(p), image_type_to_string(p->type));
    ret = main_loop(uidata, vw, NULL, p, path);
    memory_destroy(image_rendered_image(p));
    image_rendered_image(p) = NULL;
    memory_destroy(image_image(p));
    image_image(p) = NULL;
    break;
  case IDENTIFY_STREAM_MOVIE:
    ret = main_loop(uidata, vw, m, NULL, path);
    movie_unload(m);
    break;
  default:
    return 0;
  }

  movie_destroy(m);
  image_destroy(p);
  stream_destroy(s);

  return 1;
}

/* methods */

static int
ui_main(UIData *uidata)
{
  VideoPlugin *vp = uidata->vp;
  VideoWindow *vw;
  Config *c = uidata->c;
  void *disp;
  char *render_method, *interpolate_method;

  if ((disp = vp->open_video(NULL, c)) == NULL) {
    show_message("open_video() failed\n");
    if (uidata->priv) {
      free(uidata->priv);
      uidata->priv = NULL;
    }
    return 0;
  }
  uidata->disp = disp;

  uidata->vw = vw = vp->get_root(disp);

  if ((render_method = config_get(c, "/enfle/plugins/ui/wallpaper/render")) != NULL) {
    if (!strcasecmp(render_method, "normal")) {
      vw->render_method = _VIDEO_RENDER_NORMAL;
    } else if (!strcasecmp(render_method, "double")) {
      vw->render_method = _VIDEO_RENDER_MAGNIFY_DOUBLE;
    } else if (!strcasecmp(render_method, "short")) {
      vw->render_method = _VIDEO_RENDER_MAGNIFY_SHORT_FULL;
    } else if (!strcasecmp(render_method, "long")) {
      vw->render_method = _VIDEO_RENDER_MAGNIFY_LONG_FULL;
    } else {
      show_message("Invalid ui/wallpaper/render = %s\n", render_method);
      vw->render_method = _VIDEO_RENDER_NORMAL;
    }
  }

  if ((interpolate_method = config_get(c, "/enfle/plugins/ui/wallpaper/magnify_interpolate")) != NULL) {
    if (!strcasecmp(interpolate_method, "no")) {
      vw->interpolate_method = _NOINTERPOLATE;
    } else if (!strcasecmp(interpolate_method, "bilinear")) {
      vw->interpolate_method = _BILINEAR;
    } else {
      show_message("Invalid ui/wallpaper/magnify_interpolate = %s\n", interpolate_method);
      vw->interpolate_method = _NOINTERPOLATE;
    }
  }

  process_files_of_archive(uidata, uidata->a);

  vp->close_video(disp);

  return 1;
}
