/*
 * Xlib.c -- Xlib UI plugin
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Oct 10 17:59:58 2000.
 * $Id: Xlib.c,v 1.3 2000/10/10 11:49:18 sian Exp $
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
#include <string.h>

#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "common.h"

#include "x11.h"
#include "x11window.h"
#include "x11ximage.h"

#include "ui-plugin.h"

typedef struct {
  X11 *x11;
  X11Window *xw;
  XImage *ximage;
} Xlib_info;

static int ui_main(UIData *);

static UIPlugin plugin = {
  type: ENFLE_PLUGIN_UI,
  name: "Xlib",
  description: "Xlib UI plugin version 0.3.4",
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
initialize_screen(Movie *m, int w, int h)
{
  Xlib_info *info = (Xlib_info *)m->ui_private;
  int f;

  f = x11window_resize(info->xw, w, h);
  XFlush(x11_display(info->x11));

  return f;
}

static int
render_frame(Movie *m, Image *p)
{
  Xlib_info *info = (Xlib_info *)m->ui_private;
  X11 *x11 = info->x11;
  X11Window *xw = info->xw;
  XImage *ximage = info->ximage;
  Pixmap pix;
  GC gc;

  x11ximage_convert_image(ximage, p);

  pix = x11_create_pixmap(x11, x11window_win(xw), p->width, p->height, x11_depth(x11));
  gc = x11_create_gc(x11, pix, 0, 0);
  XPutImage(x11_display(x11), pix, gc, ximage, 0, 0, 0, 0, p->width, p->height);

  if (ximage->data != (char *)p->image)
    free(p->image);
  p->image = NULL;

  XCopyArea(x11_display(x11), pix, x11window_win(xw), gc, 0, 0, p->width, p->height, 0, 0);
  XFlush(x11_display(x11));

  return 1;
}

static int
play_movie(X11 *x11, X11Window *xw, Movie *m)
{
  XImage *ximage;
  Xlib_info *info = (Xlib_info *)m->ui_private;

  ximage = x11_create_ximage(x11, x11_visual(x11), x11_depth(x11), m->get_screen(m),
			     m->width, m->height, 8, 0);
  info->ximage = ximage;

  do {
    if (movie_play_main(m) != PLAY_OK) {
      show_message("play_movie: Error\n");
      return 0;
    }
  } while (m->status == _PLAY);

  x11_destroy_ximage(ximage);

  return 1;
}

static int
show_image(X11 *x11, X11Window *xw, Image *p)
{
  XImage *ximage;
  Pixmap pix;
  GC gc;
  int fd, ret, loop;
  int pressed;
  fd_set read_fds, write_fds, except_fds;
  struct timeval timeout;
  XEvent ev;

  ximage = x11_create_ximage(x11, x11_visual(x11), x11_depth(x11), NULL,
			     p->width, p->height, 8, 0);

  debug_message("type: %s", image_type_to_string(p->type));
  debug_message(" %s\n", p->alpha_enabled ? "with alpha enabled" : "without alpha");

  /*
   * convert image so that X can display it.
   * So far, alpha channel will be ignored.
   */
  x11ximage_convert_image(ximage, p);

  debug_message("converted type: %s\n", image_type_to_string(p->type));
  debug_message("x order %s\n", ximage->byte_order == MSBFirst ? "MSB" : "LSB");

  x11window_resize(xw, p->width, p->height);
  pix = x11_create_pixmap(x11, x11window_win(xw), p->width, p->height, x11_depth(x11));
  gc = x11_create_gc(x11, pix, 0, 0);
  XPutImage(x11_display(x11), pix, gc, ximage, 0, 0, 0, 0, p->width, p->height);

  if (ximage->data != (char *)p->image)
    free(p->image);
  free(ximage->data);
  ximage->data = NULL;
  x11_destroy_ximage(ximage);
  p->image = NULL;

  XCopyArea(x11_display(x11), pix, x11window_win(xw), gc, 0, 0, p->width, p->height, 0, 0);
  XSelectInput(x11_display(x11), x11window_win(xw), ExposureMask | ButtonPressMask | ButtonReleaseMask);
  XFlush(x11_display(x11));

  loop = 1;
  pressed = 0;
  fd = ConnectionNumber(x11_display(x11));
  while (loop) {
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&except_fds);
    FD_SET(fd, &read_fds);
    timeout.tv_sec = 0;
    timeout.tv_usec = 50;

    if ((ret = select(getdtablesize(), &read_fds, &write_fds, &except_fds, &timeout)) < 0) {
      if (errno == 4)
	continue;
      show_message("show_image: select returns %d: errno = %d\n", ret, errno);
      loop = 0;
      break;
    }
    while (XCheckMaskEvent(x11_display(x11), ExposureMask | ButtonPressMask | ButtonReleaseMask, &ev) == True) {
      switch (ev.type) {
      case Expose:
	{
	  XExposeEvent *xev = (XExposeEvent *)&ev;
          Region region = XCreateRegion();
          XRectangle rect;

          do {
            rect.x = xev->x;
            rect.y = xev->y;
            rect.width = xev->width;
            rect.height = xev->height;
            XUnionRectWithRegion(&rect, region, region);
          } while (XCheckWindowEvent(x11_display(x11), x11window_win(xw),
				     ExposureMask, (XEvent *)xev));

          XClipBox(region, &rect);
          XSetRegion(x11_display(x11), gc, region);

          XCopyArea(x11_display(x11), pix, x11window_win(xw), gc, rect.x, rect.y, rect.width, rect.height, rect.x, rect.y);

          XSync(x11_display(x11), False);
          XSetClipMask(x11_display(x11), gc, None);
          XDestroyRegion(region);
        }
        break;
      case ButtonPress:
	switch (ev.xbutton.button) {
	case Button1:
	  pressed = 1;
	  break;
	case Button2:
	  pressed = 2;
	  break;
	case Button3:
	  pressed = 3;
	  break;
	}
	break;
      case ButtonRelease:
	switch (ev.xbutton.button) {
	case Button1:
	  if (pressed == 1)
	    return 1;
	  break;
	case Button2:
	  if (pressed == 2)
	    return 0;
	  break;
	case Button3:
	  if (pressed == 3)
	    return -1;
	  break;
	}
	pressed = 0;
	break;
      }
    }
  }

  return 0;
}

static int
process_files_of_archive(UIData *uidata, X11 *x11, X11Window *xw, Archive *a)
{
  Loader *ld = uidata->ld;
  Streamer *st = uidata->st;
  Archiver *ar = uidata->ar;
  Player *player = uidata->player;
  Archive *arc;
  Stream *s;
  Image *p;
  Movie *m;
  Xlib_info *info;
  char *path;
  int f;
  int dir = 1;
  struct stat statbuf;

  s = stream_create();
  p = image_create();
  m = movie_create();
  
  m->initialize_screen = initialize_screen;
  m->render_frame = render_frame;
  if ((m->ui_private = calloc(1, sizeof(Xlib_info))) == NULL) {
    dir = 0;
  }

  info = (Xlib_info *)m->ui_private;
  info->x11 = x11;
  info->xw = xw;

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
	  dir = process_files_of_archive(uidata, x11, xw, arc);
	  archive_destroy(arc);
	  continue;
	} else if (!S_ISREG(statbuf.st_mode)) {
	  archive_iteration_delete(a);
	  continue;
	}

	if (streamer_identify(st, s, path)) {

	  debug_message("Stream identified as %s\n", s->format);

	  if (!streamer_open(st, s, s->format, path)) {
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
      if (archiver_identify(ar, arc, s)) {

	debug_message("Archiver identified as %s\n", a->format);

	if (archiver_open(ar, arc, arc->format, s)) {
	  dir = process_files_of_archive(uidata, x11, xw, arc);
	  archive_destroy(arc);
	  continue;
	} else {
	  show_message("Archive %s [%s] cannot open\n", a->format, path);
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
    if (loader_identify(ld, p, s)) {

      debug_message("Image identified as %s\n", p->format);

      if ((f = loader_load_image(ld, p->format, p, s)) == LOAD_OK)
	stream_close(s);
    } else {
      show_message("%s identification failed\n", path);
    }

    if (f != LOAD_OK) {
      if (player_identify(player, m, s)) {

	debug_message("Movie(Animation) identified as %s\n", p->format);

	if ((f = player_load_movie(player, m->format, m, s)) != PLAY_OK) {
	  stream_close(s);
	  show_message("%s play failed\n", path);
	  archive_iteration_delete(a);
	  continue;
	}
      } else {
	stream_close(s);
	show_message("%s identify failed\n", path);
	archive_iteration_delete(a);
	continue;
      }

      dir = play_movie(x11, xw, m);
      movie_unload(m);
    } else {

      debug_message("%s: (%d, %d)\n", path, p->width, p->height);

      if (p->comment) {
	show_message("comment:\n%s\n", p->comment);
	free(p->comment);
	p->comment = NULL;
      }

      dir = show_image(x11, xw, p);
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
  X11 *x11;
  X11Window *xw;

  x11 = x11_create();

  if (!x11_open(x11, NULL)) {
    show_message("x11_open() failed\n");
    return 0;
  }

  xw = x11window_create(x11, NULL, 600, 400);
  x11window_storename(xw, PROGNAME " version " VERSION);
  x11window_map(xw);

  process_files_of_archive(uidata, x11, xw, uidata->a);

  x11window_unmap(xw);
  x11window_destroy(xw);
  x11_destroy(x11);

  return 1;
}
