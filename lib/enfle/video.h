/*
 * video.h -- Video header
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Oct 21 02:43:38 2000.
 * $Id: video.h,v 1.1 2000/10/20 18:09:29 sian Exp $
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

typedef struct _video_window {
  void *private;
  unsigned int x, y;
  unsigned int width, height;
  int depth, bits_per_pixel;
} VideoWindow;

typedef enum {
  ENFLE_KEY_Unknown,
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
  ENFLE_MOD_Shift, ENFLE_MOD_Ctrl, ENFLE_MOD_Alt
} VideoModifierKey;

typedef enum {
  ENFLE_Button_None,
  ENFLE_Button_1,  ENFLE_Button_2,  ENFLE_Button_3,  ENFLE_Button_4,  ENFLE_Button_5
} VideoButton;

#define ENFLE_ExposureMask 1
#define ENFLE_ButtonMask 2
#define ENFLE_KeyMask 4
#define ENFLE_PointerMask 8
#define ENFLE_WindowMask 16

typedef enum {
  ENFLE_Event_Exposure,
  ENFLE_Event_ButtonPressed,
  ENFLE_Event_ButtonReleased,
  ENFLE_Event_KeyPressed,
  ENFLE_Event_PointerMoved,
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

#endif
