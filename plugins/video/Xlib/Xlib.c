/*
 * Xlib.c -- Xlib Video plugin
 * (C)Copyright 2000-2007 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Thu Feb 26 21:26:24 2009.
 * $Id: Xlib.c,v 1.68 2009/02/23 14:31:02 sian Exp $
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
#include <locale.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>

#define REQUIRE_STRING_H
#define REQUIRE_UNISTD_H
#include "compat.h"
#include "common.h"

/* These are NOT official X11 headers, but enfle's. */
#include "X11/x11.h"
#include "X11/x11window.h"
#include "X11/x11ximage.h"

#include "enfle/video-plugin.h"
#include "enfle/effect-plugin.h"
#include "enfle/enfle-plugins.h"

/* XXX */
#define WINDOW_CAPTION_WIDTH  0
#define WINDOW_CAPTION_HEIGHT 30

#define WAIT_CURSOR XC_watch
#define NORMAL_CURSOR XC_arrow

static Cursor normal_cursor, wait_cursor, invisible_cursor;
static const char invisible_cursor_data[] = {
  0, 0, 0, 0, 0, 0, 0, 0
};

#include "Xlib.h"

typedef struct {
  X11Window *xw;
  Pixmap pix;
  unsigned int pix_width, pix_height;
  GC gc;
} WindowResource;

typedef struct {
  X11XImage *xi;
  WindowResource normal;
  WindowResource full;
  Font caption_font;
  XFontSet xfontset;
  int fontset;
  int is_rect_draw;
  unsigned int lx, uy, rx, dy;
} X11Window_info;

static void *open_video(void *, Config *);
static int close_video(void *);
static VideoWindow *get_root(void *);
static VideoWindow *open_window(void *, VideoWindow *, unsigned int, unsigned int);
static void set_wallpaper(void *, Image *);

static ImageType request_type(VideoWindow *, unsigned int, unsigned int, unsigned int, int *);
static int calc_magnified_size(VideoWindow *, int, unsigned int, unsigned int, int *, int *);
static MemoryType preferred_memory_type(VideoWindow *);
static int set_event_mask(VideoWindow *, int);
static int dispatch_event(VideoWindow *, VideoEventData *);
static void set_caption(VideoWindow *, char *);
static void set_cursor(VideoWindow *, VideoWindowCursor);
static void set_background(VideoWindow *, Image *);
static int set_fullscreen_mode(VideoWindow *, VideoWindowFullscreenMode);
static int destroy_window(VideoWindow *);
static int resize(VideoWindow *, unsigned int, unsigned int);
static int move(VideoWindow *, unsigned int, unsigned int);
static int get_offset(VideoWindow *, int *, int *);
static int set_offset(VideoWindow *, int, int);
static int adjust_offset(VideoWindow *, int, int);
static void erase_rect(VideoWindow *);
static void draw_rect(VideoWindow *, unsigned int, unsigned int, unsigned int, unsigned int);
static int render(VideoWindow *, Image *);
static int render_scaled(VideoWindow *, Image *, int, unsigned int, unsigned int);
static void update(VideoWindow *, unsigned int, unsigned int);
static void do_sync(VideoWindow *);
static void discard_key_event(VideoWindow *);
static void discard_button_event(VideoWindow *);
static void destroy(void *);

static void draw_rect_xor(VideoWindow *, unsigned int, unsigned int, unsigned int, unsigned int);
static void __update(VideoWindow *vw, unsigned int left, unsigned int top, unsigned int w, unsigned int h);

/* internal */

static VideoPlugin plugin = {
  .type = ENFLE_PLUGIN_VIDEO,
  .name = "Xlib",
  .description = "Xlib Video plugin version 0.6.1",
  .author = "Hiroshi Takekawa",

  .open_video = open_video,
  .close_video = close_video,
  .get_root = get_root,
  .open_window = open_window,
  .set_wallpaper = set_wallpaper,
  .destroy = destroy
};

static VideoWindow template = {
  .preferred_memory_type = preferred_memory_type,
  .request_type = request_type,
  .calc_magnified_size = calc_magnified_size,
  .set_event_mask = set_event_mask,
  .dispatch_event = dispatch_event,
  .set_caption = set_caption,
  .set_cursor = set_cursor,
  .set_background = set_background,
  .set_fullscreen_mode = set_fullscreen_mode,
  .destroy = destroy_window,
  .resize = resize,
  .move = move,
  .get_offset = get_offset,
  .set_offset = set_offset,
  .adjust_offset = adjust_offset,
  .erase_rect = erase_rect,
  .draw_rect = draw_rect,
  .render = render,
  .render_scaled = render_scaled,
  .update = update,
  .do_sync = do_sync,
  .discard_key_event = discard_key_event,
  .discard_button_event = discard_button_event
};

ENFLE_PLUGIN_ENTRY(video_Xlib)
{
  VideoPlugin *vp;
  char *tmp;

  if ((vp = (VideoPlugin *)calloc(1, sizeof(VideoPlugin))) == NULL)
    return NULL;
  memcpy(vp, &plugin, sizeof(VideoPlugin));
#ifdef USE_PTHREAD
  if (!XInitThreads())
    err_message("XInitThreads() failed\n");
  else
    debug_message("XInitThreads() OK\n");
#endif

  if ((tmp = setlocale(LC_ALL, getenv("LANG"))) == NULL) {
    warning("setlocale() failed.\n");
  }
  debug_message("Locale: %s\n", tmp);
  if (XSupportsLocale() == False) {
    warning("Xlib: XSupportsLocale() failed.\n");
  } else {
    if ((tmp = XSetLocaleModifiers("")) == NULL) {
      warning("Xlib: XSetLocaleModifers() failed.\n");
    } else {
      debug_message("XSetLocaleModifiers: %s\n", tmp);
    }
  }

  return (void *)vp;
}

ENFLE_PLUGIN_EXIT(video_Xlib, p)
{
  free(p);
}

/* video plugin methods */

static void *
open_video(void *data, Config *c)
{
  Xlib_info *p;
  char *dsp = (char *)data;
  Pixmap invisible_pixmap;
  XColor black, dummy;

  if ((p = calloc(1, sizeof(Xlib_info))) == NULL)
    return NULL;
  if ((p->x11 = x11_create()) == NULL)
    goto error_x11_create;

  if (!x11_open(p->x11, dsp))
    goto error_x11_open;

  wait_cursor = XCreateFontCursor(x11_display(p->x11), WAIT_CURSOR);
  normal_cursor = XCreateFontCursor(x11_display(p->x11), NORMAL_CURSOR);
  invisible_pixmap = XCreateBitmapFromData(x11_display(p->x11), x11_root(p->x11), invisible_cursor_data, 8, 8);
  XAllocNamedColor(x11_display(p->x11), DefaultColormapOfScreen(x11_sc(p->x11)), "black", &black, &dummy);
  invisible_cursor = XCreatePixmapCursor(x11_display(p->x11), invisible_pixmap, invisible_pixmap, &black, &black, 0, 0);
  XFreePixmap(x11_display(p->x11), invisible_pixmap);

  p->c = c;
  p->root = open_window(p, NULL, 0, 0);

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

  destroy_window(p->root);
  f = x11_close(p->x11);
  free(p);

  return f;
}

static void
destroy(void *data)
{
  (void)close_video(data);
}

static void
clip(VideoWindow *vw, unsigned int *w_return, unsigned int *h_return)
{
  unsigned int w = *w_return;
  unsigned int h = *h_return;

  if (!vw->if_fullscreen) {
    if (w > vw->full_width)
      w = vw->full_width;
    if (h > vw->full_height - WINDOW_CAPTION_HEIGHT)
      h = vw->full_height - WINDOW_CAPTION_HEIGHT;
  } else {
    if (w > vw->full_width)
      w = vw->full_width;
    if (h > vw->full_height)
      h = vw->full_height;
  }

  *w_return = w;
  *h_return = h;
}

static VideoWindow *
get_root(void *data)
{
  Xlib_info *p = (Xlib_info *)data;
  return p->root;
}

static VideoWindow *
open_window(void *data, VideoWindow *parent, unsigned int w, unsigned int h)
{
  Xlib_info *p = (Xlib_info *)data;
  VideoWindow *vw;
  X11Window *xw;
  X11Window_info *xwi;
  X11 *x11 = p->x11;
  char *fontname, *fontsetname;

  if ((vw = calloc(1, sizeof(VideoWindow))) == NULL)
    return NULL;
  memcpy(vw, &template, sizeof(VideoWindow));

  if ((vw->private_data = calloc(1, sizeof(X11Window_info))) == NULL) {
    free(vw);
    return NULL;
  }
  xwi = (X11Window_info *)vw->private_data;

  vw->c = p->c;
  if ((fontname = config_get(vw->c, "/enfle/plugins/video/caption_font")) == NULL) {
    fontname = (char *)"a14";
  }
  if ((fontsetname = config_get(vw->c, "/enfle/plugins/video/caption_fontset")) == NULL) {
    fontsetname = NULL;
  }

  if (fontsetname) {
    debug_message_fnc("load fontset [%s]\n", fontsetname);
  } else {
    debug_message_fnc("load font [%s]\n", fontname);
  }

  xwi->fontset = 0;
  {
    char **miss = NULL, *def = NULL;
    int misscount = 0;

    if (fontsetname) {
      xwi->xfontset = XCreateFontSet(x11_display(x11), fontsetname, &miss, &misscount, &def);
      if (xwi->xfontset) {
	if (misscount > 0) {
	  warning("%d miss for '%s'\n", misscount, fontsetname);
	  XFreeStringList(miss);
	}
	xwi->fontset = 1;
      }
    }
  }
  xwi->caption_font = XLoadFont(x11_display(x11), fontname);
  //xwi->fs = XQueryFont(x11_display(x11), xwi->caption_font);

  vw->full_width = WidthOfScreen(x11_sc(x11));
  vw->full_height = HeightOfScreen(x11_sc(x11));
  vw->depth = x11_depth(x11);
  vw->bits_per_pixel = x11_bpp(x11);
  vw->prefer_msb = x11_prefer_msb(x11);
  vw->parent = parent;

  if (parent) {
    X11Window_info *p_xwi = (X11Window_info *)parent->private_data;
    X11Window *p_xw = parent->if_fullscreen ? p_xwi->full.xw : p_xwi->normal.xw;

    clip(vw, &w, &h);
    xwi->normal.xw = xw = x11window_create(x11, p_xw, w, h);
  } else {
    w = vw->full_width;
    h = vw->full_height;
    xwi->normal.xw = xw = x11window_create(x11, NULL, w, h);
  }
  xwi->xi = x11ximage_create(x11);

  vw->width  = w;
  vw->height = h;

  xwi->normal.pix = x11_create_pixmap(x11, x11window_win(xw), w, h, x11_depth(x11));
  xwi->normal.gc = x11_create_gc(x11, xwi->normal.pix, 0, 0);
  XSetFont(x11_display(x11), xwi->normal.gc, xwi->caption_font);
  XSetForeground(x11_display(x11), xwi->normal.gc, x11_black(x11));
  XSetBackground(x11_display(x11), xwi->normal.gc, x11_black(x11));

  if (parent) {
    x11window_map(xw);
    x11window_get_geometry(xw, &vw->x, &vw->y, &vw->width, &vw->height);
  }

  return vw;
}

static void
set_wallpaper(void *data, Image *p)
{
  Xlib_info *info = (Xlib_info *)data;
  set_background(info->root, p);
}

/* video window internal */

static void
recreate_pixmap_if_resized(VideoWindow *vw, WindowResource *wr)
{
  X11Window_info *xwi = (X11Window_info *)vw->private_data;
  X11Window *xw = vw->if_fullscreen ? xwi->full.xw : xwi->normal.xw;
  X11 *x11 = x11window_x11(xw);

  if (vw->render_width == 0 || vw->render_height == 0)
    warning_fnc("vw->render_width == 0 || vm->render_height == 0!\n");

  if (wr->pix_width != vw->render_width || wr->pix_height != vw->render_height) {
    x11_lock(x11);
    if (wr->pix)
      x11_free_pixmap(x11, wr->pix);
    wr->pix = x11_create_pixmap(x11, x11window_win(xw), vw->render_width, vw->render_height, x11_depth(x11));
    x11_unlock(x11);
    wr->pix_width  = vw->render_width;
    wr->pix_height = vw->render_height;
  }
}

static void
draw_caption(VideoWindow *vw)
{
  X11Window_info *xwi = (X11Window_info *)vw->private_data;
  X11Window *xw = vw->if_fullscreen ? xwi->full.xw : xwi->normal.xw;
  X11 *x11 = x11window_x11(xw);

  vw->if_caption = 1;

  if (!vw->if_fullscreen) {
    XTextProperty text;
    char *list[] = { vw->caption };

    XmbTextListToTextProperty(x11_display(x11), list, 1, XCompoundTextStyle, &text);
    x11_lock(x11);
    x11window_setwmname(xw, &text);
    XFree(text.value);
    x11_unlock(x11);
  } else {
    XFontSetExtents *extent = XExtentsOfFontSet(xwi->xfontset);
    int x = (vw->full_width - XmbTextEscapement(xwi->xfontset, vw->caption, strlen(vw->caption))) >> 1;
    int y = vw->full_height - extent->max_logical_extent.height;
    int oy = (vw->full_height + vw->render_height) >> 1;

    if (x < 0)
      x = 0;

    if (oy < y) {
      x11_lock(x11);
      XSetForeground(x11_display(x11), xwi->full.gc, x11_white(x11));
      if (xwi->fontset) {
	XmbDrawString(x11_display(x11), x11window_win(xw), xwi->xfontset, xwi->full.gc, x, y, vw->caption, strlen(vw->caption));
      } else {
	XDrawString(x11_display(x11), x11window_win(xw), xwi->full.gc, x, y, vw->caption, strlen(vw->caption));
      }
      x11_unlock(x11);
    } else {
      vw->if_caption = 0;
    }
  }
}

static void
erase_caption(VideoWindow *vw)
{
  X11Window_info *xwi = (X11Window_info *)vw->private_data;
  X11Window *xw = vw->if_fullscreen ? xwi->full.xw : xwi->normal.xw;
  X11 *x11 = x11window_x11(xw);
  XFontSetExtents *extent = XExtentsOfFontSet(xwi->xfontset);
  int x, y;

  x11_lock(x11);
  x = (vw->full_width - XmbTextEscapement(xwi->xfontset, vw->caption, strlen(vw->caption))) >> 1;
  x11_unlock(x11);
  y = vw->full_height - extent->max_logical_extent.height;

  if (!vw->if_caption)
    return;

  if (!vw->if_fullscreen)
    return;

  x11_lock(x11);
  XSetForeground(x11_display(x11), xwi->full.gc, x11_black(x11));
  if (xwi->fontset) {
    XmbDrawString(x11_display(x11), x11window_win(xw), xwi->xfontset, xwi->full.gc, x, y, vw->caption, strlen(vw->caption));
  } else {
    XDrawString(x11_display(x11), x11window_win(xw), xwi->full.gc, x, y, vw->caption, strlen(vw->caption));
  }
  x11_unlock(x11);

  vw->if_caption = 0;
}

/* video window methods */

static MemoryType
preferred_memory_type(VideoWindow *vw)
{
  X11Window_info *xwi = (X11Window_info *)vw->private_data;
  X11Window *xw = vw->if_fullscreen ? xwi->full.xw : xwi->normal.xw;
  X11 *x11 = x11window_x11(xw);

  return x11_if_shm(x11) ? _SHM : _NORMAL;
}

static ImageType
request_type(VideoWindow *vw, unsigned int w, unsigned int h, unsigned int types, int *direct_decode)
{
#ifdef USE_XV
  X11Window_info *xwi = (X11Window_info *)vw->private_data;
  X11Window *xw = vw->if_fullscreen ? xwi->full.xw : xwi->normal.xw;
  X11 *x11 = x11window_x11(xw);
  X11Xv *xv = x11->xv;
#endif

  static ImageType prefer_32_msb_direct[] = {
    _ARGB32, _RGB24, _BGRA32, _BGR24, _IMAGETYPE_TERMINATOR };
  static ImageType prefer_32_msb[] = {
    _RGBA32, _ABGR32,
    _RGB565, _BGR565, _RGB555, _BGR555,
    _INDEX, _GRAY, _GRAY_ALPHA, _BITMAP_MSBFirst, _BITMAP_LSBFirst, _IMAGETYPE_TERMINATOR };
  static ImageType prefer_32_lsb_direct[] = {
    _BGRA32, _BGR24, _ARGB32, _RGB24, _IMAGETYPE_TERMINATOR };
  static ImageType prefer_32_lsb[] = {
    _RGBA32, _ABGR32,
    _BGR565, _RGB565, _BGR555, _RGB555,
    _INDEX, _GRAY, _GRAY_ALPHA, _BITMAP_LSBFirst, _BITMAP_MSBFirst, _IMAGETYPE_TERMINATOR };
  static ImageType prefer_24_msb_direct[] = { _RGB24, _BGR24, _IMAGETYPE_TERMINATOR };
  static ImageType prefer_24_msb[] = {
    _RGBA32, _BGRA32, _ARGB32, _ABGR32,
    _RGB565, _BGR565, _RGB555, _BGR555,
    _INDEX, _GRAY, _GRAY_ALPHA, _BITMAP_LSBFirst, _BITMAP_MSBFirst, _IMAGETYPE_TERMINATOR };
  static ImageType prefer_24_lsb_direct[] = { _BGR24, _RGB24, _IMAGETYPE_TERMINATOR };
  static ImageType prefer_24_lsb[] = {
    _BGRA32, _RGBA32, _ABGR32, _ARGB32,
    _BGR565, _RGB565, _BGR555, _RGB555,
    _INDEX, _GRAY, _GRAY_ALPHA, _BITMAP_LSBFirst, _BITMAP_MSBFirst, _IMAGETYPE_TERMINATOR };
  static ImageType prefer_16_msb_direct[] = {
    _RGB565, _BGR565, _RGB555, _BGR555, _IMAGETYPE_TERMINATOR };
  static ImageType prefer_16_msb[] = {
    _BGR24, _RGB24,
    _BGRA32, _RGBA32, _ABGR32, _ARGB32,
    _INDEX, _GRAY, _GRAY_ALPHA, _BITMAP_LSBFirst, _BITMAP_MSBFirst, _IMAGETYPE_TERMINATOR };
  static ImageType prefer_16_lsb_direct[] = {
    _BGR565, _RGB565, _BGR555, _RGB555, _IMAGETYPE_TERMINATOR };
  static ImageType prefer_16_lsb[] = {
    _BGR24, _RGB24,
    _BGRA32, _RGBA32, _ABGR32, _ARGB32,
    _INDEX, _GRAY, _GRAY_ALPHA, _BITMAP_LSBFirst, _BITMAP_MSBFirst, _IMAGETYPE_TERMINATOR };
  ImageType *prefer_direct = NULL, *prefer = NULL;
  int i;

#ifdef USE_XV
  /* YUV support for Xv extension */
#define CHECK_AND_RETURN(f) \
  if (types & IMAGE_ ## f) { \
    debug_message_fnc("decoder can provide image in " #f " format.\n"); \
    if  (xv->capable_format & XV_ ## f ## _FLAG) { \
      debug_message_fnc("Xv can display " #f " format image, good.\n"); \
      *direct_decode = 1; \
      return _ ## f; \
    } else { \
      debug_message_fnc("Xv cannot display " #f " format image.\n"); \
    } \
  }
  {
    char *tmp = config_get(vw->c, "/enfle/plugins/video/Xlib/preferred_format");

    if (xv->image_width >= w && xv->image_height >= h) {
      if (strcasecmp(tmp, "planar") == 0) {
	CHECK_AND_RETURN(YV12);
	CHECK_AND_RETURN(I420);
	CHECK_AND_RETURN(YUY2);
	CHECK_AND_RETURN(UYVY);
      } else {
	CHECK_AND_RETURN(YUY2);
	CHECK_AND_RETURN(UYVY);
	CHECK_AND_RETURN(YV12);
	CHECK_AND_RETURN(I420);
      }
    }
  }
#undef CHECK_AND_RETURN
#endif

  switch (vw->bits_per_pixel) {
  case 32:
    prefer_direct = (vw->prefer_msb) ? prefer_32_msb_direct : prefer_32_lsb_direct;
    prefer        = (vw->prefer_msb) ? prefer_32_msb        : prefer_32_lsb;
    break;
  case 24:
    prefer_direct = (vw->prefer_msb) ? prefer_24_msb_direct : prefer_24_lsb_direct;
    prefer        = (vw->prefer_msb) ? prefer_24_msb        : prefer_24_lsb;
    break;
  case 16:
    prefer_direct = (vw->prefer_msb) ? prefer_16_msb_direct : prefer_16_lsb_direct;
    prefer        = (vw->prefer_msb) ? prefer_16_msb        : prefer_16_lsb;
    break;
  default:
    break;
  }

  for (i = 0; prefer_direct[i] != _IMAGETYPE_TERMINATOR; i++) {
    if (types & (1 << prefer_direct[i])) {
      *direct_decode = 1;
      return prefer_direct[i];
    }
  }

  for (i = 0; prefer[i] != _IMAGETYPE_TERMINATOR; i++) {
    if (types & (1 << prefer[i])) {
      *direct_decode = 0;
      return prefer[i];
    }
  }

  err_message_fnc("No appropriate image type. should not be happened\n");
  return _IMAGETYPE_TERMINATOR;
}

static int
calc_magnified_size(VideoWindow *vw, int use_hw_scale, unsigned int sw, unsigned int sh,
		    int *dw_return, int *dh_return)
{
  double s, ws, hs;
  unsigned int fw, fh;
  X11Window_info *xwi = (X11Window_info *)vw->private_data;
  X11Window *xw = vw->if_fullscreen ? xwi->full.xw : xwi->normal.xw;
  X11 *x11 = x11window_x11(xw);
#ifdef USE_XV
  X11Xv *xv = x11->xv;
#endif

  if (!use_hw_scale) {
    fw = vw->full_width;
    fh = vw->full_height;
  } else {
#ifdef USE_XV
    fw = (vw->full_width  > xv->image_width ) ? xv->image_width  : vw->full_width;
    fh = (vw->full_height > xv->image_height) ? xv->image_height : vw->full_height;
#else
    fw = vw->full_width;
    fh = vw->full_height;
#endif
  }

  switch (vw->render_method) {
  case _VIDEO_RENDER_MAGNIFY_DOUBLE:
    *dw_return = sw << 1;
    *dh_return = sh << 1;
    if (!(use_hw_scale && (*dw_return > fw || *dh_return > fh)))
      break;
    debug_message_fnc("clipping.\n");
    /* fall through */
  case _VIDEO_RENDER_MAGNIFY_SHORT_FULL:
    ws = (double)fw / (double)sw;
    hs = (double)fh / (double)sh;
    s = (ws * sh > fh) ? hs : ws;
    *dw_return = (int)(s * sw);
    *dh_return = (int)(s * sh);
    break;
  case _VIDEO_RENDER_MAGNIFY_LONG_FULL:
    ws = (double)fw / (double)sw;
    hs = (double)fh / (double)sh;
    s = (ws * sh > fh) ? ws : hs;
    *dw_return = (int)(s * sw);
    *dh_return = (int)(s * sh);
    break;
  case _VIDEO_RENDER_NORMAL:
  default:
    *dw_return = sw;
    *dh_return = sh;
    break;
  }

  return 1;
}

static int
set_event_mask(VideoWindow *vw, int mask)
{
  X11Window_info *xwi = (X11Window_info *)vw->private_data;
  X11Window *xw = vw->if_fullscreen ? xwi->full.xw : xwi->normal.xw;
  X11 *x11 = x11window_x11(xw);
  int x_mask = 0;

  if (mask & ENFLE_ExposureMask) 
    x_mask |= ExposureMask;
  if (mask & ENFLE_ButtonMask)
    x_mask |= (ButtonPressMask | ButtonReleaseMask);
  if (mask & ENFLE_KeyMask)
    x_mask |= (KeyPressMask | KeyReleaseMask);
  if (mask & ENFLE_PointerMask)
    x_mask |= PointerMotionMask;
  if (mask & ENFLE_WindowMask)
    x_mask |= (EnterWindowMask | LeaveWindowMask);

  /* needs for wait_mapped() */
  x_mask |= StructureNotifyMask;

  /* x_mask |= VisibilityChangeMask; */

  x11_lock(x11);
  x11window_set_event_mask(xw, x_mask);
  x11_unlock(x11);

  return 1;
}

#define TRANSLATE(k) case XK_##k: ev->key.key = ENFLE_KEY_##k; break
#define TRANSLATE_S(k,s) case XK_##k: ev->key.key = ENFLE_KEY_##s; break

static int
dispatch_event(VideoWindow *vw, VideoEventData *ev)
{
  X11Window_info *xwi = (X11Window_info *)vw->private_data;
  X11Window *xw = vw->if_fullscreen ? xwi->full.xw : xwi->normal.xw;
  GC gc = vw->if_fullscreen ? xwi->full.gc : xwi->normal.gc;
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
    if (errno == EINTR)
      return 0;
    err_message("Xlib: %s: select returns %d: errno = %d\n", __FUNCTION__, ret, errno);
    return 0;
  }

  ev->type = ENFLE_Event_None;
  x11_lock(x11);
  if (XCheckMaskEvent(x11_display(x11), x11window_mask(xw), &xev) == True) {
    switch (xev.type) {
    case Expose:
      if (!vw->if_direct) {
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
	/* Prepare to draw a rectangle */
	if (xwi->is_rect_draw) {
	  rect.x = xwi->lx;
	  rect.y = xwi->uy;
	  rect.width  = xwi->rx - xwi->lx + 1;
	  rect.height = xwi->dy - xwi->uy + 1;
	  XUnionRectWithRegion(&rect, region, region);
	}

	XClipBox(region, &rect);
	XSetRegion(x11_display(x11), gc, region);

	if (!vw->if_fullscreen)
	  __update(vw, rect.x, rect.y, rect.width, rect.height);
	else
	  update(vw, vw->render_width, vw->render_height);

	XSetClipMask(x11_display(x11), gc, None);
	XDestroyRegion(region);

	/* Draw a rectangle */
	if (xwi->is_rect_draw)
	  draw_rect_xor(vw, xwi->lx, xwi->uy, xwi->rx, xwi->dy);
      } else {
	update(vw, vw->render_width, vw->render_height);
      }
      if (vw->if_fullscreen)
	draw_caption(vw);
      ret = 0;
      break;
    case ButtonPress:
    case ButtonRelease:
      ev->type = (xev.type == ButtonPress) ?
	ENFLE_Event_ButtonPressed : ENFLE_Event_ButtonReleased;
      ev->pointer.x = xev.xbutton.x;
      ev->pointer.y = xev.xbutton.y;
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
      case Button4:
	ev->button.button = ENFLE_Button_4;
	break;
      case Button5:
	ev->button.button = ENFLE_Button_5;
	break;
      }
      ret = 1;
      break;
    case KeyPress:
    case KeyRelease:
      {
	char c;
	KeySym keysym;

	ev->type = (xev.type == KeyPress) ?
	  ENFLE_Event_KeyPressed : ENFLE_Event_KeyReleased;
	ev->key.modkey = ENFLE_MOD_None;
	if (xev.xkey.state & ShiftMask)
	  ev->key.modkey |= ENFLE_MOD_Shift;
	if (xev.xkey.state & ControlMask)
	  ev->key.modkey |= ENFLE_MOD_Ctrl;
	if (xev.xkey.state & Mod1Mask)
	  ev->key.modkey |= ENFLE_MOD_Alt;
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
	TRANSLATE_S(A,a); TRANSLATE_S(B,b); TRANSLATE_S(C,c); TRANSLATE_S(D,d);
	TRANSLATE_S(E,e); TRANSLATE_S(F,f); TRANSLATE_S(G,g); TRANSLATE_S(H,h);
	TRANSLATE_S(I,i); TRANSLATE_S(J,j); TRANSLATE_S(K,k); TRANSLATE_S(L,l);
	TRANSLATE_S(M,m); TRANSLATE_S(N,n); TRANSLATE_S(O,o); TRANSLATE_S(P,p);
	TRANSLATE_S(Q,q); TRANSLATE_S(R,r); TRANSLATE_S(S,s); TRANSLATE_S(T,t);
	TRANSLATE_S(U,u); TRANSLATE_S(V,v); TRANSLATE_S(W,w); TRANSLATE_S(X,x);
	TRANSLATE_S(Y,y); TRANSLATE_S(Z,z);
	default:
	  ev->key.key = ENFLE_KEY_Unknown;
	  break;
	}
      }
      ret = 1;
      break;
    case MotionNotify:
      ev->type = ENFLE_Event_PointerMoved;
      do {
	ev->pointer.button = ENFLE_Button_None;
	if (xev.xmotion.state & Button1Mask)
	  ev->pointer.button |= ENFLE_Button_1;
	if (xev.xmotion.state & Button2Mask)
	  ev->pointer.button |= ENFLE_Button_2;
	if (xev.xmotion.state & Button3Mask)
	  ev->pointer.button |= ENFLE_Button_3;
	if (xev.xmotion.state & Button4Mask)
	  ev->pointer.button |= ENFLE_Button_4;
	if (xev.xmotion.state & Button5Mask)
	  ev->pointer.button |= ENFLE_Button_5;
	ev->pointer.x = xev.xmotion.x;
	ev->pointer.y = xev.xmotion.y;
      } while (XCheckWindowEvent(x11_display(x11), x11window_win(xw),
				 PointerMotionMask, &xev));
      ret = 1;
      break;
    case EnterNotify:
      XSetInputFocus(x11_display(x11), x11window_win(vw->if_fullscreen ? xwi->full.xw : xwi->normal.xw), RevertToPointerRoot, CurrentTime);
      ev->type = ENFLE_Event_EnterWindow;
      ret = 1;
      break;
    case LeaveNotify:
      ev->type = ENFLE_Event_LeaveWindow;
      ret = 1;
      break;
    case ConfigureNotify:
      {
	XConfigureEvent *xcev = (XConfigureEvent *)&xev;
	ev->type = ENFLE_Event_WindowConfigured;
	ev->window.width = xcev->width;
	ev->window.height = xcev->height;
	debug_message_fnc("ConfigureNotify: (%d, %d) -> (%d, %d)\n", vw->width, vw->height, xcev->width, xcev->height);
	vw->width = xcev->width;
	vw->height = xcev->height;
      }
      ret = 1;
      break;
#if 0
    case VisibilityNotify:
      ret = 0;
      break;
#endif
    default:
      ret = 0;
      break;
    }
  }
  x11_unlock(x11);

  return ret;
}

static void
set_caption(VideoWindow *vw, char *cap)
{
  if (vw->caption) {
    erase_caption(vw);
    free(vw->caption);
  }
  if ((vw->caption = strdup(cap)) == NULL)
    return;

  draw_caption(vw);
}

static void
set_cursor(VideoWindow *vw, VideoWindowCursor vc)
{
  X11Window_info *xwi = (X11Window_info *)vw->private_data;
  X11Window *xw = vw->if_fullscreen ? xwi->full.xw : xwi->normal.xw;
  X11 *x11 = x11window_x11(xw);

  x11_lock(x11);
  switch (vc) {
  case _VIDEO_CURSOR_NORMAL:
    XDefineCursor(x11_display(x11), x11window_win(xw), normal_cursor);
    XFlush(x11_display(x11));
    break;
  case _VIDEO_CURSOR_WAIT:
    XDefineCursor(x11_display(x11), x11window_win(xw), wait_cursor);
    XFlush(x11_display(x11));
    break;
  case _VIDEO_CURSOR_INVISIBLE:
    XDefineCursor(x11_display(x11), x11window_win(xw), invisible_cursor);
    XFlush(x11_display(x11));
    break;
  }
  x11_unlock(x11);
}

static void
set_background(VideoWindow *vw, Image *p)
{
  X11Window_info *xwi = (X11Window_info *)vw->private_data;
  X11Window *xw = vw->if_fullscreen ? xwi->full.xw : xwi->normal.xw;
  X11 *x11 = x11window_x11(xw);
  X11XImage *xi = x11ximage_create(x11);
  Pixmap pix;
  GC gc = vw->if_fullscreen ? xwi->full.gc : xwi->normal.gc;

  debug_message_fn("() called\n");

  x11_lock(x11);

  x11ximage_convert(xi, p, IMAGE_INDEX_RENDERED, IMAGE_INDEX_WORK);

  pix = x11_create_pixmap(x11, x11window_win(xw), image_work_width(p), image_work_height(p), x11_depth(xi->x11));

  x11ximage_put(xi, pix, gc, 0, 0, 0, 0, image_work_width(p), image_work_height(p));
  XSetWindowBackgroundPixmap(x11_display(x11), x11window_win(xw), pix);
  XClearWindow(x11_display(x11), x11window_win(xw));

  x11_free_pixmap(x11, pix);
  x11ximage_destroy(xi);
  x11_unlock(x11);
}

static int
set_fullscreen_mode(VideoWindow *vw, VideoWindowFullscreenMode mode)
{
  int state_changed = 0;
  X11Window_info *xwi = (X11Window_info *)vw->private_data;
  X11Window *xw = vw->if_fullscreen ? xwi->full.xw : xwi->normal.xw;
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
    err_message("Xlib: %s: invalid fullscreen mode(%d).\n", __FUNCTION__, mode);
    return 0;
  }

  if (!state_changed)
    return 1;

  x11_lock(x11);
  x11window_unmap(xw);

  if (!vw->if_fullscreen) {
    recreate_pixmap_if_resized(vw, &xwi->normal);
    
    resize(vw, vw->render_width, vw->render_height);
    set_offset(vw, 0, 0);
    if (!xwi->xi->use_xv)
      x11ximage_put(xwi->xi, xwi->normal.pix, xwi->normal.gc, 0, 0, 0, 0, vw->render_width, vw->render_height);
    draw_caption(vw);
    x11window_map_raised(xwi->normal.xw);
    x11window_wait_mapped(xwi->normal.xw);
  } else {
    if (xwi->full.xw == NULL) {
      XSetWindowAttributes set_attr;
      X11Window_info *p_xwi = (X11Window_info *)vw->parent->private_data;
      X11Window *p_xw = vw->parent->if_fullscreen ? p_xwi->full.xw : p_xwi->normal.xw;

      xwi->full.xw = x11window_create(x11, p_xw, vw->full_width, vw->full_height);
      x11window_set_event_mask(xwi->full.xw, ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask | EnterWindowMask | LeaveWindowMask);
      xwi->full.pix = x11_create_pixmap(x11, x11window_win(xw), vw->render_width, vw->render_height, x11_depth(x11));
      xwi->full.gc = x11_create_gc(x11, xwi->full.pix, 0, 0);
      XSetFont(x11_display(x11), xwi->full.gc, xwi->caption_font);
      XSetForeground(x11_display(x11), xwi->full.gc, x11_black(x11));
      XSetBackground(x11_display(x11), xwi->full.gc, x11_black(x11));
      set_attr.override_redirect = True;
      XChangeWindowAttributes(x11_display(x11), x11window_win(xwi->full.xw), CWOverrideRedirect, &set_attr);
    }
    recreate_pixmap_if_resized(vw, &xwi->full);
    resize(vw, vw->render_width, vw->render_height);
    set_offset(vw, 0, 0);
    if (!xwi->xi->use_xv)
      x11ximage_put(xwi->xi, xwi->full.pix, xwi->full.gc, 0, 0, 0, 0, vw->render_width, vw->render_height);
    x11window_map_raised(xwi->full.xw);
    x11window_wait_mapped(xwi->full.xw);
  }
  x11_unlock(x11);

  return 1;
}

static int
resize(VideoWindow *vw, unsigned int w, unsigned int h)
{
  X11Window_info *xwi = (X11Window_info *)vw->private_data;
  X11Window *xw = vw->if_fullscreen ? xwi->full.xw : xwi->normal.xw;
  X11 *x11 = x11window_x11(xw);

  if (!vw->parent)
    return 1;

  if (w == 0 || h == 0)
    return 0;

  clip(vw, &w, &h);

  if (!vw->if_fullscreen) {
    unsigned int x, y;

    x11_lock(x11);
    x11window_get_geometry(xw, &vw->x, &vw->y, &vw->width, &vw->height);
    x11_unlock(x11);
    if (w == vw->width && h == vw->height)
      return 1;

    x = vw->x;
    y = vw->y;
    if (x + w > vw->full_width)
      x = vw->full_width - w;
    if (y + h > vw->full_height)
      y = vw->full_height - h;

    //debug_message_fnc("(%d, %d) -> (%d, %d) w: %d h: %d\n", vw->x, vw->y, x, y, w, h);
    debug_message_fnc("w: %d h: %d\n", w, h);

    x11_lock(x11);
    //x11window_unmap(xw);
    x11window_moveresize(xw, x, y, w, h);
    //x11window_map(xw);
    //x11window_wait_mapped(xw);
    x11_unlock(x11);
  } else {
    if (w == vw->render_width && h == vw->render_height)
      return 1;

    if (vw->render_width > w || vw->render_height > h) {
      x11_lock(x11);
      XSetForeground(x11_display(x11), xwi->full.gc, x11_black(x11));
      XFillRectangle(x11_display(x11), xwi->full.pix,  xwi->full.gc, 0, 0,
		     vw->full_width, vw->full_height);
      XFillRectangle(x11_display(x11), x11window_win(xw), xwi->full.gc, 0, 0,
		     vw->full_width, vw->full_height);
      x11_unlock(x11);
    }
  }

  x11_lock(x11);
  x11window_get_geometry(xw, &vw->x, &vw->y, &vw->width, &vw->height);
  x11_unlock(x11);

  return 1;
}

static int
move(VideoWindow *vw, unsigned int x, unsigned int y)
{
  X11Window_info *xwi = (X11Window_info *)vw->private_data;
  X11Window *xw = vw->if_fullscreen ? xwi->full.xw : xwi->normal.xw;
  X11 *x11 = x11window_x11(xw);

  if (!vw->if_fullscreen) {
    x11_lock(x11);
    x11window_move((X11Window *)vw->private_data, x, y);
    x11window_get_geometry((X11Window *)vw->private_data, &vw->x, &vw->y, &vw->width, &vw->height);
    x11_unlock(x11);
  }

  return 1;
}

static void
commit_offset(VideoWindow *vw, int offset_x, int offset_y)
{
  X11Window_info *xwi = (X11Window_info *)vw->private_data;
  int old_offset_x = vw->offset_x;
  int old_offset_y = vw->offset_y;

  vw->offset_x = offset_x;
  vw->offset_y = offset_y;

  if (!vw->if_fullscreen) {
    if (vw->render_width > vw->width) {
      if (vw->offset_x < 0)
	vw->offset_x = 0;
      if (vw->offset_x > (int)vw->render_width - (int)vw->width)
	vw->offset_x = vw->render_width - vw->width;
    } else {
      vw->offset_x = 0;
    }

    if (vw->render_height > vw->height) {
      if (vw->offset_y < 0)
	vw->offset_y = 0;
      if (vw->offset_y > (int)vw->render_height - (int)vw->height)
	vw->offset_y = vw->render_height - vw->height;
    } else {
      vw->offset_y = 0;
    }
  } else {
    if (vw->render_width > vw->full_width) {
      if (vw->offset_x < 0)
	vw->offset_x = 0;
      if (vw->offset_x > (int)vw->render_width - (int)vw->full_width)
	vw->offset_x = vw->render_width - vw->full_width;
    } else {
      vw->offset_x = 0;
    }

    if (vw->render_height > vw->full_height) {
      if (vw->offset_y < 0)
	vw->offset_y = 0;
      if (vw->offset_y > (int)vw->render_height - (int)vw->full_height)
	vw->offset_y = vw->render_height - vw->full_height;
    } else {
      vw->offset_y = 0;
    }
  }

  if (vw->offset_x != old_offset_x || vw->offset_y != old_offset_y)
    update(vw, vw->render_width, vw->render_height);
  if (xwi->is_rect_draw && (vw->offset_x != old_offset_x || vw->offset_y != old_offset_y))
    draw_rect_xor(vw, xwi->lx, xwi->uy, xwi->rx, xwi->dy);
}

static int
get_offset(VideoWindow *vw, int *x_return, int *y_return)
{
  *x_return = vw->offset_x;
  *y_return = vw->offset_y;

  return 1;
}

static int
set_offset(VideoWindow *vw, int x, int y)
{
  commit_offset(vw, x, y);

  return 1;
}

static int
adjust_offset(VideoWindow *vw, int x, int y)
{
  commit_offset(vw, vw->offset_x + x, vw->offset_y + y);

  return 1;
}

/* This is not a method */
static void
draw_rect_xor(VideoWindow *vw, unsigned int lx, unsigned int uy, unsigned int rx, unsigned int dy)
{
  X11Window_info *xwi = (X11Window_info *)vw->private_data;
  X11Window *xw = vw->if_fullscreen ? xwi->full.xw : xwi->normal.xw;
  GC gc = vw->if_fullscreen ? xwi->full.gc : xwi->normal.gc;
  X11 *x11 = x11window_x11(xw);
  XGCValues gcv_save;

  if (vw->if_fullscreen) {
    unsigned int slx = (vw->full_width  - vw->render_width ) >> 1;
    unsigned int suy = (vw->full_height - vw->render_height) >> 1;

    lx += slx;
    rx += slx;
    uy += suy;
    dy += suy;
  }
  lx -= vw->offset_x;
  rx -= vw->offset_x;
  uy -= vw->offset_y;
  dy -= vw->offset_y;

  x11_lock(x11);
  XGetGCValues(x11_display(x11), gc, GCFunction | GCForeground, &gcv_save);
  XSetForeground(x11_display(x11), gc, x11_white(x11));
  XSetFunction(x11_display(x11), gc, GXxor);
  XDrawRectangle(x11_display(x11), x11window_win(xw), xwi->normal.gc, lx, uy, rx - lx, dy - uy);
  XChangeGC(x11_display(x11), gc, GCFunction | GCForeground, &gcv_save);
  x11_unlock(x11);
}

static void
__update(VideoWindow *vw, unsigned int left, unsigned int top, unsigned int w, unsigned int h)
{
  X11Window_info *xwi = (X11Window_info *)vw->private_data;
  X11Window *xw = vw->if_fullscreen ? xwi->full.xw : xwi->normal.xw;
  X11 *x11 = x11window_x11(xw);

  left += xwi->xi->left;
  top += xwi->xi->top;

  //debug_message_fnc("(%d, %d) - (%d, %d) -> (%d, %d)\n", left + vw->offset_x, top + vw->offset_y, left + vw->offset_x + w, top + vw->offset_y + h, left, top);

  x11_lock(x11);
  if (!vw->if_fullscreen) {
    XCopyArea(x11_display(x11), xwi->normal.pix, x11window_win(xw), xwi->normal.gc,
	      vw->offset_x, vw->offset_y, w, h, left, top);
  } else {
    XCopyArea(x11_display(x11), xwi->full.pix, x11window_win(xw), xwi->full.gc,
	      vw->offset_x, vw->offset_y, w, h, left, top);
  }
  x11_unlock(x11);
}

static void
update(VideoWindow *vw, unsigned int w, unsigned int h)
{
  X11Window_info *xwi = (X11Window_info *)vw->private_data;
  X11Window *xw = vw->if_fullscreen ? xwi->full.xw : xwi->normal.xw;

  if (!vw->if_fullscreen) {
    if (vw->if_direct) {
      if (xwi->xi->use_xv) {
#ifdef USE_XV
	x11ximage_put_scaled(xwi->xi, x11window_win(xw), xwi->normal.gc, vw->offset_x, vw->offset_y, 0, 0, xwi->xi->xvimage->width, xwi->xi->xvimage->height, w, h);
#endif
      } else {
	x11ximage_put(xwi->xi, x11window_win(xw), xwi->normal.gc, vw->offset_x, vw->offset_y, 0, 0, w, h);
      }
    } else {
      __update(vw, 0, 0, w, h);
    }
  } else {
    int sx = (vw->full_width < w) ? 0 : (vw->full_width - w) / 2;
    int sy = (vw->full_height < h) ? 0 : (vw->full_height - h) / 2;

    if (vw->if_direct) {
      if (xwi->xi->use_xv) {
#ifdef USE_XV
	x11ximage_put_scaled(xwi->xi, x11window_win(xw), xwi->full.gc, vw->offset_x, vw->offset_y,
			     sx, sy,
			     xwi->xi->xvimage->width, xwi->xi->xvimage->height, w, h);
#endif
      } else {
	x11ximage_put(xwi->xi, x11window_win(xw), xwi->full.gc, vw->offset_x, vw->offset_y,
		      sx, sy,
		      w, h);
      }
    } else {
      __update(vw, sx, sy, w, h);
    }
  }
}

static void
erase_rect(VideoWindow *vw)
{
  X11Window_info *xwi = (X11Window_info *)vw->private_data;

  if (xwi->is_rect_draw != 1)
    return;
  draw_rect_xor(vw, xwi->lx, xwi->uy, xwi->rx, xwi->dy);
  xwi->is_rect_draw = 0;
}

static void
draw_rect(VideoWindow *vw, unsigned int lx, unsigned int uy, unsigned int rx, unsigned int dy)
{
  X11Window_info *xwi = (X11Window_info *)vw->private_data;

  erase_rect(vw);

  if (lx > rx)
    SWAP(lx, rx);
  if (uy > dy)
    SWAP(uy, dy);

  if (vw->if_fullscreen) {
    unsigned int slx = (vw->full_width  - vw->render_width ) >> 1;
    unsigned int suy = (vw->full_height - vw->render_height) >> 1;

    lx -= slx;
    rx -= slx;
    uy -= suy;
    dy -= suy;
  }
  lx += vw->offset_x;
  rx += vw->offset_x;
  uy += vw->offset_y;
  dy += vw->offset_y;

  draw_rect_xor(vw, lx, uy, rx, dy);
  xwi->is_rect_draw = 1;
  xwi->lx = lx;
  xwi->uy = uy;
  xwi->rx = rx;
  xwi->dy = dy;
}

static int
render(VideoWindow *vw, Image *p)
{
  return render_scaled(vw, p, 1, 0, 0);
}

static int
render_scaled(VideoWindow *vw, Image *p, int auto_calc, unsigned int _dw, unsigned int _dh)
{
  X11Window_info *xwi = (X11Window_info *)vw->private_data;
  unsigned int sw, sh;
  int dw, dh;
  int use_hw_scale;

#ifdef USE_XV
  use_hw_scale = x11ximage_is_hw_scalable(xwi->xi, p, NULL);
#else
  use_hw_scale = 0;
#endif

  if (!p->direct_renderable) {
    image_data_copy(p, IMAGE_INDEX_ORIGINAL, IMAGE_INDEX_RENDERED);
  } else {
    image_rendered_bpl(p) = image_bpl(p);
  }

  /* Run Effect hooks */
  {
    EnflePlugins *eps = get_enfle_plugins();
    Dlist *dl = enfle_plugins_get_names(eps, ENFLE_PLUGIN_EFFECT);
    Dlist_data *dd;
    Plugin *pl;
    EffectPlugin *ep;
    char *pname;
    int direction = 1, result;

    if (dl) {
      dlist_iter(dl, dd) {
	pname = hash_key_key(dlist_data(dd));
	if ((pl = pluginlist_get(eps->pls[ENFLE_PLUGIN_EFFECT], pname)) == NULL)
	  continue;
	ep = plugin_get(pl);
	if (ep->effect) {
	  //debug_message("Run %s hook\n", pname);
	  if (direction == 1)
	    result = ep->effect(p, IMAGE_INDEX_RENDERED, IMAGE_INDEX_WORK);
	  else
	    result = ep->effect(p, IMAGE_INDEX_WORK, IMAGE_INDEX_RENDERED);
	  if (result == 1)
	    direction = -direction;
	}
      }
      if (direction == -1)
	image_data_swap(p, IMAGE_INDEX_WORK, IMAGE_INDEX_RENDERED);
    }
  }

  sw = image_rendered_width(p);
  sh = image_rendered_height(p);

  if (auto_calc) {
    calc_magnified_size(vw, use_hw_scale, sw, sh, &dw, &dh);
  } else {
    dw = _dw;
    dh = _dh;
  }

  if (vw->render_method != _VIDEO_RENDER_NORMAL) {
    if (!use_hw_scale) {
      debug_message_fnc("Magnifying..\n");
      if (image_magnify(p, IMAGE_INDEX_RENDERED, IMAGE_INDEX_WORK, dw, dh, vw->interpolate_method))
	image_data_swap(p, IMAGE_INDEX_WORK, IMAGE_INDEX_RENDERED);
    }
  }

  image_data_copy(p, IMAGE_INDEX_RENDERED, IMAGE_INDEX_WORK);
  x11ximage_convert(xwi->xi, p, IMAGE_INDEX_WORK, IMAGE_INDEX_RENDERED);

  //debug_message_fnc("sw,sh (%d, %d)  dw,dh (%d, %d)  use_xv: %d, full: %d, direct: %d\n", sw, sh, dw, dh, xwi->xi->use_xv, vw->if_fullscreen, vw->if_direct);

  resize(vw, dw, dh);
  vw->render_width  = dw;
  vw->render_height = dh;

  if (!vw->if_fullscreen) {
    recreate_pixmap_if_resized(vw, &xwi->normal);
    if (!vw->if_direct)
      x11ximage_put(xwi->xi, xwi->normal.pix, xwi->normal.gc, 0, 0, 0, 0, dw, dh);
    update(vw, dw, dh);
  } else {
    recreate_pixmap_if_resized(vw, &xwi->full);
    if (!vw->if_direct)
      x11ximage_put(xwi->xi, xwi->full.pix, xwi->full.gc, 0, 0, 0, 0, dw, dh);
    update(vw, dw, dh);
    draw_caption(vw);
  }

  return 1;
}

static void
do_sync(VideoWindow *vw)
{
  X11Window_info *xwi = (X11Window_info *)vw->private_data;
  X11Window *xw = vw->if_fullscreen ? xwi->full.xw : xwi->normal.xw;
  X11 *x11 = x11window_x11(xw);

  x11_lock(x11);
  XSync(x11_display(x11), False);
  x11_unlock(x11);
}

static void
discard_key_event(VideoWindow *vw)
{
  X11Window_info *xwi = (X11Window_info *)vw->private_data;
  X11Window *xw = vw->if_fullscreen ? xwi->full.xw : xwi->normal.xw;
  X11 *x11 = x11window_x11(xw);
  XEvent xev;

  x11_lock(x11);
  while (XCheckWindowEvent(x11_display(x11), x11window_win(xw),
			   KeyPressMask | KeyReleaseMask, &xev)) ;
  x11_unlock(x11);
}

static void
discard_button_event(VideoWindow *vw)
{
  X11Window_info *xwi = (X11Window_info *)vw->private_data;
  X11Window *xw = vw->if_fullscreen ? xwi->full.xw : xwi->normal.xw;
  X11 *x11 = x11window_x11(xw);
  XEvent xev;

  x11_lock(x11);
  while (XCheckWindowEvent(x11_display(x11), x11window_win(xw),
			   ButtonPressMask | ButtonReleaseMask, &xev)) ;
  x11_unlock(x11);
}

static int
destroy_window(VideoWindow *vw)
{
  X11Window_info *xwi = (X11Window_info *)vw->private_data;
  X11Window *xw = vw->if_fullscreen ? xwi->full.xw : xwi->normal.xw;
  X11 *x11 = x11window_x11(xw);

  if (xwi->xi)
    x11ximage_destroy(xwi->xi);
  if (xwi->normal.pix)
    x11_free_pixmap(x11, xwi->normal.pix);
  if (xwi->normal.gc)
    x11_free_gc(x11, xwi->normal.gc);
  if (xwi->full.pix)
    x11_free_pixmap(x11, xwi->full.pix);
  if (xwi->full.gc)
    x11_free_gc(x11, xwi->full.gc);
  if (xwi->caption_font)
    XUnloadFont(x11_display(x11), xwi->caption_font);
#if !defined(DEBUG)
  /* XXX: This causes valgrind to seg. fault... */
  if (xwi->fontset)
    XFreeFontSet(x11_display(x11), xwi->xfontset);
#endif

  if (vw->caption)
    free(vw->caption);
  if (vw->parent)
    x11window_unmap(xw);
  x11window_destroy(xw);

  free(xwi);
  free(vw);

  return 1;
}
