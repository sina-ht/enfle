/*
 * normal.c -- Normal UI plugin
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Dec  3 17:13:06 2000.
 * $Id: normal.c,v 1.7 2000/12/03 08:40:04 sian Exp $
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

#include "ui-plugin.h"

static int ui_main(UIData *);

static UIPlugin plugin = {
  type: ENFLE_PLUGIN_UI,
  name: "Normal",
  description: "Normal UI plugin version 0.1",
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
initialize_screen(VideoWindow *vw, Movie *m, int w, int h)
{
  video_window_resize(vw, w, h);

  return 1;
}

static int
render_frame(VideoWindow *vw, Movie *m, Image *p)
{
#if 0
  /* CHECK */
  if (p->width != m->width || p->height != m->height) {
    show_message(__FUNCTION__ ": p(%d, %d) != m(%d, %d)\n", p->width, p->height, m->width, m->height);
    exit(1);
  }
#endif

  video_window_render(vw, p);

  return 1;
}

static int
main_loop(VideoWindow *vw, Movie *m, Image *p)
{
  VideoEventData ev;
  int loop = 1;
  VideoButton button = ENFLE_Button_None;
  VideoKey key = ENFLE_KEY_Unknown;

  if (p) {
    video_window_resize(vw, p->width, p->height);
    vw->if_direct = 0;
    video_window_render(vw, p);
  } else if (m) {
    vw->if_direct = 1;
  }

  while (loop) {
    if (video_window_dispatch_event(vw, &ev)) {
      switch (ev.type) {
      case ENFLE_Event_ButtonPressed:
	button = ev.button.button;
	break;
      case ENFLE_Event_ButtonReleased:
	if (button == ev.button.button)
	  switch (ev.button.button) {
	  case ENFLE_Button_1:
	    return 1;
	  case ENFLE_Button_2:
	    return 0;
	  case ENFLE_Button_3:
	    return -1;
	  default:
	    break;
	  }
	button = ENFLE_Button_None;
	break;
      case ENFLE_Event_KeyPressed:
	key = ev.key.key;
	break;
      case ENFLE_Event_KeyReleased:
	if (key == ev.key.key) {
	  switch (ev.key.key) {
	  case ENFLE_KEY_n:
	  case ENFLE_KEY_space:
	    return 1;
	  case ENFLE_KEY_b:
	    return -1;
	  case ENFLE_KEY_q:
	    return 0;
	  case ENFLE_KEY_f:
	    video_window_set_fullscreen_mode(vw, _VIDEO_WINDOW_FULLSCREEN_TOGGLE);
	    break;
	  default:
	    break;
	  }
	}
	key = ENFLE_KEY_Unknown;
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
	break;
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
  int f;
  int dir = 1;
  struct stat statbuf;

  s = stream_create();
  p = image_create();
  m = movie_create();

  m->initialize_screen = initialize_screen;
  m->render_frame = render_frame;

  /* XXX: This setting should be configurable. */
  movie_set_play_every_frame(m, 1);

  path = NULL;
  while (dir) {
    if (path == NULL)
      path = archive_iteration_start(a);
    else {
      switch (dir) {
      case 1:
	path = archive_iteration_next(a);
	break;
      case -1:
	path = archive_iteration_prev(a);
	break;
      }
    }
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

	  if (!archive_read_directory(arc, path, 1)) {
	    archive_destroy(arc);
	    archive_iteration_delete(a);
	    continue;
	  }
	  dir = process_files_of_archive(uidata, arc);
	  if (arc->nfiles == 0) {
	    /* Now that deleted all paths in this archive, should be deleted wholly. */
	    archive_iteration_delete(a);
	  }
	  archive_destroy(arc);
	  continue;
	} else if (!S_ISREG(statbuf.st_mode)) {
	  archive_iteration_delete(a);
	  continue;
	}

	if (streamer_identify(st, eps, s, path)) {

	  debug_message("Stream identified as %s\n", s->format);

	  if (!streamer_open(st, eps, s, s->format, path)) {
	    show_message("Stream %s [%s] cannot open\n", s->format, path);
	    archive_iteration_delete(a);
	    continue;
	  }
	} else if (!stream_make_filestream(s, path)) {
	  show_message("Stream NORMAL [%s] cannot open\n", path);
	  archive_iteration_delete(a);
	  continue;
	}
      }

      arc = archive_create();
      if (archiver_identify(ar, eps, arc, s)) {

	debug_message("Archiver identified as %s\n", arc->format);

	if (archiver_open(ar, eps, arc, arc->format, s)) {
	  dir = process_files_of_archive(uidata, arc);
	  archive_destroy(arc);
	  continue;
	} else {
	  show_message("Archive %s [%s] cannot open\n", arc->format, path);
	  archive_iteration_delete(a);
	}
      }
      archive_destroy(arc);
    } else if (!archive_open(a, s, path)) {
      show_message("File %s in %s archive cannot open\n", path, a->format);
      archive_iteration_delete(a);
      continue;
    }

    f = LOAD_NOT;
    if (loader_identify(ld, eps, p, s)) {

      debug_message("Image identified as %s\n", p->format);

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
	  continue;
	}
      } else {
	stream_close(s);
	show_message("%s identification failed\n", path);
	archive_iteration_delete(a);
	continue;
      }

      dir = main_loop(vw, m, NULL);
      movie_unload(m);
    } else {

      debug_message("%s: (%d, %d)\n", path, p->width, p->height);

      if (p->comment) {
	show_message("comment:\n%s\n", p->comment);
	free(p->comment);
	p->comment = NULL;
      }

      dir = main_loop(vw, NULL, p);
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
  void *disp;

  if ((disp = vp->open_video(NULL)) == NULL) {
    show_message("open_video() failed\n");
    free(uidata->private);
    return 0;
  }

  uidata->vw = vw = vp->open_window(disp, 600, 400);
  video_window_set_caption(vw, (unsigned char *)PROGNAME " version " VERSION);

  video_window_set_event_mask(vw, ENFLE_ExposureMask | ENFLE_ButtonMask | ENFLE_KeyMask);
  /* video_window_set_event_mask(vw, ENFLE_ExposureMask | ENFLE_ButtonMask | ENFLE_KeyPressMask | ENFLE_PointerMask | ENFLE_WindowMask); */

  process_files_of_archive(uidata, uidata->a);

  video_window_destroy(vw);
  vp->close_video(disp);

  return 1;
}
