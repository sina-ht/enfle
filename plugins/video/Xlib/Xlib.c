/*
 * Xlib.c -- Xlib Video plugin
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Dec  3 17:10:07 2000.
 * $Id: Xlib.c,v 1.7 2000/12/03 08:40:04 sian Exp $
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
  Pixmap pix;
  GC gc;
} WindowResource;

typedef struct {
  X11XImage *xi;
  WindowResource current;
  WindowResource normal;
  WindowResource full;
  unsigned int full_width, full_height;
  int share_image;
} X11Window_info;

static void *open_video(void *);
static int close_video(void *);
static VideoWindow *open_window(void *, unsigned int, unsigned int);
static MemoryType preferred_memory_type(VideoWindow *);
static int set_event_mask(VideoWindow *, int);
static int dispatch_event(VideoWindow *, VideoEventData *);
static void set_caption(VideoWindow *, unsigned char *);
static int set_fullscreen_mode(VideoWindow *, VideoWindowFullscreenMode);
static int destroy_window(VideoWindow *);
static int resize(VideoWindow *, unsigned int, unsigned int);
static int move(VideoWindow *, unsigned int, unsigned int);
static int render(VideoWindow *, Image *);
static void update(VideoWindow *, unsigned int, unsigned int, unsigned int, unsigned int);
static void destroy(void *);

static VideoPlugin plugin = {
  type: ENFLE_PLUGIN_VIDEO,
  name: "Xlib",
  description: "Xlib Video plugin version 0.2",
  author: "Hiroshi Takekawa",

  open_video: open_video,
  close_video: close_video,
  open_window: open_window,
  destroy: destroy
};

static VideoWindow template = {
  preferred_memory_type: preferred_memory_type,
  set_event_mask: set_event_mask,
  dispatch_event: dispatch_event,
  set_caption: set_caption,
  set_fullscreen_mode: set_fullscreen_mode,
  destroy: destroy_window,
  resize: resize,
  move: move,
  render: render,
  update: update
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
create_window_doublebuffer(VideoWindow *vw, unsigned int w, unsigned int h)
{
  X11Window_info *xwi = (X11Window_info *)vw->private;
  X11Window *xw = xwi->current.xw;
  X11 *x11 = x11window_x11(xw);

  if (xwi->current.pix)
    x11_free_pixmap(x11, xwi->current.pix);
  if (xwi->current.gc)
    x11_free_gc(x11, xwi->current.gc);

  xwi->current.pix = x11_create_pixmap(x11, x11window_win(xw), w, h, x11_depth(x11));
  xwi->current.gc = x11_create_gc(x11, xwi->current.pix, 0, 0);
  XSetForeground(x11_display(x11), xwi->current.gc, x11_black(x11));
}

static void
create_window_resource(VideoWindow *vw, unsigned int w, unsigned int h)
{
  vw->width = w;
  vw->height = h;
  create_window_doublebuffer(vw, w, h);
}

/* video plugin methods */

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

static void
destroy(void *p)
{
  close_video(p);
  free(p);
}

static VideoWindow *
open_window(void *data, unsigned int w, unsigned int h)
{
  Xlib_info *p = (Xlib_info *)data;
  VideoWindow *vw;
  X11Window *xw;
  X11Window_info *xwi;
  X11 *x11 = p->x11;

  if ((vw = calloc(1, sizeof(VideoWindow))) == NULL)
    return NULL;
  memcpy(vw, &template, sizeof(VideoWindow));

  if ((vw->private = calloc(1, sizeof(X11Window_info))) == NULL) {
    free(vw);
    return NULL;
  }
  xwi = (X11Window_info *)vw->private;

  xwi->current.xw = xwi->normal.xw = xw = x11window_create(x11, NULL, w, h);
  xwi->xi = x11ximage_create(x11);

  vw->depth = x11_depth(x11);
  vw->bits_per_pixel = x11_bpp(x11);
  create_window_resource(vw, w, h);
  xwi->normal.pix = xwi->current.pix;
  xwi->normal.gc  = xwi->current.gc;

  x11window_get_position(xw, &vw->x, &vw->y);
  x11window_map(xw);

  return vw;
}

/* video window methods */

static MemoryType
preferred_memory_type(VideoWindow *vw)
{
  X11Window_info *xwi = (X11Window_info *)vw->private;
  X11Window *xw = xwi->current.xw;
  X11 *x11 = x11window_x11(xw);

  return x11_if_shm(x11) ? _SHM : _NORMAL;
}

static int
set_event_mask(VideoWindow *vw, int mask)
{
  X11Window_info *xwi = (X11Window_info *)vw->private;
  X11Window *xw = xwi->current.xw;
  int x_mask = 0;

  if (mask & ENFLE_ExposureMask) 
    x_mask |= ExposureMask;
  if (mask & ENFLE_ButtonMask)
    x_mask |= (ButtonPressMask | ButtonReleaseMask);
  if (mask & ENFLE_KeyMask)
    x_mask |= (KeyPressMask | KeyReleaseMask);
  if (mask & ENFLE_PointerMask)
    x_mask |= ButtonMotionMask;
  if (mask & ENFLE_WindowMask)
    x_mask |= VisibilityChangeMask;

  /* needs for wait_mapped() */
  x_mask |= StructureNotifyMask;

  x11window_set_event_mask(xw, x_mask);

  return 1;
}

#define TRANSLATE(k) case XK_##k: ev->key.key = ENFLE_KEY_##k; break

static int
dispatch_event(VideoWindow *vw, VideoEventData *ev)
{
  X11Window_info *xwi = (X11Window_info *)vw->private;
  X11Window *xw = xwi->current.xw;
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
	XSetRegion(x11_display(x11), xwi->current.gc, region);

	update(vw, rect.x, rect.y, rect.width, rect.height);

	XSync(x11_display(x11), False);
	XSetClipMask(x11_display(x11), xwi->current.gc, None);
	XDestroyRegion(region);
      }
      return 0;
    case ButtonPress:
    case ButtonRelease:
      ev->type = (xev.type == ButtonPress) ?
	ENFLE_Event_ButtonPressed : ENFLE_Event_ButtonReleased;
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
    case KeyRelease:
      {
	char c;
	KeySym keysym;

	ev->type = (xev.type == KeyPress) ?
	  ENFLE_Event_KeyPressed : ENFLE_Event_KeyReleased;
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
set_caption(VideoWindow *vw, unsigned char *cap)
{
  X11Window_info *xwi = (X11Window_info *)vw->private;

  x11window_storename(xwi->current.xw, cap);
}

static int
set_fullscreen_mode(VideoWindow *vw, VideoWindowFullscreenMode mode)
{
  int state_changed = 0;
  X11Window_info *xwi = (X11Window_info *)vw->private;
  X11Window *xw = xwi->current.xw;
  X11 *x11 = x11window_x11(xw);

  switch (mode) {
  case _VIDEO_WINDOW_FULLSCREEN_ON:
    if (vw->if_fullscreen == 0)
      state_changed = 1;
    vw->if_fullscreen = 1;
    break;
  case _VIDEO_WINDOW_FULLSCREEN_OFF:
    if (vw->if_fullscreen == 1)
      state_changed = 1;
    vw->if_fullscreen = 0;
    break;
  case _VIDEO_WINDOW_FULLSCREEN_TOGGLE:
    vw->if_fullscreen = 1 - vw->if_fullscreen;
    state_changed = 1;
    break;
  default:
    show_message("Xlib: set_fullscreen_mode: invalid fullscreen mode(%d).\n", mode);
    return 0;
  }

  if (!state_changed)
    return 1;

  debug_message("fullscreen_mode %d\n", vw->if_fullscreen);

  x11window_get_position(xw, &vw->x ,&vw->y);
  x11window_unmap(xw);

  if (vw->if_fullscreen == 0) {
    xwi->current.xw = xwi->normal.xw;
    xwi->current.pix = xwi->normal.pix;
    xwi->current.gc = xwi->normal.gc;

    x11window_resize(xwi->current.xw, vw->width, vw->height);
    create_window_doublebuffer(vw, vw->width, vw->height);
    xwi->normal.pix = xwi->current.pix;
    xwi->normal.gc  = xwi->current.gc;
    x11ximage_put(xwi->xi, xwi->current.pix, xwi->current.gc, 0, 0, 0, 0, vw->width, vw->height);
  } else {
    unsigned int full_w, full_h;

    full_w = WidthOfScreen(x11_sc(x11));
    full_h = HeightOfScreen(x11_sc(x11));

    if (xwi->full.xw == NULL) {
      XSetWindowAttributes set_attr;

      xwi->full.xw = x11window_create(x11, NULL, full_w, full_h);
      xwi->full_width = full_w;
      xwi->full_height = full_h;
      x11window_set_event_mask(xwi->full.xw, ExposureMask | ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask);
      xwi->current.pix = 0;
      xwi->current.gc = NULL;
      create_window_doublebuffer(vw, full_w, full_h);
      xwi->full.pix = xwi->current.pix;
      xwi->full.gc = xwi->current.gc;
      set_attr.override_redirect = True;
      XChangeWindowAttributes(x11_display(x11), x11window_win(xwi->full.xw), CWOverrideRedirect, &set_attr);
    }
    xwi->current.xw = xwi->full.xw;
    xwi->current.pix = xwi->full.pix;
    xwi->current.gc = xwi->full.gc;
    XFillRectangle(x11_display(x11), xwi->current.pix, xwi->current.gc, 0, 0, full_w, full_h);
    x11ximage_put(xwi->xi, xwi->current.pix, xwi->current.gc, 0, 0,
		  (full_w - vw->width) >> 1, (full_h - vw->height) >> 1, vw->width, vw->height);
  }

  x11window_map_raised(xwi->current.xw);
  x11window_wait_mapped(xwi->current.xw);
  XSetInputFocus(x11_display(x11), x11window_win(xwi->current.xw), RevertToPointerRoot, CurrentTime);

  return 1;
}

static int
resize(VideoWindow *vw, unsigned int w, unsigned int h)
{
  X11Window_info *xwi = (X11Window_info *)vw->private;
  X11Window *xw = (X11Window *)xwi->current.xw;
  X11 *x11 = x11window_x11(xw);

  if (!vw->if_fullscreen) {
    if (vw->width != w || vw->height != h) {
      x11window_resize(xw, w, h);
      create_window_doublebuffer(vw, w, h);
      xwi->normal.pix = xwi->current.pix;
      xwi->normal.gc = xwi->current.gc;
    }
  } else {
    if (vw->width > w || vw->height > h) {
      XFillRectangle(x11_display(x11), xwi->current.pix,  xwi->current.gc, 0, 0,
		     xwi->full_width, xwi->full_height);
      XFillRectangle(x11_display(x11), x11window_win(xw), xwi->current.gc, 0, 0,
		     xwi->full_width, xwi->full_height);
    }
  }

  vw->width = w;
  vw->height = h;

  return 1;
}

static int
move(VideoWindow *vw, unsigned int x, unsigned int y)
{
  if (!vw->if_fullscreen) {
    x11window_move((X11Window *)vw->private, x, y);
    x11window_get_position((X11Window *)vw->private, &vw->x, &vw->y);
  }

  return 1;
}

static void
update(VideoWindow *vw, unsigned int left, unsigned int top, unsigned int w, unsigned int h)
{
  X11Window_info *xwi = (X11Window_info *)vw->private;
  X11Window *xw = xwi->current.xw;
  X11 *x11 = x11window_x11(xw);

  XCopyArea(x11_display(x11), xwi->current.pix, x11window_win(xw), xwi->current.gc,
	    left, top, w, h, left, top);
}

static int
render(VideoWindow *vw, Image *p)
{
  X11Window_info *xwi = (X11Window_info *)vw->private;
  X11Window *xw = xwi->current.xw;

  x11ximage_convert(xwi->xi, p);

  if (vw->if_fullscreen) {
    if (vw->if_direct) {
      x11ximage_put(xwi->xi, x11window_win(xw), xwi->current.gc, 0, 0,
		    (xwi->full_width - p->width) >> 1, (xwi->full_height - p->height) >> 1,
		    p->width, p->height);
    } else {
      x11ximage_put(xwi->xi, xwi->current.pix, xwi->current.gc, 0, 0,
		    (xwi->full_width - p->width) >> 1, (xwi->full_height - p->height) >> 1,
		    p->width, p->height);
      update(vw, p->left + ((xwi->full_width - p->width) >> 1), p->top + ((xwi->full_height - p->height) >> 1), p->width, p->height);
    }
  } else {
    if (vw->if_direct) {
      x11ximage_put(xwi->xi, x11window_win(xw), xwi->current.gc, 0, 0, 0, 0, p->width, p->height);
    } else {
      x11ximage_put(xwi->xi, xwi->current.pix, xwi->current.gc, 0, 0, 0, 0, p->width, p->height);
      update(vw, p->left, p->top, p->width, p->height);
    }
  }

  return 1;
}

static int
destroy_window(VideoWindow *vw)
{
  X11Window_info *xwi = (X11Window_info *)vw->private;
  X11Window *xw = (X11Window *)xwi->current.xw;
  X11 *x11 = x11window_x11(xw);

  if (xwi->xi)
    x11ximage_destroy(xwi->xi);
  if (xwi->current.pix)
    x11_free_pixmap(x11, xwi->current.pix);
  if (xwi->current.gc)
    x11_free_gc(x11, xwi->current.gc);

  x11window_unmap(xw);
  x11window_destroy(xw);

  free(xwi);
  free(vw);

  return 1;
}
