/*
 * normal.c -- Normal UI plugin
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Jul  3 20:48:04 2001.
 * $Id: normal.c,v 1.47 2001/07/10 12:59:45 sian Exp $
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
#include "utils/misc.h"
#include "enfle/ui-plugin.h"
#include "enfle/loader.h"
#include "enfle/player.h"
#include "enfle/saver.h"
#include "enfle/effect.h"
#include "enfle/streamer.h"
#include "enfle/archiver.h"

static int ui_main(UIData *);

static UIPlugin plugin = {
  type: ENFLE_PLUGIN_UI,
  name: "Normal",
  description: "Normal UI plugin version 0.4.5",
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
  int use_hw_scale = 0;

  if (p->type == _YUY2 || p->type == _YV12 || p->type == _I420 || p->type == _UYVY)
    use_hw_scale = 1;

  switch (vw->render_method) {
  case _VIDEO_RENDER_NORMAL:
    p->if_magnified = 0;
    if (!use_hw_scale && p->rendered.image)
      memory_destroy(p->rendered.image);
    if (!use_hw_scale)
      p->rendered.image = memory_dup(p->image);
    p->rendered.width  = p->magnified.width  = p->width;
    p->rendered.height = p->magnified.height = p->height;
    break;
  case _VIDEO_RENDER_MAGNIFY_DOUBLE:
    video_window_set_cursor(vw, _VIDEO_CURSOR_WAIT);
    if (!use_hw_scale) {
      if (!image_magnify(p, p->width * 2, p->height * 2, vw->interpolate_method))
	show_message(__FUNCTION__ ": image_magnify() failed.\n");
      if (p->rendered.image)
	memory_destroy(p->rendered.image);
      p->rendered.image = memory_dup(p->magnified.image);
    }
    video_window_set_cursor(vw, _VIDEO_CURSOR_NORMAL);
    break;
  case _VIDEO_RENDER_MAGNIFY_SHORT_FULL:
    ws = (double)vw->full_width  / (double)p->width;
    hs = (double)vw->full_height / (double)p->height;
    s = (ws * p->height > vw->full_height) ? hs : ws;
    video_window_set_cursor(vw, _VIDEO_CURSOR_WAIT);
    if (!use_hw_scale) {
      image_magnify(p, s * p->width, s * p->height, vw->interpolate_method);
      if (p->rendered.image)
	memory_destroy(p->rendered.image);
      p->rendered.image = memory_dup(p->magnified.image);
    }
    video_window_set_cursor(vw, _VIDEO_CURSOR_NORMAL);
    break;
  case _VIDEO_RENDER_MAGNIFY_LONG_FULL:
    ws = (double)vw->full_width  / (double)p->width;
    hs = (double)vw->full_height / (double)p->height;
    s = (ws * p->height > vw->full_height) ? ws : hs;
    video_window_set_cursor(vw, _VIDEO_CURSOR_WAIT);
    if (!use_hw_scale) {
      image_magnify(p, s * p->width, s * p->height, vw->interpolate_method);
      if (p->rendered.image)
	memory_destroy(p->rendered.image);
      p->rendered.image = memory_dup(p->magnified.image);
    }
    video_window_set_cursor(vw, _VIDEO_CURSOR_NORMAL);
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

static int
save_image(Image *p, UIData *uidata, char *path, char *format)
{
  char *outpath;
  char *ext;
  FILE *fp;

  if ((ext = saver_get_ext(uidata->eps, format, uidata->c)) == NULL)
    return 0;
  if ((outpath = misc_replace_ext(path, ext)) == NULL) {
    show_message(__FUNCTION__ ": No enough memory.\n");
    return 0;
  }
  free(ext);

  if ((fp = fopen(outpath, "wb")) == NULL) {
    show_message(__FUNCTION__ ": Cannot open %s for writing.\n", outpath);
    return 0;
  }

  if (!saver_save(uidata->eps, format, p, fp, uidata->c, NULL)) {
    show_message("Save failed.\n");
    fclose(fp);
    return 0;
  }

  fclose(fp);
  free(outpath);

  return 1;
}

#define MAIN_LOOP_QUIT 0
#define MAIN_LOOP_NEXT 1
#define MAIN_LOOP_PREV -1
#define MAIN_LOOP_NEXTARCHIVE 2
#define MAIN_LOOP_PREVARCHIVE -2
#define MAIN_LOOP_LAST 3
#define MAIN_LOOP_FIRST -3
#define MAIN_LOOP_DELETE_FROM_LIST 4
#define MAIN_LOOP_DELETE_FILE 5
#define MAIN_LOOP_DO_NOTHING 6

typedef struct _main_loop {
  UIData *uidata;
  VideoWindow *vw;
  VideoEventData ev;
  VideoButton button;
  VideoKey key;
  Image *original_p;
  Image *p;
  Movie *m;
  char *path;
  int first_point;
  unsigned int old_x, old_y;
  int offset_x, offset_y;
  int ret;
} MainLoop;

/* action handler */

static int main_loop_quit(MainLoop *ml) { ml->ret = MAIN_LOOP_QUIT; return 1; }
static int main_loop_next(MainLoop *ml) { ml->ret = MAIN_LOOP_NEXT; return 1; }
static int main_loop_nextarchive(MainLoop *ml) { ml->ret = MAIN_LOOP_NEXTARCHIVE; return 1; }
static int main_loop_prev(MainLoop *ml) { ml->ret = MAIN_LOOP_PREV; return 1; }
static int main_loop_prevarchive(MainLoop *ml) { ml->ret = MAIN_LOOP_PREVARCHIVE; return 1; }
static int main_loop_last(MainLoop *ml) { ml->ret = MAIN_LOOP_LAST; return 1; }
static int main_loop_first(MainLoop *ml) { ml->ret = MAIN_LOOP_FIRST; return 1; }
static int main_loop_delete_file(MainLoop *ml) { ml->ret = MAIN_LOOP_DELETE_FILE; return 1; }
static int main_loop_delete_from_list(MainLoop *ml) { ml->ret = MAIN_LOOP_DELETE_FROM_LIST; return 1;}

static int
main_loop_toggle_fullscreen_mode(MainLoop *ml)
{
  video_window_set_fullscreen_mode(ml->vw, _VIDEO_WINDOW_FULLSCREEN_TOGGLE);
  return 1;
}
static int
main_loop_set_wallpaper(MainLoop *ml)
{
  if (ml->p) 
    ml->uidata->vp->set_wallpaper(ml->uidata->disp, ml->p);
  return 1;
}
static int
main_loop_toggle_interpolate(MainLoop *ml)
{
  switch (ml->vw->interpolate_method) {
  case _NOINTERPOLATE:
    ml->vw->interpolate_method = _BILINEAR;
    break;
  case _BILINEAR:
    ml->vw->interpolate_method = _NOINTERPOLATE;
    break;
  default:
    show_message(__FUNCTION__ ": invalid interpolate method %d\n", ml->vw->interpolate_method);
    ml->vw->interpolate_method = _NOINTERPOLATE;
    break;
  }
  magnify_if_requested(ml->vw, ml->p);
  video_window_render(ml->vw, ml->p);
  set_caption_string(ml->vw, ml->path, ml->p ? ml->p->format : ml->m->format);
  return 1;
}

static int
main_loop_magnify_main(MainLoop *ml)
{
  Image *p = ml->p;

  if (p) {
    magnify_if_requested(ml->vw, p);
    video_window_resize(ml->vw, p->magnified.width, p->magnified.height);
    video_window_set_offset(ml->vw, 0, 0);
    video_window_render(ml->vw, p);
  }
  return 1;
}
static int
main_loop_magnify_short(MainLoop *ml)
{
  ml->vw->render_method = (ml->vw->render_method == _VIDEO_RENDER_NORMAL) ?
    _VIDEO_RENDER_MAGNIFY_SHORT_FULL : _VIDEO_RENDER_NORMAL;
  return main_loop_magnify_main(ml);
}
static int
main_loop_magnify_long(MainLoop *ml)
{
  ml->vw->render_method = (ml->vw->render_method == _VIDEO_RENDER_NORMAL) ?
    _VIDEO_RENDER_MAGNIFY_LONG_FULL : _VIDEO_RENDER_NORMAL;
  return main_loop_magnify_main(ml);
}
static int
main_loop_magnify_double(MainLoop *ml)
{
  ml->vw->render_method = (ml->vw->render_method == _VIDEO_RENDER_NORMAL) ?
    _VIDEO_RENDER_MAGNIFY_DOUBLE : _VIDEO_RENDER_NORMAL;
  return main_loop_magnify_main(ml);
}

static int
main_loop_save_main(MainLoop *ml, char *format)
{
  video_window_set_cursor(ml->vw, _VIDEO_CURSOR_WAIT);
  if (!save_image(ml->p, ml->uidata, ml->path, format)) {
    show_message("save_image() (format %s) failed.\n", format);
    return 0;
  }
  video_window_set_cursor(ml->vw, _VIDEO_CURSOR_NORMAL);
  return 1;
}
static int
main_loop_save_png(MainLoop *ml)
{
  return main_loop_save_main(ml, (char *)"PNG");
}
static int
main_loop_save(MainLoop *ml)
{
  char *format;

  if ((format = config_get(ml->uidata->c, "/enfle/plugins/ui/normal/save_format")) == NULL) {
    show_message("save_format is not specified.\n");
    return 0;
  } else {
    return main_loop_save_main(ml, format);
  }
}

static int
main_loop_rotate_main(MainLoop *ml, int f)
{
  Image *old_p;
  UIData *uidata = ml->uidata;

  config_set_int(uidata->c, (char *)"/enfle/plugins/effect/rotate/function", f);
  if ((old_p = effect_call(uidata->eps, (char *)"Rotate", ml->p, uidata->c))) {
    if (old_p != ml->p) {
      image_destroy(old_p);
      video_window_resize(ml->vw, ml->p->width, ml->p->height);
    }
    magnify_if_requested(ml->vw, ml->p);
    video_window_set_offset(ml->vw, 0, 0);
    video_window_render(ml->vw, ml->p);
    return 1;
  } else {
    show_message("Rotate effect failed.\n");
    return 0;
  }
}
static int main_loop_rotate_right(MainLoop *ml) { return main_loop_rotate_main(ml, 1); }
static int main_loop_rotate_left(MainLoop *ml) { return main_loop_rotate_main(ml, -1); }
static int main_loop_flip_vertical(MainLoop *ml) { return main_loop_rotate_main(ml, 2); }
static int main_loop_flip_horizontal(MainLoop *ml) { return main_loop_rotate_main(ml, -2); }

static int
main_loop_gamma_main(MainLoop *ml, int index)
{
  UIData *uidata = ml->uidata;

  if (index == 3) {
    image_destroy(ml->p);
    ml->p = image_dup(ml->original_p);
    magnify_if_requested(ml->vw, ml->p);
    video_window_resize(ml->vw, ml->p->magnified.width, ml->p->magnified.height);
    video_window_set_offset(ml->vw, 0, 0);
    video_window_render(ml->vw, ml->p);
    return 1;
  } else {
    Image *old_p;
    int result = 1;

    config_set_int(uidata->c, (char *)"/enfle/plugins/effect/gamma/index", index);
    video_window_set_cursor(ml->vw, _VIDEO_CURSOR_WAIT);
    if ((old_p = effect_call(uidata->eps, (char *)"Gamma", ml->p, uidata->c))) {
      if (old_p != ml->p)
	image_destroy(old_p);
      magnify_if_requested(ml->vw, ml->p);
      video_window_set_offset(ml->vw, 0, 0);
      video_window_render(ml->vw, ml->p);
    } else {
      show_message("Gamma correction failed.\n");
      result = 0;
    }
    video_window_set_cursor(ml->vw, _VIDEO_CURSOR_NORMAL);
    return result;
  }
}
static int main_loop_gamma_1(MainLoop *ml) { return main_loop_gamma_main(ml, 0); }
static int main_loop_gamma_2(MainLoop *ml) { return main_loop_gamma_main(ml, 1); }
static int main_loop_gamma_3(MainLoop *ml) { return main_loop_gamma_main(ml, 2); }
static int main_loop_gamma_4(MainLoop *ml) { return main_loop_gamma_main(ml, 3); }
static int main_loop_gamma_5(MainLoop *ml) { return main_loop_gamma_main(ml, 4); }
static int main_loop_gamma_6(MainLoop *ml) { return main_loop_gamma_main(ml, 5); }
static int main_loop_gamma_7(MainLoop *ml) { return main_loop_gamma_main(ml, 6); }

/* Event dispatcher */

static int
main_loop_button_pressed(MainLoop *ml)
{
  ml->button = ml->ev.button.button;
  if (ml->button == ENFLE_Button_1)
    ml->first_point = 1;
  return 1;
}

static int
main_loop_key_pressed(MainLoop *ml)
{
  ml->key = ml->ev.key.key;
  return 1;
}

static int
main_loop_button_released(MainLoop *ml)
{
  VideoButton button = ml->button;

  ml->button = ENFLE_Button_None;
  if (button == ml->ev.button.button) {
    switch (button) {
    case ENFLE_Button_1:
      return main_loop_next(ml);
    case ENFLE_Button_2:
      return main_loop_nextarchive(ml);
    case ENFLE_Button_3:
      return main_loop_prev(ml);
    case ENFLE_Button_4:
      return main_loop_next(ml);
    case ENFLE_Button_5:
      return main_loop_prev(ml);
    default:
      return 1;
    }
  }
  return 0;
}

static int
main_loop_key_released(MainLoop *ml)
{
  VideoKey key = ml->key;
  int result;

  ml->key = ENFLE_KEY_Unknown;
  if (key == ml->ev.key.key) {
    switch (key) {
    case ENFLE_KEY_n:
    case ENFLE_KEY_space:
      return (ml->ev.key.modkey & ENFLE_MOD_Shift) ? main_loop_nextarchive(ml) : main_loop_next(ml);
    case ENFLE_KEY_b:
      return (ml->ev.key.modkey & ENFLE_MOD_Shift) ? main_loop_prevarchive(ml) : main_loop_prev(ml);
    case ENFLE_KEY_less:
      return main_loop_first(ml);
    case ENFLE_KEY_greater:
      return main_loop_last(ml);
    case ENFLE_KEY_q:
      return main_loop_quit(ml);
    case ENFLE_KEY_f:
      return main_loop_toggle_fullscreen_mode(ml);
    case ENFLE_KEY_d:
      if (config_get_boolean(ml->uidata->c, "/enfle/plugins/ui/normal/deletefile_without_shift", &result)) {
	return !(ml->ev.key.modkey & ENFLE_MOD_Shift) ?
	  main_loop_delete_file(ml) : main_loop_delete_from_list(ml);
      } else {
	return (ml->ev.key.modkey & ENFLE_MOD_Shift) ?
	  main_loop_delete_file(ml) : main_loop_delete_from_list(ml);
      }
    case ENFLE_KEY_s:
      if (ml->ev.key.modkey & ENFLE_MOD_Shift) {
	return main_loop_toggle_interpolate(ml);
      } else if (ml->ev.key.modkey & ENFLE_MOD_Ctrl) {
	return main_loop_save_png(ml);
      } else if (ml->ev.key.modkey & ENFLE_MOD_Alt) {
	return main_loop_save(ml);
      }
      break;
    case ENFLE_KEY_m:
      if (ml->ev.key.modkey & ENFLE_MOD_Shift)
	return main_loop_magnify_short(ml);
      else if (ml->ev.key.modkey & ENFLE_MOD_Alt)
	return main_loop_magnify_long(ml);
      else
	return main_loop_magnify_double(ml);
      break;
      return 1;
    case ENFLE_KEY_w:
      return main_loop_set_wallpaper(ml);
    case ENFLE_KEY_l:
      return main_loop_rotate_left(ml);
    case ENFLE_KEY_r:
      return main_loop_rotate_right(ml);
    case ENFLE_KEY_v:
      return main_loop_flip_vertical(ml);
    case ENFLE_KEY_h:
      return main_loop_flip_horizontal(ml);
    case ENFLE_KEY_1:
      return main_loop_gamma_1(ml);
    case ENFLE_KEY_2:
      return main_loop_gamma_2(ml);
    case ENFLE_KEY_3:
      return main_loop_gamma_3(ml);
    case ENFLE_KEY_4:
      return main_loop_gamma_4(ml);
    case ENFLE_KEY_5:
      return main_loop_gamma_5(ml);
    case ENFLE_KEY_6:
      return main_loop_gamma_6(ml);
    case ENFLE_KEY_7:
      return main_loop_gamma_7(ml);
    default:
      break;
    }
  }
  return 1;
}

static int
main_loop_pointer_moved(MainLoop *ml)
{
  if (ml->ev.pointer.button & ENFLE_Button_1) {
    if (ml->first_point) {
      ml->old_x = ml->ev.pointer.x;
      ml->old_y = ml->ev.pointer.y;
      ml->first_point = 0;
    } else {
      ml->offset_x = ml->ev.pointer.x - ml->old_x;
      ml->offset_y = ml->ev.pointer.y - ml->old_y;
      ml->old_x = ml->ev.pointer.x;
      ml->old_y = ml->ev.pointer.y;

      if (ml->offset_x != 0 || ml->offset_y != 0) {
	ml->button = ENFLE_Button_None;
	video_window_adjust_offset(ml->vw, ml->offset_x, ml->offset_y);
      }
    }
  }
  return 1;
}

static int
main_loop(UIData *uidata, VideoWindow *vw, Movie *m, Image *p, char *path)
{
  MainLoop ml;

  memset(&ml, 0, sizeof(ml));
  ml.button = ENFLE_Button_None;
  ml.key = ENFLE_KEY_Unknown;
  ml.uidata = uidata;
  ml.vw = vw;
  ml.m = m;
  ml.p = p;
  ml.path = path;
  ml.original_p = NULL;

  if (p) {
    if (p->type == _YUY2 || p->type == _YV12 || p->type == _I420 || p->type == _UYVY)
      vw->if_direct = 1;
    else
      vw->if_direct = 0;
    magnify_if_requested(vw, p);
    video_window_render(vw, p);
    set_caption_string(vw, path, p->format);
    ml.original_p = image_dup(p);
  } else if (m) {
    vw->if_direct = 1;
    vw->render_width  = m->width;
    vw->render_height = m->height;
    set_caption_string(vw, path, m->format);
  }

  video_window_set_offset(vw, 0, 0);

  ml.ret = MAIN_LOOP_DO_NOTHING;
  while (ml.ret == MAIN_LOOP_DO_NOTHING) {
    if (video_window_dispatch_event(vw, &ml.ev)) {
      switch (ml.ev.type) {
      case ENFLE_Event_ButtonPressed:
	main_loop_button_pressed(&ml);
	break;
      case ENFLE_Event_ButtonReleased:
	main_loop_button_released(&ml);
	video_window_discard_button_event(vw);
	break;
      case ENFLE_Event_KeyPressed:
	main_loop_key_pressed(&ml);
	break;
      case ENFLE_Event_KeyReleased:
	main_loop_key_released(&ml);
	video_window_discard_key_event(vw);
	break;
      case ENFLE_Event_PointerMoved:
	main_loop_pointer_moved(&ml);
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
	  return MAIN_LOOP_NEXT;
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
	return MAIN_LOOP_NEXT;
      }
    }
  }

  if (ml.original_p)
    image_destroy(ml.original_p);

  return ml.ret;
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
  ret = MAIN_LOOP_DO_NOTHING;
  while (ret != MAIN_LOOP_QUIT) {
    if (path == NULL)
      path = archive_iteration_start(a);
    else {
      switch (ret) {
      case MAIN_LOOP_DELETE_FROM_LIST:
	//debug_message("MAIN_LOOP_DELETE_FROM_LIST\n");
	path = archive_iteration_delete(a);
	ret = (archive_direction(a) == 1) ? MAIN_LOOP_NEXT : MAIN_LOOP_PREV;
	break;
      case MAIN_LOOP_DELETE_FILE:
	//debug_message("MAIN_LOOP_DELETE_FILE\n");
	if (strcmp(a->format, "NORMAL") == 0) {
	  unlink(s->path);
	  show_message("DELETED: %s\n", s->path);
	}
	path = archive_iteration_delete(a);
	ret = (archive_direction(a) == 1) ? MAIN_LOOP_NEXT : MAIN_LOOP_PREV;
	break;
      case MAIN_LOOP_NEXT:
	//debug_message("MAIN_LOOP_NEXT\n");
	path = archive_iteration_next(a);
	break;
      case MAIN_LOOP_PREV:
	//debug_message("MAIN_LOOP_PREV\n");
	path = archive_iteration_prev(a);
	break;
      case MAIN_LOOP_NEXTARCHIVE:
	//debug_message("MAIN_LOOP_NEXTARCHIVE\n");
	path = NULL;
	ret = MAIN_LOOP_NEXT;
	break;
      case MAIN_LOOP_PREVARCHIVE:
	//debug_message("MAIN_LOOP_PREVARCHIVE\n");
	path = NULL;
	ret = MAIN_LOOP_PREV;
	break;
      case MAIN_LOOP_FIRST:
	//debug_message("MAIN_LOOP_FIRST\n");
	path = archive_iteration_first(a);
	break;
      case MAIN_LOOP_LAST:
	//debug_message("MAIN_LOOP_LAST\n");
	path = archive_iteration_last(a);
	break;
      case MAIN_LOOP_DO_NOTHING:
	//debug_message("MAIN_LOOP_DO_NOTHING\n");
	break;
      default:
	show_message("*** " __FUNCTION__ "() returned unknown code %d\n", ret);
	path = NULL;
	break;
      }
    }

    if (!path)
      break;

    video_window_set_cursor(vw, _VIDEO_CURSOR_NORMAL);

    if (strcmp(a->format, "NORMAL") == 0) {
      if (strcmp(path, "-") == 0) {
	stream_make_fdstream(s, dup(0));
      } else {
	if (stat(path, &statbuf)) {
	  show_message("stat() failed: %s: %s\n", path, strerror(errno));
	  break;
	}
	if (S_ISDIR(statbuf.st_mode)) {
	  arc = archive_create(a);

	  debug_message("reading %s\n", path);

	  video_window_set_cursor(vw, _VIDEO_CURSOR_WAIT);
	  if (!archive_read_directory(arc, path, 1)) {
	    archive_destroy(arc);
	    ret = MAIN_LOOP_DELETE_FROM_LIST;
	    continue;
	  }
	  (ret == MAIN_LOOP_PREV) ? archive_iteration_last(arc) : archive_iteration_first(arc);
	  video_window_set_cursor(vw, _VIDEO_CURSOR_NORMAL);
	  ret = process_files_of_archive(uidata, arc);
	  if (arc->nfiles == 0) {
	    /* Now that all paths are deleted in this archive, should be deleted wholly. */
	    ret = MAIN_LOOP_DELETE_FROM_LIST;
	  }
	  archive_destroy(arc);
	  continue;
	} else if (!S_ISREG(statbuf.st_mode)) {
	  ret = MAIN_LOOP_DELETE_FROM_LIST;
	  continue;
	}

	if (streamer_identify(eps, s, path)) {

	  debug_message("Stream identified as %s\n", s->format);

	  if (!streamer_open(eps, s, s->format, path)) {
	    show_message("Stream %s [%s] cannot open\n", s->format, path);
	    ret = MAIN_LOOP_DELETE_FROM_LIST;
	    continue;
	  }
	} else if (!stream_make_filestream(s, path)) {
	  show_message("Stream NORMAL [%s] cannot open\n", path);
	  ret = MAIN_LOOP_DELETE_FROM_LIST;
	  continue;
	}
      }

      arc = archive_create(a);
      if (archiver_identify(eps, arc, s)) {

	debug_message("Archiver identified as %s\n", arc->format);

	if (archiver_open(eps, arc, arc->format, s)) {
	  (ret == MAIN_LOOP_PREV) ? archive_iteration_last(arc) : archive_iteration_first(arc);
	  ret = process_files_of_archive(uidata, arc);
	  archive_destroy(arc);
	  continue;
	} else {
	  show_message("Archive %s [%s] cannot open\n", arc->format, path);
	  ret = MAIN_LOOP_DELETE_FROM_LIST;
	  continue;
	}
      }
      archive_destroy(arc);
    } else if (!archive_open(a, s, path)) {
      show_message("File %s in %s archive cannot open\n", path, a->format);
      ret = MAIN_LOOP_DELETE_FROM_LIST;
      continue;
    }

    f = LOAD_NOT;
    video_window_set_cursor(vw, _VIDEO_CURSOR_WAIT);
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
	  ret = MAIN_LOOP_DELETE_FROM_LIST;
	  continue;
	}
      } else {
	stream_close(s);
	show_message("%s identification failed\n", path);
	ret = MAIN_LOOP_DELETE_FROM_LIST;
	continue;
      }

      video_window_set_cursor(vw, _VIDEO_CURSOR_NORMAL);
      ret = main_loop(uidata, vw, m, NULL, path);
      movie_unload(m);
    } else {

      debug_message("%s: (%d, %d) %s\n", path, p->width, p->height, image_type_to_string(p->type));

      if (p->comment) {
	show_message("comment: %s\n", p->comment);
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

  return ret;
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

  debug_message("Normal: " __FUNCTION__ "()\n");

  if ((disp = vp->open_video(NULL, c)) == NULL) {
    show_message("open_video() failed\n");
    return 0;
  }
  uidata->disp = disp;

  uidata->vw = vw = vp->open_window(disp, vp->get_root(disp), 600, 400);

  video_window_set_event_mask(vw,
			      ENFLE_ExposureMask | ENFLE_ButtonMask |
			      ENFLE_KeyMask | ENFLE_PointerMask); /* ENFLE_WindowMask */

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
