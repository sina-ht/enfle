/*
 * avcodec.c -- avcodec player plugin, which exploits libavcodec
 * (C)Copyright 2000-2003 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Nov 30 14:46:54 2003.
 * $Id: avcodec.c,v 1.2 2003/11/30 05:51:23 sian Exp $
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

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#ifndef USE_PTHREAD
#  error pthread is mandatory for avcodec plugin
#endif

#include <pthread.h>

#include "enfle/player-plugin.h"
#include "demultiplexer/demultiplexer_avi.h"
#include "avcodec/avcodec.h"
#include "mpglib/mpg123.h"
#include "mpglib/mpglib.h"
#include "utils/libstring.h"
#include "enfle/fourcc.h"

typedef struct _divx_info {
  Config *c;
  Demultiplexer *demux;
  Image *p;
  AudioDevice *ad;
  FIFO *vstream, *astream;
  struct mpstr mp;
  AVCodec *vcodec;
  enum CodecID vcodec_id;
  const char *vcodec_name;
  AVCodecContext *vcodec_ctx;
  AVFrame *vcodec_picture;
  AVCodec *acodec;
  AVCodecContext *acodec_ctx;
  enum CodecID acodec_id;
  const char *acodec_name;
  int use_xv;
  int eof;
  int drop;
  pthread_mutex_t update_mutex;
  pthread_cond_t update_cond;
  int nvstreams;
  int nvstream;
  pthread_t video_thread;
  int nastreams;
  int nastream;
  pthread_t audio_thread;
} avcodec_info;

//IMAGE_UYVY | IMAGE_YUY2 |
static const unsigned int types =
  (IMAGE_I420 | 
   IMAGE_BGRA32 | IMAGE_BGR24 |
   IMAGE_ARGB32 | IMAGE_RGB24 |
   IMAGE_BGR_WITH_BITMASK | IMAGE_RGB_WITH_BITMASK);

DECLARE_PLAYER_PLUGIN_METHODS;

static PlayerStatus play(Movie *);
static PlayerStatus pause_movie(Movie *);
static PlayerStatus stop_movie(Movie *);

#define PLAYER_AVCODEC_PLUGIN_DESCRIPTION "avcodec Player plugin version 0.1"

static PlayerPlugin plugin = {
  type: ENFLE_PLUGIN_PLAYER,
  name: "avcodec",
  description: NULL,
  author: "Hiroshi Takekawa",
  identify: identify,
  load: load
};

ENFLE_PLUGIN_ENTRY(player_divx)
{
  PlayerPlugin *pp;
  String *s;

  if ((pp = (PlayerPlugin *)calloc(1, sizeof(PlayerPlugin))) == NULL)
    return NULL;
  memcpy(pp, &plugin, sizeof(PlayerPlugin));
  s = string_create();
  string_set(s, (const char *)PLAYER_AVCODEC_PLUGIN_DESCRIPTION);
  string_catf(s, (const char *)" with " LIBAVCODEC_IDENT);
  pp->description = (const unsigned char *)strdup((const char *)string_get(s));
  string_destroy(s);

  /* avcodec initialization */
  avcodec_init();
  avcodec_register_all();

  return (void *)pp;
}

ENFLE_PLUGIN_EXIT(player_divx, p)
{
  PlayerPlugin *pp = p;

  if (pp->description)
    free((void *)pp->description);
  free(p);
}

/* for internal use */

static PlayerStatus
load_movie(VideoWindow *vw, Movie *m, Stream *st, Config *c)
{
  avcodec_info *info = (avcodec_info *)m->movie_private;
  AVIInfo *aviinfo;
  Image *p;

  if (!info) {
    if ((info = calloc(1, sizeof(avcodec_info))) == NULL) {
      err_message("avcodec: %s: No enough memory.\n", __FUNCTION__);
      return PLAY_ERROR;
    }
    m->movie_private = (void *)info;
  }

  info->c = c;

  pthread_mutex_init(&info->update_mutex, NULL);
  pthread_cond_init(&info->update_cond, NULL);

  m->requested_type = video_window_request_type(vw, types, &m->direct_decode);
  if (!m->direct_decode) {
    err_message_fnc("Cannot direct decoding...\n");
    return PLAY_ERROR;
  }
  debug_message("avcodec: requested type: %s direct\n", image_type_to_string(m->requested_type));

  if (info->demux) {
    info->demux = demultiplexer_avi_create();
    demultiplexer_avi_set_input(info->demux, st);
    if (!demultiplexer_examine(info->demux))
      return PLAY_NOT;
    info->nvstreams = demultiplexer_avi_nvideos(info->demux);
    info->nastreams = demultiplexer_avi_naudios(info->demux);
  }
  aviinfo = demultiplexer_avi_info(info->demux);

  if (info->nastreams == 0 && info->nvstreams == 0)
    return PLAY_NOT;
  if (info->nvstreams != 1)
    return PLAY_NOT;

  m->has_audio = 0;
  if (info->nastreams > 0) {
    if (m->ap == NULL)
      warning("Audio not played.\n");
    else {
      /* XXX: stream should be selectable */
      info->nastream = 1;
      demultiplexer_avi_set_audio(info->demux, info->nastream);

      m->sampleformat = _AUDIO_FORMAT_S16_LE;
      m->channels = aviinfo->nchannels;
      m->samplerate = aviinfo->samples_per_sec;
      m->num_of_samples = aviinfo->num_of_samples;

      show_message("audio[%s%08X](%d streams): format(%d): %d ch rate %d kHz %d samples\n",
		   aviinfo->ahandler == 0x55 ? "mp3:" : "",
		   aviinfo->ahandler, info->nastreams,
		   m->sampleformat, m->channels, m->samplerate, m->num_of_samples);
      if (m->ap->bytes_written == NULL)
	warning("audio sync may be incorrect.\n");
      m->has_audio = 1;
    }
  } else {
    debug_message("No audio streams.\n");
  }

  if (info->nvstreams) {
    m->has_video = 1;
    info->nvstream = 0;
    demultiplexer_avi_set_video(info->demux, info->nvstream);

    m->width = aviinfo->width;
    m->height = aviinfo->height;
    m->framerate = aviinfo->framerate;
    m->num_of_frames = aviinfo->num_of_frames;

    /* Select codec */
    switch (aviinfo->vhandler) {
    case FCC_H263:
      info->vcodec_id = CODEC_ID_H263; info->vcodec_name = "h263"; break;
    case FCC_I263:
      info->vcodec_id = CODEC_ID_H263I; info->vcodec_name = "h263i"; break;
    case FCC_U263:
    case FCC_viv1:
      info->vcodec_id = CODEC_ID_H263P; info->vcodec_name = "h263p"; break;
    case FCC_DIVX: // invalid_asf
    case FCC_DX50: // invalid_asf
    case FCC_XVID: // invalid_asf
    case FCC_MP4S:
    case FCC_M4S2:
    case FCC_0x04000000:
    case FCC_DIV1:
    case FCC_BLZ0:
    case FCC_mp4v:
    case FCC_UMP4:
      info->vcodec_id = CODEC_ID_MPEG4; info->vcodec_name = "mpeg4"; break;
    case FCC_DIV3: // invalid_asf
    case FCC_DIV4:
    case FCC_DIV5:
    case FCC_DIV6:
    case FCC_MP43:
    case FCC_MPG3:
    case FCC_AP41:
    case FCC_COL1:
    case FCC_COL0:
      info->vcodec_id = CODEC_ID_MSMPEG4V3; info->vcodec_name = "msmpeg4"; break;
    case FCC_MP42:
    case FCC_DIV2:
      info->vcodec_id = CODEC_ID_MSMPEG4V2; info->vcodec_name = "msmpeg4v2"; break;
    case FCC_MP41:
    case FCC_MPG4:
      info->vcodec_id = CODEC_ID_MSMPEG4V1; info->vcodec_name = "msmpeg4v1"; break;
    case FCC_WMV1:
      info->vcodec_id = CODEC_ID_WMV1; info->vcodec_name = "wmv1"; break;
    case FCC_WMV2:
      info->vcodec_id = CODEC_ID_WMV2; info->vcodec_name = "wmv2"; break;
    case FCC_dvsd:
    case FCC_dvhd:
    case FCC_dvsl:
    case FCC_dv25:
      info->vcodec_id = CODEC_ID_DVVIDEO; info->vcodec_name = "dvvideo"; break;
    case FCC_mpg1:
    case FCC_mpg2:
    case FCC_PIM1:
    case FCC_VCR2:
      info->vcodec_id = CODEC_ID_MPEG1VIDEO; info->vcodec_name = "mpeg1video"; break;
    case FCC_MJPG:
      info->vcodec_id = CODEC_ID_MJPEG; info->vcodec_name = "mjpeg"; break;
    case FCC_JPGL:
    case FCC_LJPG:
      info->vcodec_id = CODEC_ID_LJPEG; info->vcodec_name = "ljpeg"; break;
    case FCC_HFYU:
      info->vcodec_id = CODEC_ID_HUFFYUV; info->vcodec_name = "huffyuv"; break;
    case FCC_CYUV:
      info->vcodec_id = CODEC_ID_CYUV; info->vcodec_name = "cyuv"; break;
    case FCC_Y422:
    case FCC_I420:
      info->vcodec_id = CODEC_ID_RAWVIDEO; info->vcodec_name = "rawvideo"; break;
    case FCC_IV31:
    case FCC_IV32:
      info->vcodec_id = CODEC_ID_INDEO3; info->vcodec_name = "indeo3"; break;
    case FCC_VP31:
      info->vcodec_id = CODEC_ID_VP3; info->vcodec_name = "vp3"; break;
    case FCC_ASV1:
      info->vcodec_id = CODEC_ID_ASV1; info->vcodec_name = "asv1"; break;
    case FCC_ASV2:
      info->vcodec_id = CODEC_ID_ASV2; info->vcodec_name = "asv2"; break;
    case FCC_VCR1:
      info->vcodec_id = CODEC_ID_VCR1; info->vcodec_name = "vcr1"; break;
    case FCC_FFV1:
      info->vcodec_id = CODEC_ID_FFV1; info->vcodec_name = "ffv1"; break;
    case FCC_Xxan:
      info->vcodec_id = CODEC_ID_XAN_WC4; info->vcodec_name = "xan_wc4"; break;
    case FCC_mrle:
    case FCC_0x01000000:
      info->vcodec_id = CODEC_ID_MSRLE; info->vcodec_name = "msrle"; break;
    case FCC_cvid:
      info->vcodec_id = CODEC_ID_CINEPAK; info->vcodec_name = "cinepak"; break;
    case FCC_MSVC:
    case FCC_msvc:
    case FCC_CRAM:
    case FCC_cram:
    case FCC_WHAM:
    case FCC_wham:
      info->vcodec_id = CODEC_ID_MSVIDEO1; info->vcodec_name = "msvideo1"; break;
    default:
      demultiplexer_destroy(info->demux);
      free(info);
      m->movie_private = NULL;
      return PLAY_NOT;
    }
    if ((info->vcodec = avcodec_find_decoder(info->vcodec_id)) == NULL) {
      show_message("avcodec %s not found\n", info->vcodec_name);
      demultiplexer_destroy(info->demux);
      free(info);
      m->movie_private = NULL;
      return PLAY_NOT;
    }
  } else {
    m->width = 128;
    m->height = 128;
    m->num_of_frames = 1;
  }

  p = info->p = image_create();
  image_width(p) = m->width;
  image_height(p) = m->height;

  switch (m->requested_type) {
  case _I420:
    p->bits_per_pixel = 12;
    image_bpl(p) = image_width(p) * 3 / 2;
    info->use_xv = 1;
    break;
  case _UYVY:
    p->bits_per_pixel = 16;
    image_bpl(p) = image_width(p) << 1;
    info->use_xv = 1;
    break;
  case _YUY2:
    p->bits_per_pixel = 16;
    image_bpl(p) = image_width(p) << 1;
    info->use_xv = 1;
    break;
  default:
    info->use_xv = 0;
    switch (vw->bits_per_pixel) {
    case 32:
      p->depth = 24;
      p->bits_per_pixel = 32;
      image_bpl(p) = image_width(p) * 4;
      break;
    case 24:
      p->depth = 24;
      p->bits_per_pixel = 24;
      image_bpl(p) = image_width(p) * 3;
      break;
#if 0
    case 16:
      p->depth = 16;
      p->bits_per_pixel = 16;
      image_bpl(p) = image_width(p) * 2;
      break;
    case 15:
      p->depth = 15;
      p->bits_per_pixel = 16;
      image_bpl(p) = image_width(p) * 2;
      break;
#endif
    default:
      err_message("Cannot render bpp %d\n", vw->bits_per_pixel);
      return PLAY_ERROR;
    }
  }

  video_window_calc_magnified_size(vw, info->use_xv, m->width, m->height, &m->rendering_width, &m->rendering_height);
  if (info->use_xv) {
    image_rendered_width(p) = m->width;
    image_rendered_height(p) = m->height;
  } else {
    image_rendered_width(p) = m->rendering_width;
    image_rendered_height(p) = m->rendering_height;
  }

  show_message("video[%c%c%c%c(%08X):codec avcodec %s](%d streams): (%d,%d) -> (%d,%d), %f fps %d frames\n",
	       aviinfo->vhandler        & 0xff,
	      (aviinfo->vhandler >>  8) & 0xff,
	      (aviinfo->vhandler >> 16) & 0xff,
	      (aviinfo->vhandler >> 24) & 0xff,
	       aviinfo->vhandler,
	       info->vcodec_name, info->nvstreams,
	       m->width, m->height, m->rendering_width, m->rendering_height,
	       m->framerate, m->num_of_frames);

  p->type = m->requested_type;
  if ((image_rendered_image(p) = memory_create()) == NULL)
    goto error;
  memory_request_type(image_rendered_image(p), video_window_preferred_memory_type(vw));

  if (memory_alloc(image_rendered_image(p), image_bpl(p) * image_height(p)) == NULL)
    goto error;

  m->st = st;
  m->status = _STOP;

  m->initialize_screen(vw, m, m->rendering_width, m->rendering_height);

  return play(m);

 error:
  if (info->demux) {
    demultiplexer_destroy(info->demux);
    info->demux = NULL;
  }
  free(info);
  m->movie_private = NULL;
  return PLAY_ERROR;
}

static int
get_buffer(AVCodecContext *vcodec_ctx, AVFrame *vcodec_picture)
{
  avcodec_info *info = (avcodec_info *)vcodec_ctx->opaque;
  int width, height;
        
  /* alignment */
  width  = (vcodec_ctx->width  + 15) & ~15;
  height = (vcodec_ctx->height + 15) & ~15;

  if (vcodec_ctx->pix_fmt != PIX_FMT_YUV420P ||
      width != vcodec_ctx->width || height != vcodec_ctx->height) {
    debug_message_fnc("avcodec: unsupported frame format, DR1 disabled.\n");
    info->vcodec_ctx->get_buffer = avcodec_default_get_buffer;
    info->vcodec_ctx->release_buffer = avcodec_default_release_buffer;
    return avcodec_default_get_buffer(vcodec_ctx, vcodec_picture);
  }

  vcodec_picture->data[0] = memory_ptr(image_rendered_image(info->p));
  vcodec_picture->data[1] = vcodec_picture->data[0] + image_width(info->p) * image_height(info->p);
  vcodec_picture->data[2] = vcodec_picture->data[1] + (image_width(info->p) >> 1) * (image_height(info->p) >> 1);

  vcodec_picture->linesize[0] = image_width(info->p);
  vcodec_picture->linesize[1] = image_width(info->p) >> 1;
  vcodec_picture->linesize[2] = image_width(info->p) >> 1;

  /*
   * We should keep track of the ages of frames (see
   * avcodec_default_get_buffer in lib/avcodec/utils.c) For the moment
   * tell ffmpeg that every frame is new (age = bignumber)
   */
  vcodec_picture->age = 256 * 256 * 256 * 64;

  vcodec_picture->type = FF_BUFFER_TYPE_USER;

  return 0;
}

static void
release_buffer(AVCodecContext *vcodec_ctx, AVFrame *vcodec_picture){
  vcodec_picture->data[0] = NULL;
  vcodec_picture->data[1] = NULL;
  vcodec_picture->data[2] = NULL;
}

static void *play_video(void *);
static void *play_audio(void *);

static PlayerStatus
play(Movie *m)
{
  avcodec_info *info = (avcodec_info *)m->movie_private;

  debug_message_fn("()\n");

  switch (m->status) {
  case _PLAY:
  case _RESIZING:
    return PLAY_OK;
  case _PAUSE:
    return pause_movie(m);
  case _STOP:
    m->status = _PLAY;
    break;
  case _UNLOADED:
    return PLAY_ERROR;
  default:
    warning("Unknown status %d\n", m->status);
    return PLAY_ERROR;
  }

  m->current_frame = 0;
  m->current_sample = 0;
  timer_start(m->timer);
  demultiplexer_set_eof(info->demux, 0);
  demultiplexer_rewind(info->demux);

  if (m->has_video) {
    if ((info->vstream = fifo_create()) == NULL)
      return PLAY_ERROR;
    demultiplexer_avi_set_vst(info->demux, info->vstream);

    /* XXX: decoder initialize */
    info->vcodec_ctx = avcodec_alloc_context();
    info->vcodec_ctx->pix_fmt = -1;
    info->vcodec_ctx->opaque = info;
    info->vcodec_picture = avcodec_alloc_frame();
    if (info->vcodec->capabilities & CODEC_CAP_TRUNCATED)
      info->vcodec_ctx->flags |= CODEC_FLAG_TRUNCATED;
    if (avcodec_open(info->vcodec_ctx, info->vcodec) < 0) {
      show_message("avcodec_open() failed.\n");
      free(info->vcodec_ctx);
      free(info->vcodec_picture);
      return PLAY_ERROR;
    }
    if (info->vcodec_ctx->pix_fmt == PIX_FMT_YUV420P &&
	info->vcodec->capabilities & CODEC_CAP_DR1) {
      info->vcodec_ctx->get_buffer = get_buffer;
      info->vcodec_ctx->release_buffer = release_buffer;
      show_message("DR1 direct rendering enabled.\n");
#if 1
      // XXX: direct rendering causes seg. fault...
      info->vcodec_ctx->get_buffer = avcodec_default_get_buffer;
      info->vcodec_ctx->release_buffer = avcodec_default_release_buffer;
      show_message("DR1 direct rendering disabled.\n");
#endif
    }

    pthread_create(&info->video_thread, NULL, play_video, m);
  }

  if (m->has_audio) {
    m->has_audio = 1;
    if ((info->astream = fifo_create()) == NULL)
      return PLAY_ERROR;
    demultiplexer_avi_set_ast(info->demux, info->astream);
    InitMP3(&info->mp);
    pthread_create(&info->audio_thread, NULL, play_audio, m);
  }

  demultiplexer_start(info->demux);

  return PLAY_OK;
}

static void *
play_video(void *arg)
{
  Movie *m = arg;
  avcodec_info *info = (avcodec_info *)m->movie_private;
  void *data;
  AVIPacket *ap;
  FIFO_destructor destructor;
  int offset, size, len, got_picture;

  debug_message_fn("()\n");

  while (m->status == _PLAY) {
    if (m->current_frame >= m->num_of_frames) {
      info->eof = 1;
      break;
    }
    
    if (!fifo_get(info->vstream, &data, &destructor)) {
      debug_message_fnc("fifo_get() failed.\n");
      break;
    }

    if ((ap = (AVIPacket *)data) == NULL || ap->data == NULL) {
      info->eof = 1;
      break;
    }
    /* XXX: decode */
    offset = 0;
    size = ap->size;
    while (m->status == _PLAY && size > 0) {
      len = avcodec_decode_video(info->vcodec_ctx, info->vcodec_picture, &got_picture,
				 ap->data + offset, size);
      if (len < 0) {
	warning_fnc("avcodec: avcodec_decode_video return %d\n", len);
	break;
      }

      size -= len;
      offset += len;

      if (got_picture) {
	m->current_frame++;
	if (info->drop > 0) {
	  info->drop--;
	} else {
	  pthread_mutex_lock(&info->update_mutex);
	  if (info->vcodec_ctx->get_buffer != get_buffer) {
	    int y;

	    for (y = 0; y < m->height; y++) {
	      memcpy(memory_ptr(image_rendered_image(info->p)) + m->width * y, info->vcodec_picture->data[0] + info->vcodec_picture->linesize[0] * y, m->width);
	    }
	    for (y = 0; y < m->height >> 1; y++) {
	      memcpy(memory_ptr(image_rendered_image(info->p)) + (m->width >> 1) * y + m->width * m->height, info->vcodec_picture->data[1] + info->vcodec_picture->linesize[1] * y, m->width >> 1);
	    }
	    for (y = 0; y < m->height >> 1; y++) {
	      memcpy(memory_ptr(image_rendered_image(info->p)) + (m->width >> 1) * y + m->width * m->height + (m->width >> 1) * (m->height >> 1), info->vcodec_picture->data[2] + info->vcodec_picture->linesize[2] * y, m->width >> 1);
	    }
	  }
	  pthread_cond_wait(&info->update_cond, &info->update_mutex);
	  pthread_mutex_unlock(&info->update_mutex);
	}
      }
    }
    destructor(ap);
  }

  debug_message_fn(" exiting.\n");

  return (void *)PLAY_OK;
}

#define MP3_DECODE_BUFFER_SIZE 16384

static void *
play_audio(void *arg)
{
  Movie *m = arg;
  avcodec_info *info = (avcodec_info *)m->movie_private;
  AudioDevice *ad;
  char output_buffer[MP3_DECODE_BUFFER_SIZE];
  int ret, write_size;
  int param_is_set = 0;
  void *data;
  AVIPacket *ap;
  FIFO_destructor destructor;

  debug_message_fn("()\n");

  if ((ad = m->ap->open_device(NULL, info->c)) == NULL) {
    err_message("Cannot open device. Audio disabled.\n");
  }
  info->ad = ad;

  while (m->status == _PLAY) {
    if (!fifo_get(info->astream, &data, &destructor)) {
      debug_message_fnc("fifo_get() failed.\n");
      break;
    }
    ap = (AVIPacket *)data;
    if (ad) {
      ret = decodeMP3(&info->mp, ap->data, ap->size,
		      output_buffer, MP3_DECODE_BUFFER_SIZE, &write_size);
      if (!param_is_set) {
	m->sampleformat = _AUDIO_FORMAT_S16_LE;
	m->channels = info->mp.fr.stereo;
	m->samplerate = freqs[info->mp.fr.sampling_frequency];
	m->sampleformat_actual = m->sampleformat;
	m->channels_actual = m->channels;
	m->samplerate_actual = m->samplerate;
	if (!m->ap->set_params(ad, &m->sampleformat_actual, &m->channels_actual, &m->samplerate_actual))
	  err_message("Some params are set wrong.\n");
	param_is_set++;
      }
      while (ret == MP3_OK) {
	m->ap->write_device(ad, (unsigned char *)output_buffer, write_size);
	ret = decodeMP3(&info->mp, NULL, 0,
			output_buffer, MP3_DECODE_BUFFER_SIZE, &write_size);
      }
    }
    destructor(ap);
  }

  if (ad) {
    m->ap->sync_device(ad);
    m->ap->close_device(ad);
    info->ad = NULL;
  }

  ExitMP3(&info->mp);

  debug_message_fn(" exiting.\n");

  return (void *)PLAY_OK;
}

static int
get_audio_time(Movie *m, AudioDevice *ad)
{
  if (ad && m->ap->bytes_written)
    return (int)((double)m->ap->bytes_written(ad) / m->samplerate * 500 / m->channels);
  return (int)((double)m->current_sample * 1000 / m->samplerate);
}

static PlayerStatus
play_main(Movie *m, VideoWindow *vw)
{
  avcodec_info *info = (avcodec_info *)m->movie_private;
  Image *p = info->p;
  int video_time, audio_time;
  int i = 0;

  switch (m->status) {
  case _PLAY:
    break;
  case _RESIZING:
    video_window_resize(vw, m->rendering_width, m->rendering_height);
    video_window_calc_magnified_size(vw, info->use_xv, m->width, m->height, &m->rendering_width, &m->rendering_height);

    if (info->use_xv) {
      image_rendered_width(p) = m->width;
      image_rendered_height(p) = m->height;
    } else {
      image_rendered_width(p) = m->rendering_width;
      image_rendered_height(p) = m->rendering_height;
    }
    m->status = _PLAY;
    break;
  case _PAUSE:
  case _STOP:
    return PLAY_OK;
  case _UNLOADED:
    return PLAY_ERROR;
  default:
    warning("Unknown status %d\n", m->status);
    return PLAY_ERROR;
  }

  if (info->eof) {
    stop_movie(m);
    return PLAY_OK;
  }

  if (!m->has_video)
    return PLAY_OK;

  if (demultiplexer_get_eof(info->demux)) {
    if (info->astream && fifo_is_empty(info->astream))
      /* Audio existed, but over. */
      m->has_audio = 2;
    if ((!info->vstream || fifo_is_empty(info->vstream)) &&
	(!info->astream || fifo_is_empty(info->astream))) {
      stop_movie(m);
      return PLAY_OK;
    }
  }

  pthread_mutex_lock(&info->update_mutex);

  video_time = m->current_frame * 1000 / m->framerate;
  if (m->has_audio == 1 && info->ad) {
    audio_time = get_audio_time(m, info->ad);
    //debug_message("%d frames(v: %d a: %d)\n", i, video_time, audio_time);

    /* if too fast to display, wait before render */
    while (video_time > audio_time)
      audio_time = get_audio_time(m, info->ad);

    /* skip if delayed */
    i = (get_audio_time(m, info->ad) * m->framerate / 1000) - m->current_frame - 1;
    if (i > 0) {
      debug_message("dropped %d frames(v: %d a: %d)\n", i, video_time, audio_time);
      info->drop = i;
    }
  } else {
    /* if too fast to display, wait before render */
    while (video_time > timer_get_milli(m->timer)) ;

    /* skip if delayed */
#if 0
    i = (timer_get_milli(m->timer) * m->framerate / 1000) - m->current_frame - 1;
    if (i > 0) {
      debug_message("dropped %d frames(v: %d t: %d)\n", i, video_time, (int)timer_get_milli(m->timer));
      info->drop = i;
    }
#endif
  }

  /* tell video thread to continue decoding */
  pthread_cond_signal(&info->update_cond);
  pthread_mutex_unlock(&info->update_mutex);

  m->render_frame(vw, m, p);

  return PLAY_OK;
}

static PlayerStatus
pause_movie(Movie *m)
{
  switch (m->status) {
  case _PLAY:
  case _RESIZING:
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
  default:
    warning("Unknown status %d\n", m->status);
    return PLAY_ERROR;
  }

  return PLAY_ERROR;
}

static PlayerStatus
stop_movie(Movie *m)
{
  avcodec_info *info = (avcodec_info *)m->movie_private;
  void *v, *a;

  debug_message_fn("()\n");

  switch (m->status) {
  case _PLAY:
  case _RESIZING:
    m->status = _STOP;
    timer_stop(m->timer);
    break;
  case _PAUSE:
  case _STOP:
    return PLAY_OK;
  case _UNLOADED:
    return PLAY_ERROR;
  default:
    warning("Unknown status %d\n", m->status);
    return PLAY_ERROR;
  }

  if (info->demux)
    demultiplexer_stop(info->demux);
  if (info->vstream)
    fifo_destroy(info->vstream);
  if (info->astream)
    fifo_destroy(info->astream);

  pthread_mutex_lock(&info->update_mutex);
  pthread_cond_signal(&info->update_cond);
  pthread_mutex_unlock(&info->update_mutex);
  if (info->video_thread) {
    pthread_join(info->video_thread, &v);
    info->video_thread = 0;
  }

  if (info->audio_thread) {
    pthread_join(info->audio_thread, &a);
    info->audio_thread = 0;
  }

  /* XXX: decoder clean up */
  if (info->vcodec_ctx) {
    avcodec_close(info->vcodec_ctx);
    free(info->vcodec_ctx);
  }
  if (info->vcodec_picture)
    free(info->vcodec_picture);

  debug_message_fn("() OK\n");

  return PLAY_OK;
}

static void
unload_movie(Movie *m)
{
  avcodec_info *info = (avcodec_info *)m->movie_private;

  debug_message_fn("()\n");

  stop_movie(m);

  if (info) {
    if (info->p)
      image_destroy(info->p);
    if (info->demux)
      demultiplexer_destroy(info->demux);
    pthread_mutex_destroy(&info->update_mutex);
    pthread_cond_destroy(&info->update_cond);
    debug_message_fnc("freeing info\n");
    free(info);
    m->movie_private = NULL;
    debug_message_fnc("freed info\n");
  }

  debug_message_fn("() Ok\n");
}

/* methods */

DEFINE_PLAYER_PLUGIN_IDENTIFY(m, st, c, priv)
{
  avcodec_info *info = (avcodec_info *)m->movie_private;
  AVIInfo *aviinfo;

  if (!info) {
    if ((info = calloc(1, sizeof(avcodec_info))) == NULL) {
      err_message("avcodec: %s: No enough memory.\n", __FUNCTION__);
      return PLAY_ERROR;
    }
    m->movie_private = (void *)info;
  }

  info->demux = demultiplexer_avi_create();
  demultiplexer_avi_set_input(info->demux, st);
  if (!demultiplexer_examine(info->demux)) {
    demultiplexer_destroy(info->demux);
    free(info);
    m->movie_private = NULL;
    return PLAY_NOT;
  }
  info->nvstreams = demultiplexer_avi_nvideos(info->demux);
  info->nastreams = demultiplexer_avi_naudios(info->demux);
  aviinfo = demultiplexer_avi_info(info->demux);

  if (info->nvstreams) {
    switch (aviinfo->vhandler) {
    case FCC_H263:
    case FCC_I263:
    case FCC_U263:
    case FCC_viv1:
    case FCC_DIVX:
    case FCC_DX50:
    case FCC_XVID:
    case FCC_MP4S:
    case FCC_M4S2:
    case FCC_0x04000000:
    case FCC_DIV1:
    case FCC_BLZ0:
    case FCC_mp4v:
    case FCC_UMP4:
    case FCC_DIV3:
    case FCC_DIV4:
    case FCC_DIV5:
    case FCC_DIV6:
    case FCC_MP43:
    case FCC_MPG3:
    case FCC_AP41:
    case FCC_COL1:
    case FCC_COL0:
    case FCC_MP42:
    case FCC_DIV2:
    case FCC_MP41:
    case FCC_MPG4:
    case FCC_WMV1:
    case FCC_WMV2:
    case FCC_dvsd:
    case FCC_dvhd:
    case FCC_dvsl:
    case FCC_dv25:
    case FCC_mpg1:
    case FCC_mpg2:
    case FCC_PIM1:
    case FCC_VCR2:
    case FCC_MJPG:
    case FCC_JPGL:
    case FCC_LJPG:
    case FCC_HFYU:
    case FCC_CYUV:
    case FCC_Y422:
    case FCC_I420:
    case FCC_IV31:
    case FCC_IV32:
    case FCC_VP31:
    case FCC_ASV1:
    case FCC_ASV2:
    case FCC_VCR1:
    case FCC_FFV1:
    case FCC_Xxan:
    case FCC_mrle:
    case FCC_0x01000000:
    case FCC_cvid:
    case FCC_MSVC:
    case FCC_msvc:
    case FCC_CRAM:
    case FCC_cram:
    case FCC_WHAM:
    case FCC_wham:
      break;
    default:
	debug_message_fnc("Not identified as any avcodec supported format: %08X\n", aviinfo->vhandler);
	demultiplexer_destroy(info->demux);
      free(info);
      m->movie_private = NULL;
      return PLAY_NOT;
    }
  }

  return PLAY_OK;
}

DEFINE_PLAYER_PLUGIN_LOAD(vw, m, st, c, priv)
{
  debug_message("avcodec: %s()\n", __FUNCTION__);

#ifdef IDENTIFY_BEFORE_PLAY
  {
    PlayerStatus status;

    if ((status = identify(m, st, c, priv)) != PLAY_OK)
      return status;
    stream_rewind(st);
  }
#endif

  m->play = play;
  m->play_main = play_main;
  m->pause_movie = pause_movie;
  m->stop = stop_movie;
  m->unload_movie = unload_movie;

  return load_movie(vw, m, st, c);
}
