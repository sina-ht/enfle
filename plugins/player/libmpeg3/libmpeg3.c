/*
 * libmpeg3.c -- libmpeg3 player plugin, which exploits libmpeg3.
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Dec  3 17:11:48 2000.
 * $Id: libmpeg3.c,v 1.7 2000/12/03 08:40:04 sian Exp $
 *
 * NOTES: 
 *  This plugin is not fully enfle plugin compatible, because stream
 *  cannot be used for input. Stream's path is used as input filename.
 *
 *  Requires libmpeg3 1.2.1 (or later).
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
#include <mpeg3/libmpeg3.h>

#include "common.h"

#include "timer.h"
#include "timer_gettimeofday.h"
#include "player-plugin.h"

typedef struct _libmpeg3_info {
  mpeg3_t *file;
  int nstreams;
  int nstream;
  unsigned char **lines;
  Memory *buffer;
} LibMPEG3_info;

static PlayerStatus identify(Movie *, Stream *);
static PlayerStatus load(VideoWindow *, Movie *, Stream *);

static PlayerStatus pause_movie(Movie *);
static PlayerStatus stop_movie(Movie *);

static PlayerPlugin plugin = {
  type: ENFLE_PLUGIN_PLAYER,
  name: "LibMPEG3",
  description: "LibMPEG3 Player plugin version 0.2",
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

/* for internal use */

static PlayerStatus
load_movie(VideoWindow *vw, Movie *m, Stream *st)
{
  LibMPEG3_info *info;
  int i, Bpp;

  if ((info = calloc(1, sizeof(LibMPEG3_info))) == NULL) {
    show_message("LibMPEG3: play_movie: No enough memory.\n");
    return PLAY_ERROR;
  }

  if ((info->file = mpeg3_open(st->path)) == NULL) {
    free(info);
    return PLAY_ERROR;
  }

  /* can set how many CPUs you want to use. */
  /* mpeg3_set_cpus(info->file, 2); */
  /* mpeg3_set_mmx(info->file, 1); */

  if (!mpeg3_has_video(info->file)) {
    show_message("This stream is audio only. Not supported(so far).\n");
    goto error;
  }

  if (mpeg3_has_audio(info->file)) {
    show_message("Audio support is not yet implemented, might be implemented someday.\n");
  }

  if ((info->nstreams = mpeg3_total_vstreams(info->file)) > 1) {
    show_message("There are %d video streams in this whole stream.\n",
		 info->nstreams);
    show_message("Only the first video stream will be played(so far). Sorry.\n");
  }
  info->nstream = 0;

  m->width = mpeg3_video_width(info->file, info->nstream);
  m->height = mpeg3_video_height(info->file, info->nstream);
  m->framerate = mpeg3_frame_rate(info->file, 0);
  m->num_of_frames = mpeg3_video_frames(info->file, 0);

  debug_message("libmpeg3 player: (%d x %d) %f fps %d frames\n",
		m->width, m->height, m->framerate, m->num_of_frames);

  /* rewind stream */
  mpeg3_seek_percentage(info->file, 0);

  if ((info->buffer = memory_create()) == NULL)
    goto error;
  memory_request_type(info->buffer, video_window_preferred_memory_type(vw));

  debug_message(__FUNCTION__ ": requested memory type %s\n", memory_type(info->buffer) == _NORMAL ? "NORMAL" : "SHM");

  if ((info->lines = calloc(m->height, sizeof(unsigned char *))) == NULL)
    goto error;

  Bpp = vw->bits_per_pixel >> 3;
  /* extra 4 bytes are needed for MMX routine */
  if (memory_alloc(info->buffer, m->width * m->height * Bpp + 4) == NULL)
    goto error;

  for (i = 0; i < m->height; i++)
    info->lines[i] = memory_ptr(info->buffer) + i * m->width * Bpp;

  m->movie_private = (void *)info;
  m->st = st;
  m->status = _PLAY;
  m->previous_frame = 0;
  m->current_frame = 0;

  m->initialize_screen(vw, m, m->width, m->height);

  timer_start(m->timer);

  return PLAY_OK;

 error:
  free(info);
  return PLAY_ERROR;
}

static void *
get_screen(Movie *m)
{
  LibMPEG3_info *info;

  if (m->movie_private) {
    info = (LibMPEG3_info *)m->movie_private;
    return memory_ptr(info->buffer);
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
    m->current_frame = 0;
    m->previous_frame = 0;
    timer_start(m->timer);
    return PLAY_OK;
  case _UNLOADED:
    return PLAY_ERROR;
  }

  return PLAY_ERROR;
}

static PlayerStatus
play_main(Movie *m, VideoWindow *vw)
{
  int decode_error;
  LibMPEG3_info *info = (LibMPEG3_info *)m->movie_private;
  Image *p;
  int rendering_type, dropframes;
  float time_elapsed, fps;

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

  p = image_create();

  p->width  = m->width;
  p->height = m->height;

  switch (vw->bits_per_pixel) {
  case 32:
    rendering_type = MPEG3_RGBA8888;
    p->type = _RGBA32;
    p->depth = 24;
    p->bytes_per_line = m->width * 4;
    p->bits_per_pixel = 32;
    break;
  case 24:
    rendering_type = MPEG3_RGB888;
    p->type = _RGB24;
    p->depth = 24;
    p->bytes_per_line = m->width * 3;
    p->bits_per_pixel = 24;
    break;
  case 16:
    rendering_type = MPEG3_RGB565;
    p->depth = 16;
    p->type = _BGR_WITH_BITMASK;
    p->bytes_per_line = m->width * 2;
    p->bits_per_pixel = 16;
    break;
  default:
    show_message("Cannot render bpp %d\n", vw->bits_per_pixel);
    rendering_type = MPEG3_RGBA8888;
    break;
  }

  decode_error = (mpeg3_read_frame(info->file, info->lines,
				   0, 0,
				   m->width, m->height,
				   m->width, m->height,
				   rendering_type, info->nstream) == -1) ? 0 : 1;

  p->next = NULL;
  memory_destroy(p->image);
  p->image = info->buffer;

  m->current_frame++;

  timer_pause(m->timer);
  time_elapsed = timer_get_milli(m->timer);
  timer_restart(m->timer);
  fps = m->current_frame * 1000 / time_elapsed;

  debug_message("%3.2f fps\r", fps);

  m->render_frame(vw, m, p);
  m->previous_frame = m->current_frame;

  if (fps <= m->framerate) {
    dropframes = m->framerate * time_elapsed / 1000 - m->current_frame;
    if (dropframes && !movie_get_play_every_frame(m)) {
      mpeg3_drop_frames(info->file, dropframes, info->nstream);
      m->current_frame += dropframes;

      debug_message("\ndropped %d frame\n", dropframes);
    }
  }

  p->image = NULL;
  image_destroy(p);

  if (!decode_error || m->current_frame >= m->num_of_frames)
    stop_movie(m);

  return PLAY_OK;
}

static PlayerStatus
pause_movie(Movie *m)
{
  switch (m->status) {
  case _PLAY:
    m->status = _PAUSE;
    timer_pause(m->timer);
    return PLAY_OK;
  case _PAUSE:
    m->status = _PLAY;
    timer_restart(m->timer);
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
  LibMPEG3_info *info = (LibMPEG3_info *)m->movie_private;

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

  mpeg3_seek_percentage(info->file, 0);
  timer_stop(m->timer);

  return PLAY_OK;
}

static void
unload_movie(Movie *m)
{
  LibMPEG3_info *info = (LibMPEG3_info *)m->movie_private;

  if (info) {
    if (info->buffer)
      memory_destroy(info->buffer);
    if (info->lines)
      free(info->lines);

    /* close */
    mpeg3_close(info->file);

    free(info);
  }
}

/* methods */

static PlayerStatus
identify(Movie *m, Stream *st)
{
#if 0
  char buf[4];
  static unsigned char video_stream_header[]  = { 0x00, 0x00, 0x01, 0xb3 };
  static unsigned char system_stream_header[] = { 0x00, 0x00, 0x01, 0xba };

  if (stream_read(st, buf, 4) != 4)
    return PLAY_NOT;

  if (memcmp(buf, video_stream_header, 4) == 0)
    return PLAY_OK;
  if (memcmp(buf, system_stream_header, 4) == 0)
    return PLAY_OK;

  return PLAY_NOT;
#else
  if (st->path)
    return mpeg3_check_sig(st->path) ? PLAY_OK : PLAY_NOT;
  return PLAY_NOT;
#endif
}

static PlayerStatus
load(VideoWindow *vw, Movie *m, Stream *st)
{
  debug_message("libmpeg3 player: load() called\n");

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
