/*
 * video.h -- Video header
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun May  1 16:56:52 2005.
 * $Id: video.h,v 1.22 2005/05/01 15:37:55 sian Exp $
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

#ifndef _VIDEO_H
#define _VIDEO_H

#include "image.h"
#include "utils/libconfig.h"

typedef enum {
  ENFLE_KEY_Empty, ENFLE_KEY_Unknown,
  ENFLE_KEY_0, ENFLE_KEY_1, ENFLE_KEY_2, ENFLE_KEY_3, ENFLE_KEY_4,
  ENFLE_KEY_5, ENFLE_KEY_6, ENFLE_KEY_7, ENFLE_KEY_8, ENFLE_KEY_9,
  ENFLE_KEY_BackSpace, ENFLE_KEY_Tab, ENFLE_KEY_Return, ENFLE_KEY_Escape, ENFLE_KEY_Delete,
  ENFLE_KEY_Left, ENFLE_KEY_Up, ENFLE_KEY_Right, ENFLE_KEY_Down, ENFLE_KEY_Insert,
  ENFLE_KEY_space, ENFLE_KEY_colon, ENFLE_KEY_semicolon,
  ENFLE_KEY_less, ENFLE_KEY_equal, ENFLE_KEY_greater, ENFLE_KEY_question, ENFLE_KEY_at,
  ENFLE_KEY_a, ENFLE_KEY_b, ENFLE_KEY_c, ENFLE_KEY_d,
  ENFLE_KEY_e, ENFLE_KEY_f, ENFLE_KEY_g, ENFLE_KEY_h,
  ENFLE_KEY_i, ENFLE_KEY_j, ENFLE_KEY_k, ENFLE_KEY_l,
  ENFLE_KEY_m, ENFLE_KEY_n, ENFLE_KEY_o, ENFLE_KEY_p,
  ENFLE_KEY_q, ENFLE_KEY_r, ENFLE_KEY_s, ENFLE_KEY_t,
  ENFLE_KEY_u, ENFLE_KEY_v, ENFLE_KEY_w, ENFLE_KEY_x,
  ENFLE_KEY_y, ENFLE_KEY_z,
} VideoKey;

typedef enum {
  ENFLE_MOD_None  = 0,
  ENFLE_MOD_Shift = 1,
  ENFLE_MOD_Ctrl  = 2,
  ENFLE_MOD_Alt   = 4
} VideoModifierKey;

typedef enum {
  ENFLE_Button_None = 0,
  ENFLE_Button_1    = 1,
  ENFLE_Button_2    = 2,
  ENFLE_Button_3    = 4,
  ENFLE_Button_4    = 8,
  ENFLE_Button_5    = 16
} VideoButton;

#define ENFLE_ExposureMask 1
#define ENFLE_ButtonMask   2
#define ENFLE_KeyMask      4
#define ENFLE_PointerMask  8
#define ENFLE_WindowMask   16

typedef enum {
  ENFLE_Event_None,
  ENFLE_Event_Exposure,
  ENFLE_Event_ButtonPressed,
  ENFLE_Event_ButtonReleased,
  ENFLE_Event_KeyPressed,
  ENFLE_Event_KeyReleased,
  ENFLE_Event_PointerMoved,
  ENFLE_Event_EnterWindow,
  ENFLE_Event_LeaveWindow,
  ENFLE_Event_WindowConfigured
} VideoEventType;

typedef union {
  VideoEventType type;
  struct {
    VideoEventType type;
    unsigned int x,y;
  } exposure;
  struct {
    VideoEventType type;
    VideoButton button;
  } button;
  struct {
    VideoEventType type;
    VideoModifierKey modkey;
    VideoKey key;
  } key;
  struct {
    VideoEventType type;
    VideoButton button;
    unsigned int x, y;
  } pointer;
  struct {
    VideoEventType type;
    unsigned int width, height;
    unsigned int x, y;
  } window;
} VideoEventData;

typedef enum _videowindowfullscreenmode {
  _VIDEO_WINDOW_FULLSCREEN_ON,
  _VIDEO_WINDOW_FULLSCREEN_OFF,
  _VIDEO_WINDOW_FULLSCREEN_TOGGLE
} VideoWindowFullscreenMode;

typedef enum _videorendermethod {
  _VIDEO_RENDER_NORMAL,
  _VIDEO_RENDER_MAGNIFY_DOUBLE,
  _VIDEO_RENDER_MAGNIFY_SHORT_FULL,
  _VIDEO_RENDER_MAGNIFY_LONG_FULL
} VideoRenderMethod;

typedef enum _videowindowcursor {
  _VIDEO_CURSOR_NORMAL,
  _VIDEO_CURSOR_WAIT,
  _VIDEO_CURSOR_INVISIBLE
} VideoWindowCursor;

#define VIDEO_COLORSPACE_RGB (1 << 0)
#define VIDEO_COLORSPACE_YUV (1 << 1)

typedef struct _video_window VideoWindow;
struct _video_window {
  Config *c;
  VideoWindow *parent;
  void *private_data;
  unsigned int x, y;
  unsigned int width, height;
  unsigned int full_width, full_height;
  unsigned int render_width, render_height;
  int offset_x, offset_y;
  int depth, bits_per_pixel;
  int if_fullscreen, if_direct, if_caption, prefer_msb;
  unsigned int displayable_colorspace;
  char *caption;
  VideoRenderMethod render_method;
  ImageInterpolateMethod interpolate_method;

  MemoryType (*preferred_memory_type)(VideoWindow *);
  ImageType (*request_type)(VideoWindow *, unsigned int, int *);
  int (*calc_magnified_size)(VideoWindow *, int, unsigned int, unsigned int, int *, int *);
  int (*set_event_mask)(VideoWindow *, int);
  int (*dispatch_event)(VideoWindow *, VideoEventData *);
  void (*set_caption)(VideoWindow *, char *);
  void (*set_cursor)(VideoWindow *, VideoWindowCursor);
  void (*set_background)(VideoWindow *, Image *);
  int (*set_fullscreen_mode)(VideoWindow *, VideoWindowFullscreenMode);
  int (*resize)(VideoWindow *, unsigned int, unsigned int);
  int (*move)(VideoWindow *, unsigned int, unsigned int);
  int (*get_offset)(VideoWindow *, int *, int *);
  int (*set_offset)(VideoWindow *, int, int);
  int (*adjust_offset)(VideoWindow *, int, int);
  void (*erase_rect)(VideoWindow *);
  void (*draw_rect)(VideoWindow *, unsigned int, unsigned int, unsigned int, unsigned int);
  int (*render)(VideoWindow *, Image *);
  int (*render_scaled)(VideoWindow *, Image *, int, unsigned int, unsigned int);
  void (*update)(VideoWindow *, unsigned int, unsigned int, unsigned int, unsigned int);
  void (*do_sync)(VideoWindow *);
  void (*discard_key_event)(VideoWindow *);
  void (*discard_button_event)(VideoWindow *);
  int (*destroy)(VideoWindow *);
};

#define video_window_get_rgb(vw) ((vw)->displayable_colorspace & VIDEO_COLORSPACE_RGB)
#define video_window_get_yuv(vw) ((vw)->displayable_colorspace & VIDEO_COLORSPACE_YUV)
#define video_window_set_rgb(vw) (vw)->displayable_colorspace |= VIDEO_COLORSPACE_RGB
#define video_window_set_yuv(vw) (vw)->displayable_colorspace |= VIDEO_COLORSPACE_YUV

#define video_window_preferred_memory_type(vw) (vw)->preferred_memory_type((vw))
#define video_window_request_type(vw, types, ddp) (vw)->request_type((vw), (types), (ddp))
#define video_window_calc_magnified_size(vw, uhs, sw, sh, dw, dh) (vw)->calc_magnified_size((vw), (uhs), (sw), (sh), (dw), (dh))
#define video_window_set_event_mask(vw, m) (vw)->set_event_mask((vw), (m))
#define video_window_dispatch_event(vw, ved) (vw)->dispatch_event((vw), (ved))
#define video_window_set_caption(vw, c) (vw)->set_caption((vw), (c))
#define video_window_set_cursor(vw, vc) (vw)->set_cursor((vw), (vc))
#define video_window_set_background(vw, p) (vw)->set_background((vw), (p))
#define video_window_set_fullscreen_mode(vw, m) (vw)->set_fullscreen_mode((vw), (m))
#define video_window_resize(vw, w, h) (vw)->resize((vw), (w), (h))
#define video_window_move(vw, x, y) (vw)->move((vw), (x), (y))
#define video_window_get_offset(vw, xp, yp) (vw)->get_offset((vw), (xp), (yp))
#define video_window_set_offset(vw, x, y) (vw)->set_offset((vw), (x), (y))
#define video_window_adjust_offset(vw, x, y) (vw)->adjust_offset((vw), (x), (y))
#define video_window_erase_rect(vw) (vw)->erase_rect((vw))
#define video_window_draw_rect(vw, lx, uy, rx, dy) (vw)->draw_rect((vw), (lx), (uy), (rx), (dy))
#define video_window_render(vw, p) (vw)->render((vw), (p))
#define video_window_render_scaled(vw, p, ac, dw, dh) (vw)->render_scaled((vw), (p), (ac), (dw), (dh))
#define video_window_update(vw, x, y, w, h) (vw)->update((vw), (x), (y), (w), (h))
#define video_window_sync(vw) (vw)->do_sync((vw))
#define video_window_discard_key_event(vw) (vw)->discard_key_event((vw))
#define video_window_discard_button_event(vw) (vw)->discard_button_event((vw))
#define video_window_destroy(vw) (vw)->destroy((vw))

#endif
