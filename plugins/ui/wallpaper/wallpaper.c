/*
 * wallpaper.c -- Wallpaper UI plugin
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Jul  3 20:48:44 2001.
 * $Id: wallpaper.c,v 1.4 2001/07/10 12:59:45 sian Exp $
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

#include "utils/libstring.h"
#include "enfle/ui-plugin.h"
#include "enfle/utils.h"
#include "enfle/loader.h"
#include "enfle/player.h"
#include "enfle/streamer.h"
#include "enfle/archiver.h"

static int ui_main(UIData *);

static UIPlugin plugin = {
  type: ENFLE_PLUGIN_UI,
  name: "Wallpaper",
  description: "Wallpaper UI plugin version 0.1",
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

static void
magnify_if_requested(VideoWindow *vw, Image *p)
{
  double s, ws, hs;

  switch (vw->render_method) {
  case _VIDEO_RENDER_NORMAL:
    p->if_magnified = 0;
    if (p->rendered.image)
      memory_destroy(p->rendered.image);
    p->rendered.image = memory_dup(p->image);
    p->rendered.width  = p->magnified.width  = p->width;
    p->rendered.height = p->magnified.height = p->height;
    break;
  case _VIDEO_RENDER_MAGNIFY_DOUBLE:
    if (!image_magnify(p, p->width * 2, p->height * 2, vw->interpolate_method))
      show_message(__FUNCTION__ ": image_magnify() failed.\n");
    if (p->rendered.image)
      memory_destroy(p->rendered.image);
    p->rendered.image = memory_dup(p->magnified.image);
    break;
  case _VIDEO_RENDER_MAGNIFY_SHORT_FULL:
    ws = (double)vw->full_width  / (double)p->width;
    hs = (double)vw->full_height / (double)p->height;
    s = (ws * p->height > vw->full_height) ? hs : ws;
    image_magnify(p, s * p->width, s * p->height, vw->interpolate_method);
    if (p->rendered.image)
      memory_destroy(p->rendered.image);
    p->rendered.image = memory_dup(p->magnified.image);
    break;
  case _VIDEO_RENDER_MAGNIFY_LONG_FULL:
    ws = (double)vw->full_width  / (double)p->width;
    hs = (double)vw->full_height / (double)p->height;
    s = (ws * p->height > vw->full_height) ? ws : hs;
    image_magnify(p, s * p->width, s * p->height, vw->interpolate_method);
    if (p->rendered.image)
      memory_destroy(p->rendered.image);
    p->rendered.image = memory_dup(p->magnified.image);
    break;
  default:
    show_message(__FUNCTION__ ": invalid render_method %d\n", vw->render_method);
    p->if_magnified = 0;
    p->rendered.image = memory_dup(p->image);
    p->rendered.width  = p->magnified.width  = p->width;
    p->rendered.height = p->magnified.height = p->height;
    break;
  }
}

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
  //VideoPlugin *vp = uidata->vp;
  //EnflePlugins *eps = uidata->eps;
  //Config *c = uidata->c;
  //Effect *ef = uidata->ef;
  int loop = 1;

  if (p) {
    vw->if_direct = 0;
    magnify_if_requested(vw, p);
    video_window_set_background(vw, p);
  } else if (m) {
    vw->if_direct = 1;
    vw->render_width  = m->width;
    vw->render_height = m->height;

    while (loop) {
      switch (m->status) {
      case _PLAY:
	if (movie_play_main(m, vw) != PLAY_OK) {
	  show_message(__FUNCTION__ ": movie_play_main() failed.\n");
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
  EnflePlugins *eps = uidata->eps;
  VideoWindow *vw = uidata->vw;
  Config *c = uidata->c;
  Archive *arc;
  Stream *s;
  Image *p;
  Movie *m;
  char *path;
  int f, ret;
  struct stat statbuf;

  s = stream_create();
  p = image_create();
  m = movie_create();

  m->initialize_screen = initialize_screen;
  m->render_frame = render_frame;
  m->ap = uidata->ap;

  /* XXX: This setting should be configurable. */
  //movie_set_play_every_frame(m, 1);
  movie_set_play_every_frame(m, 0);

  path = NULL;
  if (!(path = archive_iteration_start(a)))
    return 1;

  if (strcmp(a->format, "NORMAL") == 0) {
    if (strcmp(path, "-") == 0) {
      stream_make_fdstream(s, dup(0));
    } else {
      if (stat(path, &statbuf)) {
	show_message("stat() failed: %s: %s\n", path, strerror(errno));
	return 0;
      }
      if (S_ISDIR(statbuf.st_mode)) {
	arc = archive_create(a);

	debug_message("reading %s\n", path);

	if (!archive_read_directory(arc, path, 1)) {
	  archive_destroy(arc);
	  return 0;
	}
	ret = process_files_of_archive(uidata, arc);
	archive_destroy(arc);
	return ret;
      } else if (!S_ISREG(statbuf.st_mode)) {
	return 0;
      }

      if (streamer_identify(eps, s, path)) {

	debug_message("Stream identified as %s\n", s->format);

	if (!streamer_open(eps, s, s->format, path)) {
	  show_message("Stream %s [%s] cannot open\n", s->format, path);
	  return 0;
	}
      } else if (!stream_make_filestream(s, path)) {
	show_message("Stream NORMAL [%s] cannot open\n", path);
	return 0;
      }
    }

    arc = archive_create(a);
    if (archiver_identify(eps, arc, s)) {

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
  } else if (!archive_open(a, s, path)) {
    show_message("File %s in %s archive cannot open\n", path, a->format);
    return 0;
  }

  f = LOAD_NOT;
  if (loader_identify(eps, p, s, vw, c)) {

#ifdef DEBUG
    if (p->format_detail)
      debug_message("Image identified as %s: %s\n", p->format, p->format_detail);
    else
      debug_message("Image identified as %s\n", p->format);
#endif

    p->image = memory_create();
    if ((f = loader_load(eps, p->format, p, s, vw, c)) == LOAD_OK)
      stream_close(s);
  }

  if (f != LOAD_OK) {
    if (player_identify(eps, m, s, c)) {

      debug_message("Movie identified as %s\n", m->format);

      if ((f = player_load(eps, vw, m->format, m, s, c)) != PLAY_OK) {
	stream_close(s);
	show_message("%s load failed\n", path);
	return 0;
      }
    } else {
      stream_close(s);
      show_message("%s identification failed\n", path);
      return 0;
    }

    ret = main_loop(uidata, vw, m, NULL, path);
    movie_unload(m);
  } else {

    debug_message("%s: (%d, %d) %s\n", path, p->width, p->height, image_type_to_string(p->type));

    ret = main_loop(uidata, vw, NULL, p, path);
    memory_destroy(p->rendered.image);
    p->rendered.image = NULL;
    memory_destroy(p->image);
    p->image = NULL;
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
    if (uidata->private) {
      free(uidata->private);
      uidata->private = NULL;
    }
    return 0;
  }
  uidata->disp = disp;

  uidata->vw = vw = vp->get_root(disp);

  /* video_window_set_event_mask(vw, ENFLE_ExposureMask | ENFLE_ButtonMask | ENFLE_KeyMask | ENFLE_PointerMask); */

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
