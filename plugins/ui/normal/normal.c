/*
 * normal.c -- Normal UI plugin
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Thu Feb 22 04:34:51 2001.
 * $Id: normal.c,v 1.26 2001/02/24 08:23:03 sian Exp $
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

static int ui_main(UIData *);

static UIPlugin plugin = {
  type: ENFLE_PLUGIN_UI,
  name: "Normal",
  description: "Normal UI plugin version 0.4.1",
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
  video_window_resize(vw, w, h);

  return 1;
}

static int
render_frame(VideoWindow *vw, Movie *m, Image *p)
{
  video_window_render(vw, p);

  return 1;
}

static void
set_caption_string(VideoWindow *vw, char *path, char *format)
{
  String *cap;
  char *pos;

  if ((cap = string_create()) == NULL)
    return;

  pos = strdup("(%%4d,%%4d)");
  string_set(cap, PROGNAME);
  string_cat_ch(cap, vw->interpolate_method == _BILINEAR ? ')' : '>');
  if (pos) {
    snprintf(pos, strlen(pos) + 1, "(%4d,%4d)", vw->render_width, vw->render_height);
    string_cat(cap, pos);
    free(pos);
  }
  string_cat_ch(cap, ' ');
  string_cat(cap, path);

  string_cat_ch(cap, '(');
  string_cat(cap, format);
  string_cat_ch(cap, ')');

  video_window_set_caption(vw, string_get(cap));

  string_destroy(cap);
}

#define MAIN_LOOP_QUIT 0
#define MAIN_LOOP_NEXT 1
#define MAIN_LOOP_PREV -1
#define MAIN_LOOP_DELETE_FROM_LIST 2
#define MAIN_LOOP_DELETE_FILE 3
#define MAIN_LOOP_DO_NOTHING 4

static int
main_loop(UIData *uidata, VideoWindow *vw, Movie *m, Image *p, char *path)
{
  VideoPlugin *vp = uidata->vp;
  VideoEventData ev;
  int loop = 1;
  VideoButton button = ENFLE_Button_None;
  VideoKey key = ENFLE_KEY_Unknown;
  unsigned int old_x = 0, old_y = 0;
  int first_point = 1;
  int offset_x, offset_y;

  if (p) {
    vw->if_direct = 0;
    magnify_if_requested(vw, p);
    video_window_render(vw, p);
    set_caption_string(vw, path, p->format);
  } else if (m) {
    vw->if_direct = 1;
    vw->render_width  = m->width;
    vw->render_height = m->height;
    set_caption_string(vw, path, m->format);
  }

  video_window_set_offset(vw, 0, 0);

  while (loop) {
    if (video_window_dispatch_event(vw, &ev)) {
      /* XXX: Key bindings should be configurable. */
      switch (ev.type) {
      case ENFLE_Event_ButtonPressed:
	button = ev.button.button;
	if (button == ENFLE_Button_1)
	  first_point = 1;
	break;
      case ENFLE_Event_ButtonReleased:
	if (button == ev.button.button) {
	  video_window_sync_discard(vw);
	  switch (ev.button.button) {
	  case ENFLE_Button_1:
	    return MAIN_LOOP_NEXT;
	  case ENFLE_Button_2:
	    return MAIN_LOOP_QUIT;
	  case ENFLE_Button_3:
	    return MAIN_LOOP_PREV;
	  case ENFLE_Button_4:
	    return MAIN_LOOP_NEXT;
	  case ENFLE_Button_5:
	    return MAIN_LOOP_PREV;
	  default:
	    break;
	  }
	}
	button = ENFLE_Button_None;
	break;
      case ENFLE_Event_KeyPressed:
	key = ev.key.key;
	break;
      case ENFLE_Event_KeyReleased:
	if (key == ev.key.key) {
	  video_window_sync_discard(vw);
	  switch (ev.key.key) {
	  case ENFLE_KEY_n:
	  case ENFLE_KEY_space:
	    return MAIN_LOOP_NEXT;
	  case ENFLE_KEY_b:
	    return MAIN_LOOP_PREV;
	  case ENFLE_KEY_q:
	    return MAIN_LOOP_QUIT;
	  case ENFLE_KEY_f:
	    video_window_set_fullscreen_mode(vw, _VIDEO_WINDOW_FULLSCREEN_TOGGLE);
	    break;
	  case ENFLE_KEY_d:
	    return (ev.key.modkey & ENFLE_MOD_Shift) ?
	      MAIN_LOOP_DELETE_FILE : MAIN_LOOP_DELETE_FROM_LIST;
	  case ENFLE_KEY_s:
	    if (ev.key.modkey & ENFLE_MOD_Shift) {
	      switch (vw->interpolate_method) {
	      case _NOINTERPOLATE:
		vw->interpolate_method = _BILINEAR;
		break;
	      case _BILINEAR:
		vw->interpolate_method = _NOINTERPOLATE;
		break;
	      default:
		show_message(__FUNCTION__ ": invalid interpolate method %d\n", vw->interpolate_method);
		vw->interpolate_method = _NOINTERPOLATE;
		break;
	      }
	      magnify_if_requested(vw, p);
	      video_window_render(vw, p);
	      set_caption_string(vw, path, p ? p->format : m->format);
	    }
	    break;
	  case ENFLE_KEY_m:
	    switch (vw->render_method) {
	    case _VIDEO_RENDER_NORMAL:
	      if (ev.key.modkey & ENFLE_MOD_Shift)
		vw->render_method = _VIDEO_RENDER_MAGNIFY_SHORT_FULL;
	      else if (ev.key.modkey & ENFLE_MOD_Alt)
		vw->render_method = _VIDEO_RENDER_MAGNIFY_LONG_FULL;
	      else
		vw->render_method = _VIDEO_RENDER_MAGNIFY_DOUBLE;
	      break;
	    case _VIDEO_RENDER_MAGNIFY_DOUBLE:
	    case _VIDEO_RENDER_MAGNIFY_SHORT_FULL:
	    case _VIDEO_RENDER_MAGNIFY_LONG_FULL:
	      vw->render_method = _VIDEO_RENDER_NORMAL;
	      break;
	    default:
	      break;
	    }
	    if (p) {
	      magnify_if_requested(vw, p);
	      video_window_resize(vw, p->magnified.width, p->magnified.height);
	      video_window_render(vw, p);
	      video_window_set_offset(vw, 0, 0);
	    }
	    break;
	  case ENFLE_KEY_w:
	    if (p) 
	      vp->set_wallpaper(uidata->disp, p);
	    break;
	  default:
	    break;
	  }
	}
	key = ENFLE_KEY_Unknown;
	break;
      case ENFLE_Event_PointerMoved:
	if (ev.pointer.button & ENFLE_Button_1) {
	  if (first_point) {
	    old_x = ev.pointer.x;
	    old_y = ev.pointer.y;
	    first_point = 0;
	  } else {
	    offset_x = ev.pointer.x - old_x;
	    offset_y = ev.pointer.y - old_y;
	    old_x = ev.pointer.x;
	    old_y = ev.pointer.y;

	    if (offset_x != 0 || offset_y != 0) {
	      button = ENFLE_Button_None;
	      video_window_adjust_offset(vw, offset_x, offset_y);
	    }
	  }
	}
	break;
      default:
	break;
      }
    }

    if (m) {
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
  Loader *ld = uidata->ld;
  Streamer *st = uidata->st;
  Archiver *ar = uidata->ar;
  Player *player = uidata->player;
  VideoWindow *vw = uidata->vw;
  Archive *arc;
  Stream *s;
  Image *p;
  Movie *m;
  char *path;
  int f, dir, ret;
  struct stat statbuf;

  s = stream_create();
  p = image_create();
  m = movie_create(uidata->c);

  m->initialize_screen = initialize_screen;
  m->render_frame = render_frame;
  m->ap = uidata->ap;

  /* XXX: This setting should be configurable. */
  //movie_set_play_every_frame(m, 1);
  movie_set_play_every_frame(m, 0);

  path = NULL;
  ret = MAIN_LOOP_DO_NOTHING;
  dir = 1;
  while (ret != MAIN_LOOP_QUIT) {
    if (path == NULL)
      path = archive_iteration_start(a);
    else {
      switch (ret) {
      case MAIN_LOOP_DELETE_FROM_LIST:
	archive_iteration_delete(a);
	path = archive_iteration(a);
	break;
      case MAIN_LOOP_DELETE_FILE:
	if (strcmp(a->format, "NORMAL") == 0) {
	  unlink(s->path);
	  fprintf(stderr, "DELETED: %s\n", s->path);
	}
	archive_iteration_delete(a);
	path = archive_iteration(a);
	break;
      case MAIN_LOOP_NEXT:
	dir = 1;
	path = archive_iteration_next(a);
	break;
      case MAIN_LOOP_PREV:
	dir = -1;
	path = archive_iteration_prev(a);
	break;
      case MAIN_LOOP_DO_NOTHING:
	break;
      default:
	fprintf(stderr, "*** main_loop() returned unknown code %d\n", ret);
	path = NULL;
	break;
      }
    }

    video_window_set_cursor(vw, _VIDEO_CURSOR_NORMAL);

    if (!path)
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
	  arc = archive_create();

	  debug_message("reading %s\n", path);

	  video_window_set_cursor(vw, _VIDEO_CURSOR_WAIT);
	  if (!archive_read_directory(arc, path, 1)) {
	    archive_destroy(arc);
	    archive_iteration_delete(a);
	    path = archive_iteration(a);
	    continue;
	  }
	  video_window_set_cursor(vw, _VIDEO_CURSOR_NORMAL);
	  dir = process_files_of_archive(uidata, arc);
	  if (arc->nfiles == 0) {
	    /* Now that deleted all paths in this archive, should be deleted wholly. */
	    archive_iteration_delete(a);
	  }
	  archive_destroy(arc);
	  ret = (dir == 1) ? MAIN_LOOP_NEXT : MAIN_LOOP_PREV;
	  continue;
	} else if (!S_ISREG(statbuf.st_mode)) {
	  archive_iteration_delete(a);
	  path = archive_iteration(a);
	  continue;
	}

	if (streamer_identify(st, eps, s, path)) {

	  debug_message("Stream identified as %s\n", s->format);

	  if (!streamer_open(st, eps, s, s->format, path)) {
	    show_message("Stream %s [%s] cannot open\n", s->format, path);
	    archive_iteration_delete(a);
	    path = archive_iteration(a);
	    continue;
	  }
	} else if (!stream_make_filestream(s, path)) {
	  show_message("Stream NORMAL [%s] cannot open\n", path);
	  archive_iteration_delete(a);
	  path = archive_iteration(a);
	  continue;
	}
      }

      arc = archive_create();
      if (archiver_identify(ar, eps, arc, s)) {

	debug_message("Archiver identified as %s\n", arc->format);

	if (archiver_open(ar, eps, arc, arc->format, s)) {
	  dir = process_files_of_archive(uidata, arc);
	  archive_destroy(arc);
	  ret = (dir == 1) ? MAIN_LOOP_NEXT : MAIN_LOOP_PREV;
	  continue;
	} else {
	  show_message("Archive %s [%s] cannot open\n", arc->format, path);
	  archive_iteration_delete(a);
	  path = archive_iteration(a);
	}
      }
      archive_destroy(arc);
    } else if (!archive_open(a, s, path)) {
      show_message("File %s in %s archive cannot open\n", path, a->format);
      archive_iteration_delete(a);
      path = archive_iteration(a);
      continue;
    }

    f = LOAD_NOT;
    video_window_set_cursor(vw, _VIDEO_CURSOR_WAIT);
    if (loader_identify(ld, eps, p, s)) {

      debug_message("Image identified as %s\n", p->format);

      p->image = memory_create();
      if ((f = loader_load_image(ld, eps, p->format, p, s)) == LOAD_OK)
	stream_close(s);
    }

    if (f != LOAD_OK) {
      if (player_identify(player, eps, m, s)) {

	debug_message("Movie(Animation) identified as %s\n", m->format);

	if ((f = player_load_movie(player, eps, vw, m->format, m, s)) != PLAY_OK) {
	  stream_close(s);
	  show_message("%s load failed\n", path);
	  archive_iteration_delete(a);
	  path = archive_iteration(a);
	  continue;
	}
      } else {
	stream_close(s);
	show_message("%s identification failed\n", path);
	archive_iteration_delete(a);
	path = archive_iteration(a);
	continue;
      }

      video_window_set_cursor(vw, _VIDEO_CURSOR_NORMAL);
      ret = main_loop(uidata, vw, m, NULL, path);
      movie_unload(m);
    } else {

      debug_message("%s: (%d, %d) %s\n", path, p->width, p->height, image_type_to_string(p->type));

      if (p->comment) {
	show_message("comment:\n%s\n", p->comment);
	free(p->comment);
	p->comment = NULL;
      }

      video_window_set_cursor(vw, _VIDEO_CURSOR_NORMAL);
      ret = main_loop(uidata, vw, NULL, p, path);
      memory_destroy(p->rendered.image);
      p->rendered.image = NULL;
      memory_destroy(p->image);
      p->image = NULL;
    }
  }

  movie_destroy(m);
  image_destroy(p);
  stream_destroy(s);

  return dir;
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

  if ((disp = vp->open_video(NULL)) == NULL) {
    show_message("open_video() failed\n");
    free(uidata->private);
    return 0;
  }
  uidata->disp = disp;

  uidata->vw = vw = vp->open_window(disp, c, 600, 400);

  video_window_set_event_mask(vw, ENFLE_ExposureMask | ENFLE_ButtonMask | ENFLE_KeyMask | ENFLE_PointerMask);
  /* video_window_set_event_mask(vw, ENFLE_ExposureMask | ENFLE_ButtonMask | ENFLE_KeyMask | ENFLE_PointerMask | ENFLE_WindowMask); */

  if ((render_method = config_get(c, "/enfle/plugins/ui/normal/render")) != NULL) {
    if (!strcasecmp(render_method, "normal")) {
      vw->render_method = _VIDEO_RENDER_NORMAL;
    } else if (!strcasecmp(render_method, "double")) {
      vw->render_method = _VIDEO_RENDER_MAGNIFY_DOUBLE;
    } else if (!strcasecmp(render_method, "short")) {
      vw->render_method = _VIDEO_RENDER_MAGNIFY_SHORT_FULL;
    } else if (!strcasecmp(render_method, "long")) {
      vw->render_method = _VIDEO_RENDER_MAGNIFY_LONG_FULL;
    } else {
      show_message("Invalid ui/normal/render = %s\n", render_method);
      vw->render_method = _VIDEO_RENDER_NORMAL;
    }
  }

  if ((interpolate_method = config_get(c, "/enfle/plugins/ui/normal/magnify_interpolate")) != NULL) {
    if (!strcasecmp(interpolate_method, "no")) {
      vw->interpolate_method = _NOINTERPOLATE;
    } else if (!strcasecmp(interpolate_method, "bilinear")) {
      vw->interpolate_method = _BILINEAR;
    } else {
      show_message("Invalid ui/normal/magnify_interpolate = %s\n", interpolate_method);
      vw->interpolate_method = _NOINTERPOLATE;
    }
  }

  process_files_of_archive(uidata, uidata->a);

  video_window_destroy(vw);
  vp->close_video(disp);

  return 1;
}
