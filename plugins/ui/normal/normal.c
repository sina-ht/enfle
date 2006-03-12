/*
 * normal.c -- Normal UI plugin
 * (C)Copyright 2000-2006 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Mar 12 17:17:39 2006.
 * $Id: normal.c,v 1.94 2006/03/12 08:25:11 sian Exp $
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

/* Please ignore. */
#undef ENABLE_GUI_GTK

#define REQUIRE_UNISTD_H
#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "utils/libstring.h"
#include "utils/misc.h"
#include "utils/timer.h"
#include "utils/timer_gettimeofday.h"
#include "utils/converter.h"
#include "enfle/ui-plugin.h"
#include "enfle/ui-action.h"
#include "enfle/loader.h"
#include "enfle/player.h"
#include "enfle/saver.h"
#include "enfle/effect.h"
#include "enfle/effect-plugin.h"
#include "enfle/streamer.h"
#include "enfle/archiver.h"
#include "enfle/identify.h"
#include "enfle/cache_image.h"
#ifdef ENABLE_GUI_GTK
# include "eventnotifier.h"
# include "gui_gtk.h"
#endif

static int ui_main(UIData *);

static UIPlugin plugin = {
  .type = ENFLE_PLUGIN_UI,
  .name = "Normal",
  .description = "Normal UI plugin version 0.6.2",
  .author = "Hiroshi Takekawa",

  .ui_main = ui_main,
};

ENFLE_PLUGIN_ENTRY(ui_normal)
{
  UIPlugin *uip;

  if ((uip = (UIPlugin *)calloc(1, sizeof(UIPlugin))) == NULL)
    return NULL;
  memcpy(uip, &plugin, sizeof(UIPlugin));

  return (void *)uip;
}

ENFLE_PLUGIN_EXIT(ui_normal, p)
{
  free(p);
}

/* for internal use */

#define MAIN_LOOP_QUIT 0
#define MAIN_LOOP_NEXT 1
#define MAIN_LOOP_PREV -1
#define MAIN_LOOP_NEXTARCHIVE 2
#define MAIN_LOOP_PREVARCHIVE -2
#define MAIN_LOOP_LAST 3
#define MAIN_LOOP_FIRST -3
#define MAIN_LOOP_DELETE_FROM_LIST 4
#define MAIN_LOOP_DELETE_FROM_LIST_DIR 5
#define MAIN_LOOP_DELETE_FILE 6
#define MAIN_LOOP_DO_NOTHING 7
#define MAIN_LOOP_NEXT5 8
#define MAIN_LOOP_PREV5 -8
#define MAIN_LOOP_NEXTARCHIVE5 9
#define MAIN_LOOP_PREVARCHIVE5 -9

typedef struct _main_loop {
  UIData *uidata;
  Archive *a;
  Stream *st;
  VideoWindow *vw;
  VideoEventData ev;
  VideoButton button;
  VideoKey key;
  Image *p;
  Movie *m;
  Timer *timer;
  //unsigned int magnified_width, magnified_height; /* For arbitrary size magnification */
  char *path;
  VideoButton pointed_button;
  unsigned int old_x, old_y;
  unsigned int lx, uy, rx, dy;
  int offset_x, offset_y;
  int ret;
} MainLoop;

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

static int
render(MainLoop *ml)
{
  video_window_render_scaled(ml->vw, ml->p, 1, 0, 0);
  return 1;
}

static int
save_image(Image *p, UIData *uidata, char *path, char *format)
{
  char *outpath;
  char *ext;
  FILE *fp;
  EnflePlugins *eps = get_enfle_plugins();

  if ((ext = saver_get_ext(eps, format, uidata->c)) == NULL)
    return 0;
  if ((outpath = misc_replace_ext(path, ext)) == NULL) {
    show_message_fnc("No enough memory.\n");
    return 0;
  }
  free(ext);

  if ((fp = fopen(outpath, "wb")) == NULL) {
    show_message_fnc("Cannot open %s for writing.\n", outpath);
    return 0;
  }

  if (!saver_save(eps, format, p, fp, uidata->c, NULL)) {
    show_message("Save failed.\n");
    fclose(fp);
    return 0;
  }

  fclose(fp);
  free(outpath);

  return 1;
}

static void
convert_cat(String *cap, char *s, Config *c)
{
  int res;

  if (config_get_boolean(c, "/enfle/plugins/ui/normal/filename_code_conversion", &res)) {
    char **froms = config_get_list(c, "/enfle/plugins/ui/normal/filename_code_from", &res);
    char *to = config_get_str(c, "/enfle/plugins/ui/normal/filename_code_to");
    char *from;
    int i = 0;

    if (res && to) {
      while ((from = froms[i++])) {
	char *tmp;

	//debug_message_fnc("%s->%s: %s\n", from, to, s);
	if ((res = converter_convert(s, &tmp, strlen(s), from, to)) < 0)
	  continue;
	string_cat(cap, tmp);
	free(tmp);
	return;
      }
    }
  }

  string_cat(cap, s);
}

static void
convert_path(String *cap, char *s, Config *c)
{
  static char delimiters[] = { '/', '#', '\0' };
  char *part, **parts, *used_delim;
  int i;

  if ((parts = misc_str_split_delimiters(s, delimiters, &used_delim)) == NULL) {
    err_message_fnc("No enough memory.\n");
    return;
  }

  i = 0;
  while ((part = parts[i])) {
    convert_cat(cap, part, c);
    if (used_delim[i] != '\0')
      string_cat_ch(cap, used_delim[i]);
    i++;
  }
  misc_free_str_array(parts);
  free(used_delim);
}


static void
set_caption_string(MainLoop *ml)
{
  String *cap;
  VideoWindow *vw = ml->vw;
  char *template;
  char *fullpath;
  int i;
  int literal_start, literal_mode;

  if ((cap = string_create()) == NULL)
    return;

  if ((template = config_get_str(ml->uidata->c, "/enfle/plugins/ui/normal/caption_template")) == NULL)
    template = (char *)"%i/%I:%p %f %xx%y";

  i = 0;
  literal_mode = 0;
  literal_start = 0;
  while (template[i]) {
    if (template[i] != '%') {
      if (literal_mode != 1) {
	literal_start = i;
	literal_mode = 1;
      }
    } else {
      if (literal_mode == 1) {
	string_ncat(cap, &template[literal_start], i - literal_start);
	literal_mode = 0;
      }
      i++;
      switch (template[i]) {
      case '\0':
	i--; break;
      case '%':
	string_cat_ch(cap, '%'); break;
      case ')':
      case '>':
	string_cat_ch(cap, vw->interpolate_method == _BILINEAR ? ')' : '>'); break;
      case 'x':
	string_catf(cap, "%d", vw->render_width); break;
      case 'y':
	string_catf(cap, "%d", vw->render_height); break;
      case 'f':
	if (ml->p)
	  string_cat(cap, (ml->p->format_detail != NULL) ? ml->p->format_detail : ml->p->format);
	else if (ml->m) {
	  if (ml->m->player_name)
	    string_cat(cap, ml->m->player_name);
	  if (ml->m->format) {
	    string_cat_ch(cap, '(');
	    string_cat(cap, ml->m->format);
	    string_cat_ch(cap, ')');
	  }
	}
	if (strcmp(ml->st->format, "FILE") != 0) {
	  string_cat_ch(cap, '/');
	  string_cat(cap, ml->st->format);
	}
	if (strcmp(ml->a->format, "NORMAL") != 0) {
	  string_cat_ch(cap, '#');
	  string_cat(cap, ml->a->format);
	}
	break;
      case 'F':
	string_cat(cap, ml->p->format); break;
      case 'p':
	if (ml->a->parent && strcmp(ml->a->parent->format, "NORMAL") != 0 && ml->a->parent->path[0] != 0) {
	  debug_message_fnc("parent path = %s\n", ml->a->parent->path);
	  convert_path(cap, misc_basename(ml->a->parent->path), ml->uidata->c);
	  fullpath = archive_getpathname(ml->a, ml->path);
	  convert_path(cap, fullpath, ml->uidata->c);
	  free(fullpath);
	} else {
	  convert_path(cap, misc_basename(ml->a->path), ml->uidata->c);
	  string_cat_ch(cap, '/');
	  convert_path(cap, ml->path, ml->uidata->c); break;
	}
	break;
      case 'P':
	convert_path(cap, ml->a->path, ml->uidata->c);
	convert_path(cap, ml->path, ml->uidata->c); break;
      case 'N':
	string_cat(cap, PROGNAME); break;
      case 'i':
	string_catf(cap, "%d", archive_iteration_index(ml->a));
	break;
      case 'I':
	string_catf(cap, "%d", archive_nfiles(ml->a));
	break;
      default:
	warning("Found an unknown format character %c.\n", template[i]);
	string_ncat(cap, &template[i - 1], 2);
	break;
      }
    }
    i++;
  }
  if (literal_mode == 1)
    string_ncat(cap, &template[literal_start], i - literal_start);

  /* XXX: dirty hack */
  {
    char *to = config_get_str(ml->uidata->c, "/enfle/plugins/ui/normal/filename_code_to");

    if (to && strcasecmp(to, "EUC-JISX0213") == 0) {
      char *p = string_get(cap);
      int i, l = 0, len = strlen(p);

      for (i = 0; i < len; i += l) {
	if (*p == (char)0xa2 && *(p + 1) == (char)0xb2) {
	  *p = 0xa1;
	  *(p + 1) = 0xc1;
	  l = 2;
	} else {
	  l = mblen(p, len - i);
	  if (l <= 0)
	    break;
	}
	p += l;
      }
    }
  }

  video_window_set_caption(vw, string_get(cap));
#if defined(MORE_DEBUG)
  if (!vw->if_fullscreen)
    debug_message_fnc("%s\n", string_get(cap));
#endif

  string_destroy(cap);
}

/* action handler */

static int main_loop_quit(void *a) { MainLoop *ml = a; ml->ret = MAIN_LOOP_QUIT; return 1; }
static int main_loop_next(void *a) { MainLoop *ml = a; ml->ret = MAIN_LOOP_NEXT; return 1; }
static int main_loop_next5(void *a) { MainLoop *ml = a; ml->ret = MAIN_LOOP_NEXT5; return 1; }
static int main_loop_nextarchive(void *a) { MainLoop *ml = a; ml->ret = MAIN_LOOP_NEXTARCHIVE; return 1; }
static int main_loop_nextarchive5(void *a) { MainLoop *ml = a; ml->ret = MAIN_LOOP_NEXTARCHIVE5; return 1; }
static int main_loop_prev(void *a) { MainLoop *ml = a; ml->ret = MAIN_LOOP_PREV; return 1; }
static int main_loop_prev5(void *a) { MainLoop *ml = a; ml->ret = MAIN_LOOP_PREV5; return 1; }
static int main_loop_prevarchive(void *a) { MainLoop *ml = a; ml->ret = MAIN_LOOP_PREVARCHIVE; return 1; }
static int main_loop_prevarchive5(void *a) { MainLoop *ml = a; ml->ret = MAIN_LOOP_PREVARCHIVE5; return 1; }
static int main_loop_last(void *a) { MainLoop *ml = a; ml->ret = MAIN_LOOP_LAST; return 1; }
static int main_loop_first(void *a) { MainLoop *ml = a; ml->ret = MAIN_LOOP_FIRST; return 1; }
static int main_loop_delete_file(void *a) { MainLoop *ml = a; ml->ret = MAIN_LOOP_DELETE_FILE; return 1; }
static int main_loop_delete_from_list(void *a) { MainLoop *ml = a; ml->ret = MAIN_LOOP_DELETE_FROM_LIST; return 1;}

static int
main_loop_erase_rectangle(void *a)
{
  MainLoop *ml = a; 
  
  video_window_erase_rect(ml->vw);
  return 1;
}

static int
main_loop_toggle_fullscreen_mode(void *a)
{
  MainLoop *ml = a; 

  video_window_set_fullscreen_mode(ml->vw, _VIDEO_WINDOW_FULLSCREEN_TOGGLE);
  return 1;
}
static int
main_loop_set_wallpaper(void *a)
{
  MainLoop *ml = a; 

  if (ml->p) 
    ml->uidata->vp->set_wallpaper(ml->uidata->disp, ml->p);
  return 1;
}
static int
main_loop_toggle_interpolate(void *a)
{
  MainLoop *ml = a; 

  switch (ml->vw->interpolate_method) {
  case _NOINTERPOLATE:
    ml->vw->interpolate_method = _BILINEAR;
    break;
  case _BILINEAR:
    ml->vw->interpolate_method = _NOINTERPOLATE;
    break;
  default:
    show_message_fnc("invalid interpolate method %d\n", ml->vw->interpolate_method);
    ml->vw->interpolate_method = _NOINTERPOLATE;
    break;
  }
  video_window_set_cursor(ml->vw, _VIDEO_CURSOR_WAIT);

  render(ml);
  set_caption_string(ml);

  video_window_set_cursor(ml->vw, _VIDEO_CURSOR_NORMAL);

  return 1;
}

static int
main_loop_magnify_main(MainLoop *ml)
{
  VideoWindow *vw = ml->vw;
  Image *p = ml->p;
  Movie *m = ml->m;

  if (p) {
    video_window_set_cursor(vw, _VIDEO_CURSOR_WAIT);

    video_window_set_offset(vw, 0, 0);
    render(ml);

    video_window_set_cursor(vw, _VIDEO_CURSOR_NORMAL);
  } else if (m) {
    movie_resize(m);
  }

  return 1;
}
static int
main_loop_magnify_short(void *a)
{
  MainLoop *ml = a;

  ml->vw->render_method = (ml->vw->render_method == _VIDEO_RENDER_NORMAL) ?
    _VIDEO_RENDER_MAGNIFY_SHORT_FULL : _VIDEO_RENDER_NORMAL;
  return main_loop_magnify_main(ml);
}
static int
main_loop_magnify_long(void *a)
{
  MainLoop *ml = a;

  ml->vw->render_method = (ml->vw->render_method == _VIDEO_RENDER_NORMAL) ?
    _VIDEO_RENDER_MAGNIFY_LONG_FULL : _VIDEO_RENDER_NORMAL;
  return main_loop_magnify_main(ml);
}
static int
main_loop_magnify_double(void *a)
{
  MainLoop *ml = a;

  ml->vw->render_method = (ml->vw->render_method == _VIDEO_RENDER_NORMAL) ?
    _VIDEO_RENDER_MAGNIFY_DOUBLE : _VIDEO_RENDER_NORMAL;
  return main_loop_magnify_main(ml);
}

static int
main_loop_save_main(MainLoop *ml, char *format)
{
  int result;

  video_window_set_cursor(ml->vw, _VIDEO_CURSOR_WAIT);

  if (!save_image(ml->p, ml->uidata, ml->path, format)) {
    show_message("save_image() (format %s) failed.\n", format);
    result = 0;
  } else
    result = 1;

  video_window_set_cursor(ml->vw, _VIDEO_CURSOR_NORMAL);

  return result;
}
static int
main_loop_save_png(void *a)
{
  MainLoop *ml = a;

  return main_loop_save_main(ml, (char *)"PNG");
}
static int
main_loop_save(void *a)
{
  MainLoop *ml = a;
  char *format;

  if ((format = config_get(ml->uidata->c, "/enfle/plugins/ui/normal/save_format")) == NULL) {
    show_message("save_format is not specified.\n");
    return 0;
  }

  return main_loop_save_main(ml, format);
}

static UIAction built_in_actions[] = {
  { "go_first", main_loop_first, NULL, ENFLE_KEY_less, ENFLE_MOD_Shift, ENFLE_Button_None },
  { "go_last", main_loop_last, NULL, ENFLE_KEY_greater, ENFLE_MOD_Shift, ENFLE_Button_None },
  { "quit", main_loop_quit, NULL, ENFLE_KEY_q, ENFLE_MOD_None, ENFLE_Button_None },
  { "toggle-fullscreen", main_loop_toggle_fullscreen_mode, NULL, ENFLE_KEY_f, ENFLE_MOD_None, ENFLE_Button_None },
  { "next", main_loop_next, NULL, ENFLE_KEY_n, ENFLE_MOD_None, ENFLE_Button_None },
  { "next", main_loop_next, NULL, ENFLE_KEY_space, ENFLE_MOD_None, ENFLE_Button_None },
  { "next", main_loop_next, NULL, ENFLE_KEY_Empty, ENFLE_MOD_None, ENFLE_Button_1 },
  { "next", main_loop_next, NULL, ENFLE_KEY_Empty, ENFLE_MOD_None, ENFLE_Button_4 },
  { "next5", main_loop_next5, NULL, ENFLE_KEY_n, ENFLE_MOD_Ctrl, ENFLE_Button_None },
  { "next5", main_loop_next5, NULL, ENFLE_KEY_space, ENFLE_MOD_Ctrl, ENFLE_Button_None },
  { "next5", main_loop_next5, NULL, ENFLE_KEY_Empty, ENFLE_MOD_Ctrl, ENFLE_Button_1 },
  { "next5", main_loop_next5, NULL, ENFLE_KEY_Empty, ENFLE_MOD_Ctrl, ENFLE_Button_4 },
  { "next_archive", main_loop_nextarchive, NULL, ENFLE_KEY_n, ENFLE_MOD_Shift, ENFLE_Button_None },
  { "next_archive", main_loop_nextarchive, NULL, ENFLE_KEY_space, ENFLE_MOD_Shift, ENFLE_Button_None },
  { "next_archive", main_loop_nextarchive, NULL, ENFLE_KEY_Empty, ENFLE_MOD_None, ENFLE_Button_2 },
  { "next_archive5", main_loop_nextarchive5, NULL, ENFLE_KEY_n, ENFLE_MOD_Alt, ENFLE_Button_None },
  { "next_archive5", main_loop_nextarchive5, NULL, ENFLE_KEY_space, ENFLE_MOD_Alt, ENFLE_Button_None },
  { "next_archive5", main_loop_nextarchive5, NULL, ENFLE_KEY_Empty, ENFLE_MOD_Alt, ENFLE_Button_1 },
  { "next_archive5", main_loop_nextarchive5, NULL, ENFLE_KEY_Empty, ENFLE_MOD_Alt, ENFLE_Button_4 },
  { "prev", main_loop_prev, NULL, ENFLE_KEY_b, ENFLE_MOD_None, ENFLE_Button_None },
  { "prev", main_loop_prev, NULL, ENFLE_KEY_Empty, ENFLE_MOD_None, ENFLE_Button_3 },
  { "prev", main_loop_prev, NULL, ENFLE_KEY_Empty, ENFLE_MOD_None, ENFLE_Button_5 },
  { "prev5", main_loop_prev5, NULL, ENFLE_KEY_b, ENFLE_MOD_Ctrl, ENFLE_Button_None },
  { "prev5", main_loop_prev5, NULL, ENFLE_KEY_Empty, ENFLE_MOD_Ctrl, ENFLE_Button_3 },
  { "prev5", main_loop_prev5, NULL, ENFLE_KEY_Empty, ENFLE_MOD_Ctrl, ENFLE_Button_5 },
  { "prev_archive", main_loop_prevarchive, NULL, ENFLE_KEY_b, ENFLE_MOD_Shift, ENFLE_Button_None },
  { "prev_archive5", main_loop_prevarchive5, NULL, ENFLE_KEY_b, ENFLE_MOD_Alt, ENFLE_Button_None },
  { "prev_archive5", main_loop_prevarchive5, NULL, ENFLE_KEY_Empty, ENFLE_MOD_Alt, ENFLE_Button_3 },
  { "prev_archive5", main_loop_prevarchive5, NULL, ENFLE_KEY_Empty, ENFLE_MOD_Alt, ENFLE_Button_5 },
  { "delete_file", main_loop_delete_file, NULL, ENFLE_KEY_d, ENFLE_MOD_Shift, ENFLE_Button_None },
  { "delete_from_list", main_loop_delete_from_list, NULL, ENFLE_KEY_d, ENFLE_MOD_None, ENFLE_Button_None },
  { "toggle_interpolate", main_loop_toggle_interpolate, NULL, ENFLE_KEY_s, ENFLE_MOD_Shift, ENFLE_Button_None },
  { "save_png", main_loop_save_png, NULL, ENFLE_KEY_s, ENFLE_MOD_Ctrl, ENFLE_Button_None },
  { "save", main_loop_save, NULL, ENFLE_KEY_s, ENFLE_MOD_Alt, ENFLE_Button_None },
  { "magnify_double", main_loop_magnify_double, NULL, ENFLE_KEY_m, ENFLE_MOD_None, ENFLE_Button_None },
  { "magnify_short",  main_loop_magnify_short, NULL, ENFLE_KEY_m, ENFLE_MOD_Shift, ENFLE_Button_None },
  { "magnify_long",  main_loop_magnify_long, NULL, ENFLE_KEY_m, ENFLE_MOD_Alt, ENFLE_Button_None },
  { "erase_rectangle", main_loop_erase_rectangle, NULL, ENFLE_KEY_c, ENFLE_MOD_None, ENFLE_Button_None },
  { "set_wallpaper", main_loop_set_wallpaper, NULL, ENFLE_KEY_w, ENFLE_MOD_None, ENFLE_Button_None },
  UI_ACTION_END
};

static int
main_loop_button_pressed(MainLoop *ml)
{
  ml->button = ml->ev.button.button;
  switch (ml->button) {
  case ENFLE_Button_1:
  case ENFLE_Button_3:
    ml->pointed_button = ml->button;
    ml->lx = ml->old_x = ml->ev.pointer.x;
    ml->uy = ml->old_y = ml->ev.pointer.y;
    break;
  default:
    break;
  }

  return 1;
}

static int
main_loop_key_pressed(MainLoop *ml)
{
  ml->key = ml->ev.key.key;
  return 1;
}

static int
main_loop_do_action(MainLoop *ml, VideoKey key, VideoModifierKey modkey, VideoButton button)
{
  Hash *actionhash = (Hash *)ml->uidata->priv;
  char buf[16];
  UIAction *uia;
  int result;

  snprintf(buf, 16, "%04X%04X%04X", key, modkey, button);
  if ((uia = hash_lookup_str(actionhash, buf)) == NULL)
    return 0;
  if (uia->group_id == 0) {
    /* Self action */
    //debug_message_fnc("call %s\n", uia->name);
    result = uia->func((void *)ml);
  } else {
    /* Effect Plugin action */
    result = uia->func(uia->arg);
    render(ml);
  }

  return result;
}

static int
main_loop_button_released(MainLoop *ml)
{
  VideoButton button = ml->button;

  ml->button = ENFLE_Button_None;
  if (button == ml->ev.button.button)
    return main_loop_do_action(ml, ENFLE_KEY_Empty, ENFLE_MOD_None, ml->ev.button.button);
  return 0;
}

static int
main_loop_key_released(MainLoop *ml)
{
  VideoKey key = ml->key;

  ml->key = ENFLE_KEY_Empty;
  if (key == ml->ev.key.key)
    return main_loop_do_action(ml, ml->ev.key.key, ml->ev.key.modkey, ENFLE_Button_None);
  return 1;
}

static int
main_loop_pointer_moved(MainLoop *ml)
{
  switch (ml->pointed_button) {
  case ENFLE_Button_1:
    if (!(ml->ev.pointer.button & ENFLE_Button_1))
      break;
    ml->offset_x = ml->ev.pointer.x - ml->old_x;
    ml->offset_y = ml->ev.pointer.y - ml->old_y;
    ml->old_x = ml->ev.pointer.x;
    ml->old_y = ml->ev.pointer.y;
    if (ml->offset_x != 0 || ml->offset_y != 0) {
      ml->button = ENFLE_Button_None;
      video_window_adjust_offset(ml->vw, ml->offset_x, ml->offset_y);
    }
    break;
  case ENFLE_Button_3:
    if (!(ml->ev.pointer.button & ENFLE_Button_3))
      break;
    ml->rx = ml->ev.pointer.x;
    ml->dy = ml->ev.pointer.y;
    video_window_draw_rect(ml->vw, ml->lx, ml->uy, ml->rx, ml->dy);
    ml->button = ENFLE_Button_None;
    break;
  default:
    break;
  }

  return 1;
}

static int
main_loop(UIData *uidata, VideoWindow *vw, Movie *m, Image *p, Stream *st, Archive *a, char *path, void *gui)
{
  MainLoop ml;
  int caption_to_be_set = 0;

  memset(&ml, 0, sizeof(ml));
  ml.button = ENFLE_Button_None;
  ml.key = ENFLE_KEY_Empty;
  ml.uidata = uidata;
  ml.vw = vw;
  ml.m = m;
  ml.p = p;
  ml.timer = enfle_timer_create(timer_gettimeofday());
  ml.st = st;
  ml.a = a;
  ml.path = path;

  if (p) {
    switch (p->type) {
    case _YUY2:
    case _YV12:
    case _I420:
    case _UYVY:
      vw->if_direct = 1;
      break;
    default:
      vw->if_direct = 0;
      break;
    }
    render(&ml);
    set_caption_string(&ml);
    caption_to_be_set = 0;
  } else if (m) {
    vw->if_direct = 1;
    caption_to_be_set = 1;
  }

  video_window_set_offset(vw, 0, 0);
  video_window_set_cursor(vw, _VIDEO_CURSOR_NORMAL);

  timer_start(ml.timer);
  ml.ret = MAIN_LOOP_DO_NOTHING;
  while (ml.ret == MAIN_LOOP_DO_NOTHING) {
    if (timer_is_running(ml.timer) && timer_get(ml.timer) >= 2) {
      /* Pointer is inside window and has been stopped 2seconds */
      video_window_set_cursor(vw, _VIDEO_CURSOR_INVISIBLE);
      timer_stop(ml.timer);
    }
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
	timer_start(ml.timer);
	video_window_set_cursor(vw, _VIDEO_CURSOR_NORMAL);
	break;
      case ENFLE_Event_EnterWindow:
	timer_start(ml.timer);
	break;
      case ENFLE_Event_LeaveWindow:
	timer_stop(ml.timer);
	break;
      default:
	break;
      }
    }
#ifdef ENABLE_GUI_GTK
    {
      GUI_Gtk *gui_gtk = (GUI_Gtk *)gui;

      switch (eventnotifier_get(gui_gtk->en)) {
      case _EVENT_NEXT:
	main_loop_next(&ml);
	break;
      case _EVENT_PREV:
	main_loop_prev(&ml);
	break;
      case _EVENT_NEXT_ARCHIVE:
	main_loop_nextarchive(&ml);
	break;
      case _EVENT_PREV_ARCHIVE:
	main_loop_prevarchive(&ml);
	break;
      default:
	break;
      }
    }
#endif

    if (m) {
      switch (m->status) {
      case _PLAY:
      case _RESIZING:
	if (movie_play_main(m, vw) != PLAY_OK) {
	  show_message_fnc("movie_play_main() failed.\n");
	  ml.ret = MAIN_LOOP_NEXT;
	}
	break;
      case _REWINDING:
	movie_play(m);
	break;
      case _PAUSE:
	break;
      case _STOP:
	ml.ret = MAIN_LOOP_NEXT;
	break;
      case _UNLOADED:
	show_message("Movie has been already unloaded.\n");
	ml.ret = MAIN_LOOP_NEXT;
	break;
      default:
	break;
      }
      if (caption_to_be_set) {
	set_caption_string(&ml);
	caption_to_be_set = 0;
      }
    }
  }

  if (ml.timer)
    timer_destroy(ml.timer);

  return ml.ret;
}

static int process_files_of_archive(UIData *uidata, Archive *a, void *gui);
static int
process_file(UIData *uidata, char *path, Archive *a, Stream *s, Movie *m, void *gui, int prev_ret)
{
  EnflePlugins *eps = get_enfle_plugins();
  VideoWindow *vw = uidata->vw;
  Config *c = uidata->c;
  Image *p;
  Archive *arc;
  char *tmp;
  int r, res, ret = 0;

  if (uidata->cache) {
    char *fullpath = archive_getpathname(a, path);

    if ((p = cache_get_image(uidata->cache, fullpath)) != NULL) {
      debug_message_fnc("Using cached image for %s\n", fullpath);
      free(fullpath);
      s->format = strdup("CACHED");
      return main_loop(uidata, vw, NULL, p, s, a, path, gui);
    }
    free(fullpath);
  }

  video_window_set_cursor(vw, _VIDEO_CURSOR_WAIT);
  r = identify_file(eps, path, s, a, c);
  switch (r) {
  case IDENTIFY_FILE_DIRECTORY:
    arc = archive_create(a);
    debug_message("Reading %s.\n", path);

    if (!archive_read_directory(arc, path, 1)) {
      show_message("Error in reading %s.\n", path);
      archive_destroy(arc);
      return MAIN_LOOP_DELETE_FROM_LIST;
    }
    (prev_ret == MAIN_LOOP_PREV) ? archive_iteration_last(arc) : archive_iteration_first(arc);
    ret = process_files_of_archive(uidata, arc, gui);
    if (arc->nfiles == 0) {
      /* Now that all paths are deleted in this archive, should be deleted wholly. */
      ret = MAIN_LOOP_DELETE_FROM_LIST;
    }
    archive_destroy(arc);
    return ret;
  case IDENTIFY_FILE_STREAM:
    if ((tmp = config_get_str(c, "/enfle/plugins/ui/normal/archiver/disabled")) == NULL ||
	strcasecmp(tmp, "yes") != 0) {
      arc = archive_create(a);
      if (archiver_identify(eps, arc, s, c)) {
	debug_message("Archiver identified as %s\n", arc->format);
	if ((r = archiver_open(eps, arc, arc->format, s)) == OPEN_OK && archive_nfiles(arc) > 0) {
	  debug_message("archiver_open() OK: %d files\n", archive_nfiles(arc));
	  (prev_ret == MAIN_LOOP_PREV) ? archive_iteration_last(arc) : archive_iteration_first(arc);
	  ret = process_files_of_archive(uidata, arc, gui);
	  archive_destroy(arc);
	  return ret;
	} else {
	  if (r == OPEN_OK) {
	    show_message("Archive %s [%s] contains 0 files.\n", arc->format, path);
	  } else {
	    show_message("Archive %s [%s] cannot open\n", arc->format, path);
	  }
	  ret = MAIN_LOOP_DELETE_FROM_LIST;
	  archive_destroy(arc);
	  return ret;
	}
      }
      archive_destroy(arc);
    }
    break;
  case IDENTIFY_FILE_NOTREG:
  case IDENTIFY_FILE_SOPEN_FAILED:
  case IDENTIFY_FILE_AOPEN_FAILED:
  case IDENTIFY_FILE_STAT_FAILED:
  case IDENTIFY_FILE_ZERO_SIZE:
  default:
    return MAIN_LOOP_DELETE_FROM_LIST;
  }

  p = image_create();
  r = identify_stream(eps, p, m, s, vw, c);
  switch (r) {
  case IDENTIFY_STREAM_MOVIE_FAILED:
  case IDENTIFY_STREAM_IMAGE_FAILED:
    show_message("%s load failed\n", path);
    ret = MAIN_LOOP_DELETE_FROM_LIST;
    break;
  case IDENTIFY_STREAM_FAILED:
    show_message("%s identification failed\n", path);
    ret = MAIN_LOOP_DELETE_FROM_LIST_DIR;
    break;
  case IDENTIFY_STREAM_IMAGE:
    debug_message("%s: (%d, %d) %s\n", path, image_width(p), image_height(p), image_type_to_string(p->type));
    if (image_width(p) < uidata->minw || image_height(p) < uidata->minh) {
      debug_message("Too small image.  Skipped.\n");
      ret = MAIN_LOOP_DELETE_FROM_LIST;
      break;
    }

    if (p->comment && config_get_boolean(c, "/enfle/plugins/ui/normal/show_comment", &res)) {
      show_message("comment: %s\n", p->comment);
      free(p->comment);
      p->comment = NULL;
    }

    ret = main_loop(uidata, vw, NULL, p, s, a, path, gui);
    if (uidata->cache) {
      char *fullpath = archive_getpathname(a, path);
      cache_add_image(uidata->cache, p, fullpath);
      free(fullpath);
    }
    goto cached_exit;
  case IDENTIFY_STREAM_MOVIE:
    ret = main_loop(uidata, vw, m, NULL, s, a, path, gui);
    movie_unload(m);
    m->width = m->height = 0;
    break;
  default:
    ret = MAIN_LOOP_DELETE_FROM_LIST;
    break;
  }

  image_destroy(p);
 cached_exit:
  stream_close(s);

  return ret;
}

static int
process_files_of_archive(UIData *uidata, Archive *a, void *gui)
{
  Stream *s;
  Movie *m;
  char *path;
  int i, ret;

  //debug_message_fnc("path = %s\n", a->path);

  s = stream_create();
  m = movie_create();

  m->initialize_screen = initialize_screen;
  m->render_frame = render_frame;
  m->ap = uidata->ap;

  path = NULL;
  if (uidata->nth > 1 && archive_nfiles(a) > 1 && archive_iteration_start(a)) {
    for (i = 0; i < uidata->nth - 2; i++) {
      path = archive_iteration_next(a);
      if (!path)
	break;
    }
    if (i == uidata->nth - 2)
      path = archive_iteration_next(a);
    uidata->nth = 0;
  }

  ret = MAIN_LOOP_DO_NOTHING;
  while (ret != MAIN_LOOP_QUIT) {
    if (path == NULL)
      path = archive_iteration_start(a);
    else {
      switch (ret) {
      case MAIN_LOOP_DELETE_FROM_LIST_DIR:
	//debug_message("MAIN_LOOP_DELETE_FROM_LIST_DIR\n");
	path = archive_iteration_delete(a);
	ret = (archive_direction(a) == 1) ? MAIN_LOOP_NEXT : MAIN_LOOP_PREV;
	break;
      case MAIN_LOOP_DELETE_FROM_LIST:
	//debug_message("MAIN_LOOP_DELETE_FROM_LIST\n");
	path = archive_delete(a, 1);
	ret = MAIN_LOOP_NEXT;
	break;
      case MAIN_LOOP_DELETE_FILE:
	//debug_message("MAIN_LOOP_DELETE_FILE\n");
	if (strcmp(a->format, "NORMAL") == 0) {
	  char *fullpath = archive_getpathname(a, path);

	  if (fullpath) {
	    unlink(fullpath);
	    show_message("DELETED: %s\n", fullpath);
	    free(fullpath);
	  }
	}
	path = archive_delete(a, 1);
	ret = MAIN_LOOP_NEXT;
	break;
      case MAIN_LOOP_NEXT:
	//debug_message("MAIN_LOOP_NEXT: %s -> ", path);
	path = archive_iteration_next(a);
	//debug_message("%s\n", path);
	break;
      case MAIN_LOOP_PREV:
	//debug_message("MAIN_LOOP_PREV: %s -> \n", path);
	path = archive_iteration_prev(a);
	//debug_message("%s\n", path);
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
      case MAIN_LOOP_NEXT5:
	//debug_message("MAIN_LOOP_NEXT5\n");
	for (i = 0; i < 5 && path; i++) {
	  path = archive_iteration_next(a);
	  //debug_message(" skip %s\n", path);
	}
	break;
      case MAIN_LOOP_PREV5:
	//debug_message("MAIN_LOOP_PREV5\n");
	for (i = 0; i < 5 && path; i++) {
	  path = archive_iteration_prev(a);
	  //debug_message(" skip %s\n", path);
	}
	break;
      case MAIN_LOOP_NEXTARCHIVE5:
	//debug_message("MAIN_LOOP_NEXTARCHIVE5\n");
	path = NULL;
	ret = MAIN_LOOP_NEXT5;
	break;
      case MAIN_LOOP_PREVARCHIVE5:
	//debug_message("MAIN_LOOP_PREVARCHIVE5\n");
	path = NULL;
	ret = MAIN_LOOP_PREV5;
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
	show_message("*** %s() returned unknown code %d\n", __FUNCTION__, ret);
	path = NULL;
	break;
      }
    }

    if (!path)
      break;

    ret = process_file(uidata, path, a, s, m, gui, ret);
  }

  movie_destroy(m);
  stream_destroy(s);

  return ret;
}

static int
action_register(Hash *actionhash, UIAction *actions, int group_id)
{
  char buf[16];
  int id = 0;

  if (!actions)
    return 1;

  while (actions->name) {
    snprintf(buf, 16, "%04X%04X%04X", actions->key, actions->mod, actions->button);
    actions->group_id = group_id;
    actions->id = id++;
    if (!hash_set_str_value(actionhash, buf, actions))
      return 0;
    actions++;
  }

  return 1;
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
  Hash *actionhash;
  int r, group_id = 0;
#ifdef ENABLE_GUI_GTK
  GUI_Gtk gui;
  pthread_t gui_thread;
#endif

  debug_message("Normal: %s()\n", __FUNCTION__);

  /* Must be prime */
  if ((actionhash = hash_create(8209)) == NULL)
    return 0;

  if ((disp = vp->open_video(NULL, c)) == NULL) {
    show_message("open_video() failed\n");
    return 0;
  }
  uidata->disp = disp;

  uidata->vw = vw = vp->open_window(disp, vp->get_root(disp), 600, 400);

  video_window_set_event_mask(vw,
			      ENFLE_ExposureMask | ENFLE_ButtonMask |
			      ENFLE_KeyMask | ENFLE_PointerMask | ENFLE_WindowMask);

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

  action_register(actionhash, built_in_actions, group_id++);
  uidata->priv = actionhash;

  {
    EnflePlugins *eps = get_enfle_plugins();
    Dlist *dl = enfle_plugins_get_names(eps, ENFLE_PLUGIN_EFFECT);
    Dlist_data *dd;
    Plugin *pl;
    EffectPlugin *ep;
    char *pname;

    if (dl) {
      dlist_iter(dl, dd) {
	pname = hash_key_key(dlist_data(dd));
	if ((pl = pluginlist_get(eps->pls[ENFLE_PLUGIN_EFFECT], pname)) == NULL)
	  continue;
	ep = plugin_get(pl);
	debug_message("Register %s actions\n", pname);
	action_register(actionhash, ep->actions, group_id++);
      }
    }
  }

  uidata->cache = NULL;
  if (!config_get_boolean(c, "/enfle/plugins/ui/normal/disable_image_cache", &r)) {
    int cache_max;
    cache_max = config_get_int(c, "/enfle/plugins/ui/normal/image_cache_max", &r);
    if (!r || cache_max == 0)
      cache_max = 4;

    uidata->cache = cache_create(cache_max);
  }

#ifdef ENABLE_GUI_GTK
  gui.en = eventnotifier_create();
  gui.uidata = uidata;
  pthread_create(&gui_thread, NULL, gui_gtk_init, (void *)&gui);
  process_files_of_archive(uidata, uidata->a, (void *)&gui);
#else
  process_files_of_archive(uidata, uidata->a, NULL);
#endif

  cache_destroy(uidata->cache);
  hash_destroy(actionhash);
  video_window_destroy(vw);
  vp->close_video(disp);

  return 1;
}
