/*
 * mng.c -- preliminary mng driver for enfle, using libmng
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Thu Dec 14 20:56:02 2000.
 * $Id: mng.c,v 1.12 2000/12/14 16:06:14 sian Exp $
 *
 * Note: mng implementation is far from complete.
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

#include <sys/time.h>
#include <signal.h>
#include <libmng.h>

#include "common.h"

#include "stream.h"
#include "player-plugin.h"

typedef struct {
  mng_handle mng;
  VideoWindow *vw;
  Movie *m;
  Image *p;
  int rc;
  unsigned int delay;
} MNG_info;

static const unsigned types = (IMAGE_ARGB32 | IMAGE_BGRA32);

static PlayerStatus identify(Movie *, Stream *);
static PlayerStatus load(VideoWindow *, Movie *, Stream *);

static PlayerStatus pause_movie(Movie *);
static PlayerStatus stop_movie(Movie *);

static PlayerPlugin plugin = {
  type: ENFLE_PLUGIN_PLAYER,
  name: "MNG",
  description: "MNG Player plugin version 0.1.2",
  author: "Hiroshi Takekawa",

  identify: identify,
  load: load
};

void *
plugin_entry(void)
{
  PlayerPlugin *pp;

  if ((pp = (PlayerPlugin *)calloc(1, sizeof(PlayerPlugin))) == NULL)
    return NULL;
  memcpy(pp, &plugin, sizeof(PlayerPlugin));

  return (void *)pp;
}

void
plugin_exit(void *p)
{
  free(p);
}

/* libmng callbacks */

static mng_ptr
memalloc(mng_size_t n)
{
  return calloc(1, n);
}

static void
memfree(mng_ptr p, mng_size_t n)
{
  free(p);
}

static mng_bool
openstream(mng_handle mng)
{
  return MNG_TRUE;
}

static mng_bool
closestream(mng_handle mng)
{
  return MNG_TRUE;
}

static mng_bool
readdata(mng_handle mng, mng_ptr p, mng_uint32 len, mng_uint32 *readsize)
{
  MNG_info *this = (MNG_info *)mng_get_userdata(mng);

  if (stream_read(this->m->st, p, len) != len) {
    *readsize = 0;
    return MNG_FALSE;
  }

  *readsize = len;
  return MNG_TRUE;
}

static mng_bool
processheader(mng_handle mng, mng_uint32 width, mng_uint32 height)
{
  MNG_info *this = (MNG_info *)mng_get_userdata(mng);
  Movie *m = this->m;
  Image *p;

  m->width  = width;
  m->height = height;
  m->rendering_width  = m->width;
  m->rendering_height = m->height;

  p = this->p = image_create();

  p->width  = m->rendering_width;
  p->height = m->rendering_height;
  p->type = m->requested_type;
  switch (p->type) {
  case _ARGB32:
    mng_set_canvasstyle(mng, MNG_CANVAS_ARGB8);
    break;
  case _BGRA32:
    mng_set_canvasstyle(mng, MNG_CANVAS_BGRA8);
    break;
  default:
    show_message(__FUNCTION__": requested type is %s.\n", image_type_to_string(p->type));
    return PLAY_ERROR;
  }

  p->bits_per_pixel = 32;
  p->depth = 24;
  p->bytes_per_line = p->width * (p->bits_per_pixel >> 3);
  if (m->direct_decode) {
    p->rendered.image = memory_create();
    memory_request_type(p->rendered.image, video_window_preferred_memory_type(this->vw));
    if (memory_alloc(p->rendered.image, p->bytes_per_line * p->height) == NULL)
      return PLAY_ERROR;
  } else {
    p->rendered.image = memory_create();
    memory_request_type(p->rendered.image, video_window_preferred_memory_type(this->vw));
    p->image = memory_create();
    if (memory_alloc(p->image, p->bytes_per_line * p->height) == NULL)
      return PLAY_ERROR;
  }

  m->initialize_screen(this->vw, m, m->width, m->height);

  return MNG_TRUE;
}

static mng_ptr
getcanvasline(mng_handle mng, mng_uint32 nthline)
{
  MNG_info *this = (MNG_info *)mng_get_userdata(mng);
  Image *p;
  unsigned char *d;

  p = this->p;
  d = memory_ptr(this->m->direct_decode ? p->rendered.image : p->image);

  return (mng_ptr)&d[p->bytes_per_line * nthline];
}

static mng_bool
refresh(mng_handle mng, mng_uint32 l, mng_uint32 t, mng_uint32 w, mng_uint32 h)
{
  MNG_info *this = (MNG_info *)mng_get_userdata(mng);

  /* debug_message(__FUNCTION__ ": (%d,%d)-(%d,%d)\n", l, t, l + w - 1, t + h - 1); */

  this->m->current_frame++;
  this->m->render_frame(this->vw, this->m, this->p);

  return MNG_TRUE;
}

static mng_uint32
gettickcount(mng_handle mng)
{
  struct timeval tv;

  gettimeofday(&tv, NULL);

  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static mng_bool
settimer(mng_handle mng, mng_uint32 msec)
{
  MNG_info *this = (MNG_info *)mng_get_userdata(mng);

  this->delay = msec;

  return MNG_TRUE;
}

static mng_bool
errorproc(mng_handle mng, mng_int32 errcode, mng_int8 severity,
	  mng_chunkid chunkname, mng_uint32 chunkseq, mng_int32 extra1,
	  mng_int32 extra2, mng_pchar errtext)
{
  unsigned int c = chunkname;

  fprintf(stderr, "mng_errorproc: [%c%c%c%c]: errcode %d: %s\n", c >> 24, c >> 16, c >> 8, c & 0xff, errcode, errtext);

  return MNG_TRUE;
}

/* end of libmng callbacks */

/* for internal use */

static void *
get_screen(Movie *m)
{
  MNG_info *this;

  if (m->movie_private) {
    this = (MNG_info *)m->movie_private;
    return getcanvasline(this->mng, 0);
  }

  return NULL;
}

static PlayerStatus
play(Movie *m)
{
  switch (m->status) {
  case _PLAY:
    return PLAY_OK;
  case _PAUSE:
    return pause_movie(m);
  case _STOP:
    m->status = _PLAY;
    return PLAY_OK;
  case _UNLOADED:
    return PLAY_ERROR;
  }

  return PLAY_ERROR;
}

static PlayerStatus
play_main(Movie *m, VideoWindow *vw)
{
  MNG_info *this = m->movie_private;

  switch (m->status) {
  case _PLAY:
    break;
  case _PAUSE:
  case _STOP:
    return PLAY_OK;
  case _UNLOADED:
    return PLAY_ERROR;
  default:
    return PLAY_ERROR;
  }

  switch (this->rc) {
  case MNG_IMAGEFROZEN:
  case MNG_NOERROR:
    break;
  case MNG_NEEDTIMERWAIT:
    m->pause_usec(this->delay * 1000);
    break;
  default:
    show_message("MNG: " __FUNCTION__ ": Error %d\n", this->rc);
    return PLAY_ERROR;
  }
  this->rc = mng_display_resume(this->mng);

  return PLAY_OK;
}

static PlayerStatus
pause_movie(Movie *m)
{
  switch (m->status) {
  case _PLAY:
    m->status = _PAUSE;
    return PLAY_OK;
  case _PAUSE:
    m->status = _PLAY;
    return PLAY_OK;
  case _STOP:
    return PLAY_OK;
  case _UNLOADED:
    return PLAY_ERROR;
  }

  return PLAY_ERROR;
}

static PlayerStatus
stop_movie(Movie *m)
{
  MNG_info *this = m->movie_private;

  switch (m->status) {
  case _PLAY:
    m->status = _STOP;
    break;
  case _PAUSE:
  case _STOP:
    return PLAY_OK;
  case _UNLOADED:
    return PLAY_ERROR;
  default:
    return PLAY_ERROR;
  }

  /* stop */
  stream_rewind(m->st);
  mng_display_reset(this->mng);

  m->current_frame = 0;

  return PLAY_OK;
}

static PlayerStatus
load_movie(VideoWindow *vw, Movie *m, Stream *st)
{
  MNG_info *this;
  int err = 0;

  if ((this = calloc(1, sizeof(MNG_info))) == NULL) {
    show_message("MNG: " __FUNCTION__ ": No enough memory.\n");
    return PLAY_ERROR;
  }

  m->requested_type = video_window_request_type(vw, types, &m->direct_decode);
  debug_message("MNG: requested type: %s %s\n", image_type_to_string(m->requested_type), m->direct_decode ? "direct" : "not direct");

  m->st = st;
  m->movie_private = (void *)this;
  m->status = _PLAY;
  m->current_frame = 0;

  this->vw = vw;
  this->m = m;

  this->mng = mng_initialize((mng_ptr)this, memalloc, memfree, NULL);
  if (mng_setcb_openstream   (this->mng, openstream   )) err++;
  if (mng_setcb_closestream  (this->mng, closestream  )) err++;
  if (mng_setcb_readdata     (this->mng, readdata     )) err++;
  if (mng_setcb_processheader(this->mng, processheader)) err++;
  if (mng_setcb_getcanvasline(this->mng, getcanvasline)) err++;
  if (mng_setcb_refresh      (this->mng, refresh      )) err++;
  if (mng_setcb_gettickcount (this->mng, gettickcount )) err++;
  if (mng_setcb_settimer     (this->mng, settimer     )) err++;
  if (mng_setcb_errorproc    (this->mng, errorproc    )) err++;
  if (err) {
    show_message("MNG: failed to install %d callback function(s)\n", err);
    return PLAY_ERROR;
  }

  this->rc = mng_readdisplay(this->mng);

  return PLAY_OK;
}

static void
unload_movie(Movie *m)
{
  MNG_info *this = m->movie_private;

  mng_cleanup(&this->mng);
  if (this->p)
    image_destroy(this->p);
  free(this);
}

/* methods */

static unsigned char mng_sig[] = { 0x8a, 'M', 'N', 'G' };

static PlayerStatus
identify(Movie *m, Stream *st)
{
  unsigned char buf[4];

  if (stream_read(st, buf, 4) != 4)
    return PLAY_NOT;
  if (memcmp(mng_sig, buf, 4))
    return PLAY_NOT;

  return PLAY_OK;
}

static PlayerStatus
load(VideoWindow *vw, Movie *m, Stream *st)
{
  debug_message("mng player: load() called\n");

#ifdef IDENTIFY_BEFORE_PLAY
  {
    PlayerStatus status;

    if ((status = identify(m, st)) != PLAY_OK)
      return status;
    stream_rewind(st);
  }
#endif

  m->get_screen = get_screen;
  m->play = play;
  m->play_main = play_main;
  m->pause_movie = pause_movie;
  m->stop = stop_movie;
  m->unload_movie = unload_movie;

  return load_movie(vw, m, st);
}
