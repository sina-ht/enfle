/*
 * Xlib.c -- Xlib Video plugin
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Fri Nov  3 04:25:41 2000.
 * $Id: Xlib.c,v 1.2 2000/11/02 19:34:09 sian Exp $
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
#include <errno.h>
#include <sys/time.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#define REQUIRE_STRING_H
#define REQUIRE_UNISTD_H
#include "compat.h"

#include "common.h"

#include "image.h"
#include "x11.h"
#include "x11window.h"
#include "x11ximage.h"

#include "video-plugin.h"

typedef struct {
  X11 *x11;
} Xlib_info;

typedef struct {
  X11Window *xw;
  XImage *ximage;
  Pixmap pix;
  GC gc;
} X11Window_info;

static void *open_video(void *);
static int close_video(void *);
static VideoWindow *open_window(void *, unsigned int, unsigned int);
static int set_event_mask(VideoWindow *, int);
static int dispatch_event(VideoWindow *, VideoEventData *);
static void set_window_caption(VideoWindow *, unsigned char *);
static int destroy_window(VideoWindow *);
static int resize_window(VideoWindow *, unsigned int, unsigned int);
static int move_window(VideoWindow *, unsigned int, unsigned int);
static int render_frame(VideoWindow *, Image *);
static void update_screen(VideoWindow *, unsigned int, unsigned int, unsigned int, unsigned int);
static void destroy(void *);

static VideoPlugin plugin = {
  type: ENFLE_PLUGIN_VIDEO,
  name: "Xlib",
  description: "Xlib Video plugin version 0.1",
  author: "Hiroshi Takekawa",

  open_video: open_video,
  close_video: close_video,
  open_window: open_window,
  set_event_mask: set_event_mask,
  dispatch_event: dispatch_event,
  set_window_caption: set_window_caption,
  destroy_window: destroy_window,
  resize_window: resize_window,
  move_window: move_window,
  render_frame: render_frame,
  update_screen: update_screen,
  destroy: destroy
};

void *
plugin_entry(void)
{
  VideoPlugin *vp;

  if ((vp = (VideoPlugin *)calloc(1, sizeof(VideoPlugin))) == NULL)
    return NULL;
  memcpy(vp, &plugin, sizeof(VideoPlugin));

  return (void *)vp;
}

void
plugin_exit(void *p)
{
  free(p);
}

/* for internal use */

static void
free_window_resource(VideoWindow *vw)
{
  X11Window_info *xwi = (X11Window_info *)vw->private;
  X11Window *xw = xwi->xw;
  X11 *x11 = x11window_x11(xw);

  if (xwi->ximage)
    x11_destroy_ximage(xwi->ximage);
  if (xwi->pix)
    x11_free_pixmap(x11, xwi->pix);
  if (xwi->gc)
    x11_free_gc(x11, xwi->gc);
}

static void
set_window_size(VideoWindow *vw, unsigned int w, unsigned int h)
{
  X11Window_info *xwi = (X11Window_info *)vw->private;
  X11Window *xw = xwi->xw;
  X11 *x11 = x11window_x11(xw);

  free_window_resource(vw);

  xwi->ximage = x11_create_ximage(x11, x11_visual(x11), x11_depth(x11), NULL, w, h, 8, 0);
  xwi->pix = x11_create_pixmap(x11, x11window_win(xw), w, h, x11_depth(x11));
  xwi->gc = x11_create_gc(x11, xwi->pix, 0, 0);

  vw->width = w;
  vw->height = h;
}

/* methods */

static void *
open_video(void *data)
{
  Xlib_info *p;
  char *dsp = (char *)data;

  if ((p = calloc(1, sizeof(Xlib_info))) == NULL)
    return NULL;
  if ((p->x11 = x11_create()) == NULL)
    goto error_x11_create;

  if (!x11_open(p->x11, dsp))
    goto error_x11_open;

  return p;

 error_x11_open:
  x11_destroy(p->x11);
 error_x11_create:
  free(p);
  return NULL;
}

static int
close_video(void *data)
{
  Xlib_info *p = (Xlib_info *)data;
  int f;

  f = x11_close(p->x11);
  free(p);

  return f;
}

static VideoWindow *
open_window(void *data, unsigned int w, unsigned int h)
{
  Xlib_info *p = (Xlib_info *)data;
  VideoWindow *vw;
  X11Window *xw;
  X11Window_info *xwi;

  if ((vw = calloc(1, sizeof(VideoWindow))) == NULL)
    return NULL;
  if ((vw->private = calloc(1, sizeof(X11Window_info))) == NULL) {
    free(vw);
    return NULL;
  }
  xwi = (X11Window_info *)vw->private;

  xwi->xw = xw = x11window_create(p->x11, NULL, w, h);
  vw->depth = x11_depth(p->x11);
  vw->bits_per_pixel = x11_bpp(p->x11);
  set_window_size(vw, w, h);

  x11window_get_position(xw, &vw->x, &vw->y);
  x11window_map(xw);

  return vw;
}

static int
set_event_mask(VideoWindow *vw, int mask)
{
  X11Window_info *xwi = (X11Window_info *)vw->private;
  X11Window *xw = xwi->xw;
  int x_mask = 0;

  if (mask & ENFLE_ExposureMask) 
    x_mask |= ExposureMask;
  if (mask & ENFLE_ButtonMask)
    x_mask |= (ButtonPressMask | ButtonReleaseMask);
  if (mask & ENFLE_KeyMask)
    x_mask |= KeyPressMask;
  if (mask & ENFLE_PointerMask)
    x_mask |= ButtonMotionMask;
  if (mask & ENFLE_WindowMask)
    x_mask |= (StructureNotifyMask | VisibilityChangeMask);

  x11window_set_event_mask(xw, x_mask);

  return 1;
}

#define TRANSLATE(k) case XK_##k: ev->key.key = ENFLE_KEY_##k; break

static int
dispatch_event(VideoWindow *vw, VideoEventData *ev)
{
  X11Window_info *xwi = (X11Window_info *)vw->private;
  X11Window *xw = xwi->xw;
  X11 *x11 = x11window_x11(xw);
  int fd, ret;
  fd_set read_fds, write_fds, except_fds;
  struct timeval timeout;
  XEvent xev;

  fd = ConnectionNumber(x11_display(x11));
  FD_ZERO(&read_fds);
  FD_ZERO(&write_fds);
  FD_ZERO(&except_fds);
  FD_SET(fd, &read_fds);
  timeout.tv_sec = 0;
  timeout.tv_usec = 50;

  if ((ret = select(getdtablesize(), &read_fds, &write_fds, &except_fds, &timeout)) < 0) {
    if (errno == 4)
      return 0;
    show_message("Xlib: get_event: select returns %d: errno = %d\n", ret, errno);
    return 0;
  }

  if (XCheckMaskEvent(x11_display(x11), x11window_mask(xw), &xev) == True) {
    switch (xev.type) {
    case Expose:
      {
	XExposeEvent *xeev = (XExposeEvent *)&xev;
	Region region = XCreateRegion();
	XRectangle rect;

	ev->type = ENFLE_Event_Exposure;
	do {
	  rect.x = xeev->x;
	  rect.y = xeev->y;
	  rect.width = xeev->width;
	  rect.height = xeev->height;
	  XUnionRectWithRegion(&rect, region, region);
	} while (XCheckWindowEvent(x11_display(x11), x11window_win(xw),
				   ExposureMask, &xev));

	XClipBox(region, &rect);
	XSetRegion(x11_display(x11), xwi->gc, region);

	update_screen(vw, rect.x, rect.y, rect.width, rect.height);

	XSync(x11_display(x11), False);
	XSetClipMask(x11_display(x11), xwi->gc, None);
	XDestroyRegion(region);
      }
      return 0;
    case ButtonPress:
      ev->type = ENFLE_Event_ButtonPressed;
      switch (xev.xbutton.button) {
      case Button1:
	ev->button.button = ENFLE_Button_1;
	break;
      case Button2:
	ev->button.button = ENFLE_Button_2;
	break;
      case Button3:
	ev->button.button = ENFLE_Button_3;
	break;
      }
      return 1;
    case ButtonRelease:
      ev->type = ENFLE_Event_ButtonReleased;
      switch (xev.xbutton.button) {
      case Button1:
	ev->button.button = ENFLE_Button_1;
	break;
      case Button2:
	ev->button.button = ENFLE_Button_2;
	break;
      case Button3:
	ev->button.button = ENFLE_Button_3;
	break;
      }
      return 1;
    case KeyPress:
      {
	char c;
	KeySym keysym;

	ev->type = ENFLE_Event_KeyPressed;
	XLookupString((XKeyEvent *)&xev, &c, 1, &keysym, NULL);
	switch (keysym) {
	TRANSLATE(0); TRANSLATE(1); TRANSLATE(2); TRANSLATE(3); TRANSLATE(4);
	TRANSLATE(5); TRANSLATE(6); TRANSLATE(7); TRANSLATE(8); TRANSLATE(9);
	TRANSLATE(BackSpace); TRANSLATE(Tab); TRANSLATE(Return); TRANSLATE(Escape);
	TRANSLATE(Delete); TRANSLATE(Left); TRANSLATE(Up); TRANSLATE(Right); TRANSLATE(Down); 
	TRANSLATE(Insert);
	TRANSLATE(space); TRANSLATE(colon); TRANSLATE(semicolon);
	TRANSLATE(less); TRANSLATE(equal); TRANSLATE(greater); TRANSLATE(question); TRANSLATE(at); 
	TRANSLATE(a); TRANSLATE(b); TRANSLATE(c); TRANSLATE(d); TRANSLATE(e); TRANSLATE(f);
	TRANSLATE(g); TRANSLATE(h); TRANSLATE(i); TRANSLATE(j); TRANSLATE(k);
	TRANSLATE(l); TRANSLATE(m); TRANSLATE(n); TRANSLATE(o); TRANSLATE(p);
	TRANSLATE(q); TRANSLATE(r); TRANSLATE(s); TRANSLATE(t); TRANSLATE(u);
	TRANSLATE(v); TRANSLATE(w); TRANSLATE(x); TRANSLATE(y); TRANSLATE(z);
	default:
	  ev->key.key = ENFLE_KEY_Unknown;
	  break;
	}
      }
      return 1;
    case MotionNotify:
      ev->type = ENFLE_Event_PointerMoved;
      if (xev.xmotion.state & Button1Mask)
	ev->pointer.button = ENFLE_Button_1;
      if (xev.xmotion.state & Button2Mask)
	ev->pointer.button = ENFLE_Button_2;
      if (xev.xmotion.state & Button3Mask)
	ev->pointer.button = ENFLE_Button_3;
      ev->pointer.x = xev.xmotion.x;
      ev->pointer.y = xev.xmotion.y;
      return 1;
    case ConfigureNotify:
      {
	XConfigureEvent *xcev = (XConfigureEvent *)&ev;
	ev->type = ENFLE_Event_WindowConfigured;
	ev->window.width = xcev->width;
	ev->window.height = xcev->height;
      }
      return 1;
    case VisibilityNotify:
      break;
    default:
      break;
    }
  }

  return 0;
}

static void
set_window_caption(VideoWindow *vw, unsigned char *cap)
{
  X11Window_info *xwi = (X11Window_info *)vw->private;

  x11window_storename(xwi->xw, cap);
}

static int
destroy_window(VideoWindow *vw)
{
  X11Window_info *xwi = (X11Window_info *)vw->private;
  X11Window *xw = (X11Window *)xwi->xw;

  free_window_resource(vw);
  x11window_unmap(xw);
  x11window_destroy(xw);

  free(xwi);
  free(vw);

  return 1;
}

static int
resize_window(VideoWindow *vw, unsigned int w, unsigned int h)
{
  X11Window_info *xwi = (X11Window_info *)vw->private;
  X11Window *xw = (X11Window *)xwi->xw;

  x11window_resize(xw, w, h);
  set_window_size(vw, w, h);

  return 1;
}

static int
move_window(VideoWindow *vw, unsigned int x, unsigned int y)
{
  x11window_move((X11Window *)vw->private, x, y);
  x11window_get_position((X11Window *)vw->private, &vw->x, &vw->y);

  return 1;
}

static void
update_screen(VideoWindow *vw, unsigned int left, unsigned int top, unsigned int w, unsigned int h)
{
  X11Window_info *xwi = (X11Window_info *)vw->private;
  X11Window *xw = xwi->xw;
  X11 *x11 = x11window_x11(xw);

  XCopyArea(x11_display(x11), xwi->pix, x11window_win(xw), xwi->gc,
	    left, top, w, h, left, top);
}

static int
render_frame(VideoWindow *vw, Image *p)
{
  X11Window_info *xwi = (X11Window_info *)vw->private;
  X11Window *xw = xwi->xw;
  X11 *x11 = x11window_x11(xw);

  x11ximage_convert_image(xwi->ximage, p);
  XPutImage(x11_display(x11), xwi->pix, xwi->gc, xwi->ximage, 0, 0, 0, 0, p->width, p->height);

  if (xwi->ximage->data != (char *)p->image)
    free(xwi->ximage->data);
  xwi->ximage->data = NULL;

  update_screen(vw, p->left, p->top, p->width, p->height);

  return 1;
}

static void
destroy(void *p)
{
  close_video(p);
  free(p);
}
