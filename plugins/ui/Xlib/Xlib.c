/*
 * Xlib.c -- Xlib UI plugin
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Oct 17 05:22:32 2000.
 * $Id: Xlib.c,v 1.10 2000/10/16 20:23:50 sian Exp $
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
  Pixmap pix;
  GC gc;
} Xlib_info;

static int ui_main(UIData *);

static UIPlugin plugin = {
  type: ENFLE_PLUGIN_UI,
  name: "Xlib",
  description: "Xlib UI plugin version 0.4.1",
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
initialize_screen(UIData *uidata, Movie *m, int w, int h)
{
  Xlib_info *info = (Xlib_info *)uidata->private;
  UIScreen *screen = uidata->screen;
  int f;

  screen->width = w;
  screen->height = h;

  /* CHECK */
  if (screen->depth == 0) {
    show_message("depth must be specified before initialize_screen().\n");
    exit(1);
  }

  f = x11window_resize(info->xw, w, h);

  if (m != NULL) {
    if (movie_get_screen(m) == NULL) {
      show_message("screen must be allocated before initialize_screen().\n");
      exit(1);
    }
    /* ximage is fixed size for movie */
    info->ximage = x11_create_ximage(info->x11, x11_visual(info->x11), screen->depth,
				     NULL, w, h, 8, 0);
  }

  /* Create pixmap for double buffering */
  info->pix = x11_create_pixmap(info->x11, x11window_win(info->xw),
				w, h, screen->depth);
  info->gc = x11_create_gc(info->x11, info->pix, 0, 0);
  XFlush(x11_display(info->x11));

  return f;
}

static void
update_screen(UIData *uidata, unsigned int left, unsigned int top, unsigned int w, unsigned int h)
{
  Xlib_info *info = (Xlib_info *)uidata->private;

  XCopyArea(x11_display(info->x11), info->pix, x11window_win(info->xw), info->gc,
	    left, top, w, h, left, top);
}

/* Must be passed whole screen in p. */
static int
render_frame(UIData *uidata, Movie *m, Image *p)
{
  Xlib_info *info = (Xlib_info *)uidata->private;
  XImage *ximage = info->ximage;

  /* CHECK */
  if (p->width != m->width || p->height != m->height) {
    show_message("render_frame: p(%d, %d) != m(%d, %d)\n", p->width, p->height, m->width, m->height);
    exit(1);
  }

  x11ximage_convert_image(ximage, p);

  XPutImage(x11_display(info->x11), info->pix, info->gc, ximage,
	    0, 0, 0, 0, p->width, p->height);

  if (ximage->data != (char *)p->image)
    free(ximage->data);
  ximage->data = NULL;

  update_screen(uidata, p->left, p->top, p->width, p->height);

  XFlush(x11_display(info->x11));

  return 1;
}

static int
render_image(UIData *uidata, Image *p)
{
  Xlib_info *info = (Xlib_info *)uidata->private;
  UIScreen *screen = uidata->screen;
  XImage *ximage;

  ximage = x11_create_ximage(info->x11, x11_visual(info->x11), screen->depth, NULL,
			     p->width, p->height, 8, 0);

  debug_message("type: %s", image_type_to_string(p->type));
  debug_message(" %s\n", p->alpha_enabled ? "with alpha enabled" : "without alpha");

  /* convert image so that X can display it. So far, alpha will be ignored. */
  x11ximage_convert_image(ximage, p);

  debug_message("x order %s\n", ximage->byte_order == MSBFirst ? "MSB" : "LSB");

  XPutImage(x11_display(info->x11), info->pix, info->gc, ximage, 0, 0, 0, 0, p->width, p->height);

  if (ximage->data != (char *)p->image)
    free(p->image);
  p->image = NULL;
  free(ximage->data);
  ximage->data = NULL;
  x11_destroy_ximage(ximage);

  update_screen(uidata, p->left, p->top, p->width, p->height);

  return 1;
}

static int
main_loop(UIData *uidata, Movie *m, Image *p)
{
  Xlib_info *info = (Xlib_info *)uidata->private;
  X11 *x11 = info->x11;
  X11Window *xw = info->xw;
  int fd, ret, loop;
  int pressed;
  fd_set read_fds, write_fds, except_fds;
  struct timeval timeout;
  XEvent ev;

  if (p) {
    initialize_screen(uidata, NULL, p->width, p->height);
    render_image(uidata, p);
  }

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
          XSetRegion(x11_display(x11), info->gc, region);

          update_screen(uidata, rect.x, rect.y, rect.width, rect.height);

          XSync(x11_display(x11), False);
          XSetClipMask(x11_display(x11), info->gc, None);
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

    if (m) {
      switch (m->status) {
      case _PLAY:
	if (movie_play_main(m, uidata) != PLAY_OK) {
	  show_message("play_movie: Error\n");
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

/* XXX: Such a routine as this should be out of ui plugin. */
static int
process_files_of_archive(UIData *uidata, Archive *a)
{
  Loader *ld = uidata->ld;
  Streamer *st = uidata->st;
  Archiver *ar = uidata->ar;
  Player *player = uidata->player;
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
	  dir = process_files_of_archive(uidata, arc);
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
    }

    if (f != LOAD_OK) {
      if (player_identify(player, m, s)) {

	debug_message("Movie(Animation) identified as %s\n", m->format);

	if ((f = player_load_movie(player, uidata, m->format, m, s)) != PLAY_OK) {
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

      dir = main_loop(uidata, m, NULL);
      movie_unload(m);
    } else {

      debug_message("%s: (%d, %d)\n", path, p->width, p->height);

      if (p->comment) {
	show_message("comment:\n%s\n", p->comment);
	free(p->comment);
	p->comment = NULL;
      }

      dir = main_loop(uidata, NULL, p);
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
  Xlib_info *info;

  x11 = x11_create();

  if (!x11_open(x11, NULL)) {
    show_message("x11_open() failed\n");
    return 0;
  }

  xw = x11window_create(x11, NULL, 600, 400);
  x11window_storename(xw, PROGNAME " version " VERSION);
  x11window_map(xw);

  if ((uidata->screen = (UIScreen *)calloc(1, sizeof(UIScreen))) == NULL) {
    show_message("ui_main: No enough memory\n");
    return 0;
  }
  uidata->screen->depth = x11_depth(x11);
  uidata->screen->bits_per_pixel = x11_bpp(x11);

  if ((uidata->private = (void *)calloc(1, sizeof(Xlib_info))) == NULL) {
    show_message("ui_main: No enough memory\n");
    return 0;
  }
  info = (Xlib_info *)uidata->private;
  info->x11 = x11;
  info->xw = xw;

  process_files_of_archive(uidata, uidata->a);

  if (info->ximage)
    x11_destroy_ximage(info->ximage);
  if (info->pix)
    x11_free_pixmap(x11, info->pix);
  if (info->gc)
    x11_free_gc(x11, info->gc);

  free(info);

  x11window_unmap(xw);
  x11window_destroy(xw);
  x11_destroy(x11);

  return 1;
}
