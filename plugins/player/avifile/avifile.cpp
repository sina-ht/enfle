/*
 * avifile.cpp -- avifile player plugin, which exploits avifile.
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Jan  7 01:19:31 2001.
 * $Id: avifile.cpp,v 1.1 2001/01/06 23:50:08 sian Exp $
 *
 * NOTES: 
 *  This plugin is not fully enfle plugin compatible, because stream
 *  cannot be used for input. Stream's path is used as input filename.
 *
 *  Requires avifile.
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
#include <errno.h>

#include <aviplay.h>

#include "aviplay_impl.h"

#include "common.h"

extern "C" {
#include "enfle/memory.h"
#include "enfle/player-plugin.h"
}

class AviFileLoader {
  static void stopfunc(int);
  static void drawfunc(CImage *);
  static int audiofunc(void *, int);

  /* XXX: hack */
  static Image *setimage(Image *p) { static Image *_p = NULL; if (!_p) _p = p; return _p; }
  static int store_updated(int f) { static int _f = 0; if (f == -1) return _f; _f = f; return _f; }

public:
  PlayerStatus load_movie(VideoWindow *, Movie *, Stream *);
  AviFileLoader();
  ~AviFileLoader();
  static Image *getimage() { return setimage(NULL); }
  static int set_updated() { int dummy = store_updated(1); }
  static int unset_updated() { int dummy = store_updated(0); }
  static int is_updated() { return store_updated(-1); }

  AviPlayer *player;
  AudioDevice *ad;
  WAVEFORMATEX owf;
};

typedef struct _avifile_info {
  AviFileLoader *afl;
  int rendering_type;
#ifdef USE_PTHREAD
  pthread_mutex_t render_mutex;
#endif
} AviFile_info;

static const unsigned int types =
  (IMAGE_RGBA32 | IMAGE_BGRA32 | IMAGE_RGB24 | IMAGE_BGR24 | IMAGE_BGR_WITH_BITMASK);

extern "C" {
static PlayerStatus identify(Movie *, Stream *);
static PlayerStatus load(VideoWindow *, Movie *, Stream *);
}

static PlayerStatus play(Movie *);
static PlayerStatus pause_movie(Movie *);
static PlayerStatus stop_movie(Movie *);

static PlayerPlugin plugin = {
  type: ENFLE_PLUGIN_PLAYER,
  name: "AviFile",
  description: (const unsigned char *)"AviFile Player plugin version 0.1",
  author: (const unsigned char *)"Hiroshi Takekawa",
  identify: identify,
  load: load
};

extern "C" {
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
}

/* for internal use */

AviFileLoader::AviFileLoader()
{
  debug_message("AviFileLoader constructor\n");
}

AviFileLoader::~AviFileLoader()
{
  debug_message("AviFileLoader destructor\n");
}

void
AviFileLoader::stopfunc(int value)
{
  debug_message("AviFileLoader::stopfunc(%d)\n", value);
}

void
AviFileLoader::drawfunc(CImage *ci)
{
  Image *p = getimage();

  //debug_message("drawfunc(%p): %p, (%d, %d), bpl %d\n", ci, ci->data(), ci->width(), ci->height(), ci->bpl());

  memory_set(p->rendered.image, ci->data(), _NORMAL, ci->bpl() * ci->height(), ci->bpl() * ci->height());
  set_updated();
}

int
AviFileLoader::audiofunc(void *data, int size)
{
  debug_message("AviFileLoader::audiofunc(%p, %d)\n", data, size);
  return size;
}

PlayerStatus
AviFileLoader::load_movie(VideoWindow *vw, Movie *m, Stream *st)
{
  AviFile_info *info = (AviFile_info *)m->movie_private;
  IAviReadStream *audiostream;
  Image *p;

#if 0
  IAviReadFile *rf;
  MainAVIHeader hdr;

  if (!rf)
    return PLAY_ERROR;
  if (!rf->StreamCount() || rf->GetFileHeader(&hdr)) {
    delete rf;
    return PLAY_ERROR;
  }
  m->num_of_frames = hdr.dwTotalFrames;
  delete rf;
#endif

  player = new AviPlayer();
  player->setKillHandler(&stopfunc);
  player->setDrawCallback2(&drawfunc);
  player->setAudioFunc(&audiofunc);

  m->requested_type = video_window_request_type(vw, types, &m->direct_decode);
  if (!m->direct_decode) {
    show_message("AviFile: load_movie: Cannot direct decoding...\n");
    return PLAY_ERROR;
  }
  debug_message("AviFile: requested type: %s direct\n", image_type_to_string(m->requested_type));

  player->initPlayer(st->path, vw->bits_per_pixel, NULL);

  if ((m->ap == NULL) || (ad = m->ap->open_device(NULL, m->c)) == NULL)
    show_message("Audio not played.\n");
  else if ((audiostream = player->audiostream)) {
    audiostream->GetOutputFormat(&owf, sizeof owf);
    m->sampleformat = _AUDIO_FORMAT_S16_LE;
    m->channels = owf.nChannels;
    m->samplerate = owf.nSamplesPerSec;
    //m->num_of_samples = owf.nSamplesPerSec * len;

    if (!m->ap->set_params(ad, &m->sampleformat, &m->channels, &m->samplerate))
      show_message("Some params are set wrong.\n");

    show_message("audio: format(%d, %d bits per sample): %d ch rate %d kHz\n", m->sampleformat, owf.wBitsPerSample, m->channels, m->samplerate);
  }

  m->width = player->width();
  m->height = player->height();
  m->framerate = player->fps();

#if 1
  m->rendering_width = m->width;
  m->rendering_height = m->height;
#else
  switch (vw->render_method) {
  case _VIDEO_RENDER_NORMAL:
    m->rendering_width  = m->width;
    m->rendering_height = m->height;
    break;
  case _VIDEO_RENDER_MAGNIFY_DOUBLE:
    m->rendering_width  = m->width  << 1;
    m->rendering_height = m->height << 1;
    break;
  default:
    m->rendering_width  = m->width;
    m->rendering_height = m->height;
    break;
  }
#endif

  debug_message("avifile player: (%d,%d) -> (%d,%d) %f fps %d frames\n",
		m->width, m->height, m->rendering_width, m->rendering_height,
		m->framerate, m->num_of_frames);

  p = image_create();
  p->width = m->rendering_width;
  p->height = m->rendering_height;
  p->type = m->requested_type;
  if ((p->rendered.image = memory_create()) == NULL)
    goto error;
  memory_request_type(p->rendered.image, video_window_preferred_memory_type(vw));

  switch (vw->bits_per_pixel) {
  case 32:
    p->depth = 24;
    p->bits_per_pixel = 32;
    p->bytes_per_line = p->width * 4;
    break;
  case 24:
    p->depth = 24;
    p->bits_per_pixel = 24;
    p->bytes_per_line = p->width * 3;
    break;
  case 16:
    p->depth = 16;
    p->bits_per_pixel = 16;
    p->bytes_per_line = p->width * 2;
    break;
  default:
    show_message("Cannot render bpp %d\n", vw->bits_per_pixel);
    return PLAY_ERROR;
  }

  m->st = st;

  setimage(p);

  m->initialize_screen(vw, m, m->rendering_width, m->rendering_height);

  play(m);

  return PLAY_OK;

 error:
  free(info);
  return PLAY_ERROR;
}

static void *
get_screen(Movie *m)
{
  AviFile_info *info;

  if (m->movie_private) {
    info = (AviFile_info *)m->movie_private;
    return memory_ptr(info->afl->getimage()->rendered.image);
  }

  return NULL;
}

static PlayerStatus
play(Movie *m)
{
  AviFile_info *info = (AviFile_info *)m->movie_private;
  AviFileLoader *afl = info->afl;
  AviPlayer *player = afl->player;

  switch (m->status) {
  case _PLAY:
    return PLAY_OK;
  case _PAUSE:
    return pause_movie(m);
  case _STOP:
    m->status = _PLAY;
    break;
  case _UNLOADED:
    return PLAY_ERROR;
  default:
    return PLAY_ERROR;
  }

  player->reseek(0);
  player->start();

  return PLAY_OK;
}

static PlayerStatus
play_main(Movie *m, VideoWindow *vw)
{
  AviFile_info *info = (AviFile_info *)m->movie_private;
  AviFileLoader *afl = info->afl;
  AviPlayer *player = afl->player;

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

  if (player->isStopped()) {
    m->status = _STOP;
    return PLAY_OK;
  }

  if (afl->is_updated()) {
#ifdef USE_PTHREAD
    if (pthread_mutex_trylock(&info->render_mutex) == EBUSY)
      return PLAY_OK;
#endif
    m->render_frame(vw, m, afl->getimage());
    afl->unset_updated();
#ifdef USE_PTHREAD
    pthread_mutex_unlock(&info->render_mutex);
#endif
  }

  return PLAY_OK;
}

static PlayerStatus
pause_movie(Movie *m)
{
  AviFile_info *info = (AviFile_info *)m->movie_private;
  AviFileLoader *afl = info->afl;
  AviPlayer *player = afl->player;

  switch (m->status) {
  case _PLAY:
    player->pause(1);
    m->status = _PAUSE;
    return PLAY_OK;
  case _PAUSE:
    player->play();
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
  AviFile_info *info = (AviFile_info *)m->movie_private;
  AviFileLoader *afl = info->afl;
  AviPlayer *player = afl->player;

  switch (m->status) {
  case _PLAY:
    player->stop();
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

  return PLAY_OK;
}

static void
unload_movie(Movie *m)
{
  AviFile_info *info = (AviFile_info *)m->movie_private;
  AviFileLoader *afl = info->afl;

  if (info) {
    AviFileLoader *afl = info->afl;

    if (afl->getimage())
      image_destroy(afl->getimage());

    if (info->afl) {
      AviPlayer *player = afl->player;

      if (afl->ad)
	m->ap->close_device(afl->ad);

      player->endPlayer();
      delete player;
    }
#ifdef USE_PTHREAD
    pthread_mutex_destroy(&info->render_mutex);
#endif
    free(info);
  }
}

/* methods */

extern "C" {

static PlayerStatus
identify(Movie *m, Stream *st)
{
  unsigned char buf[16];

  if (stream_read(st, buf, 16) != 16)
    return PLAY_NOT;

  if (memcmp(buf, "RIFF", 4))
    return PLAY_NOT;
  if (memcmp(buf + 8, "AVI ", 4))
    return PLAY_NOT;

  return PLAY_OK;
}

static PlayerStatus
load(VideoWindow *vw, Movie *m, Stream *st)
{
  AviFile_info *info;

  debug_message("avifile player: load() called\n");

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

  if ((info = (AviFile_info *)calloc(1, sizeof(AviFile_info))) == NULL) {
    show_message("AviFile: load: No enough memory.\n");
    return PLAY_ERROR;
  }
  m->movie_private = (void *)info;

  info->afl = new AviFileLoader();
#ifdef USE_PTHREAD
  pthread_mutex_init(&info->render_mutex, NULL);
#endif

  return info->afl->load_movie(vw, m, st);
}
}
