/*
 * libmpeg2_vo.c -- video output routine for libmpeg2 player plugin
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Jan 10 00:54:51 2004.
 * $Id: libmpeg2_vo.c,v 1.1 2004/01/09 15:58:31 sian Exp $
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <inttypes.h>

#include "compat.h"
#include "common.h"

#include "enfle/image.h"
#include "enfle/video.h"
#include "enfle/movie.h"
#include "libmpeg2_vo.h"

/* RGB */

static void
enfle_draw(vo_frame_t *_frame)
{
  enfle_frame_t *frame = (enfle_frame_t *)_frame;
  enfle_instance_t *instance = (enfle_instance_t *)frame->vo.instance;
  Movie *m = instance->m;
  Image *p = instance->p;
  Libmpeg2_info *info = (Libmpeg2_info *)m->movie_private;

  if (m->status == _PLAY) {
    if ((int)m->framerate == 0 && info->mpeg2dec.frame_rate > 0) {
      m->framerate = info->mpeg2dec.frame_rate;
      show_message("(%d, %d) fps %2.5f\n", m->width, m->height, m->framerate);
    }

    pthread_mutex_lock(&info->update_mutex);
    memcpy(memory_ptr(image_rendered_image(p)), frame->rgb_ptr_base, instance->image_size);
    info->to_render++;
    m->current_frame++;
    while (m->status == _PLAY && info->to_render > 0)
      pthread_cond_wait(&info->update_cond, &info->update_mutex);
    pthread_mutex_unlock(&info->update_mutex);
  }
}

static int
enfle_alloc_frames(vo_instance_t *_instance, int width, int height, int frame_size,
		   void (*copy)(vo_frame_t *, uint8_t **),
		   void (*field)(vo_frame_t *, int),
		   void (*draw)(vo_frame_t *))
{
  enfle_instance_t *instance;
  int size;
  uint8_t *alloc;
  int i;

  debug_message_fn("(%d, %d)\n", width, height);

  instance = (enfle_instance_t *)_instance;
  instance->prediction_index = 1;
  size = width * height >> 2;
  if ((alloc = memalign(ATTRIBUTE_ALIGNED_MAX, 18 * size)) == NULL) {
    debug_message_fnc("memalign() ERROR\n");
    return 1;
  } else {
    debug_message_fnc("memalign(%d, %d) = %p\n", ATTRIBUTE_ALIGNED_MAX, 18 * size, alloc);
  }

  for (i = 0; i < 3; i++) {
    instance->frame_ptr[i] = (vo_frame_t *)(instance->frame + i);
    instance->frame[i].rgb_ptr_base = malloc(instance->image_size);
    instance->frame[i].vo.base[0] = alloc;
    instance->frame[i].vo.base[1] = alloc + 4 * size;
    instance->frame[i].vo.base[2] = alloc + 5 * size;
    instance->frame[i].vo.copy = copy;
    instance->frame[i].vo.field = field;
    instance->frame[i].vo.draw = draw;
    instance->frame[i].vo.instance = (vo_instance_t *)instance;
    alloc += 6 * size;
  }

  debug_message_fnc("OK\n");

  return 0;
}

static void
enfle_free_frames (vo_instance_t *_instance)
{
  enfle_instance_t *instance;
  int i;

  debug_message_fn("()\n");

  instance = (enfle_instance_t *)_instance;
  if (instance->frame[0].vo.base[0]) {
    debug_message_fnc("freeing %p\n", instance->frame[0].vo.base[0]);
    free(instance->frame[0].vo.base[0]);
  }
  for (i = 0; i < 3; i++) {
    if (instance->frame[i].rgb_ptr_base) {
      debug_message_fnc("%d:rgb_ptr_base: freeing %p\n", i, instance->frame[i].rgb_ptr_base);
      free(instance->frame[i].rgb_ptr_base);
    }
  }
  debug_message_fnc("OK\n");
}

static vo_frame_t *
enfle_get_frame(vo_instance_t *_instance, int flags)
{
  enfle_instance_t *instance;

  instance = (enfle_instance_t *)_instance;
  if (flags & VO_PREDICTION_FLAG) {
    instance->prediction_index ^= 1;
    return instance->frame_ptr[instance->prediction_index];
  } else
    return instance->frame_ptr[2];
}

static vo_frame_t *
rgb_get_frame (vo_instance_t *_instance, int flags)
{
  enfle_instance_t *instance;
  enfle_frame_t *frame;

  instance = (enfle_instance_t *)_instance;
  frame = (enfle_frame_t *)enfle_get_frame((vo_instance_t *)instance, flags);

  //debug_message_fn("(%p: rgb_stride %d yuv_stride %d)\n",
  //	frame->rgb_ptr_base, instance->rgbstride, instance->width);

  frame->rgb_ptr    = frame->rgb_ptr_base;
  frame->rgb_stride = instance->rgbstride;
  frame->yuv_stride = instance->width;

  if ((flags & VO_TOP_FIELD) == 0)
    frame->rgb_ptr += frame->rgb_stride;
  if ((flags & VO_BOTH_FIELDS) != VO_BOTH_FIELDS) {
    frame->rgb_stride <<= 1;
    frame->yuv_stride <<= 1;
  }

  return (vo_frame_t *)frame;
}

static void
rgb_copy_slice(vo_frame_t *_frame, uint8_t **src)
{
  enfle_frame_t *frame;
  enfle_instance_t *instance;

  frame = (enfle_frame_t *)_frame;
  instance = (enfle_instance_t *)frame->vo.instance;

  //debug_message_fn("(%p: width %d, rgb_stride %d, yuv_stride %d)\n", frame->rgb_ptr, instance->width, frame->rgb_stride, frame->yuv_stride);

  yuv2rgb(frame->rgb_ptr, src[0], src[1], src[2], instance->width, 16,
	  frame->rgb_stride, frame->yuv_stride, frame->yuv_stride >> 1);
  frame->rgb_ptr += frame->rgb_stride << 4;
}

static void
rgb_field (vo_frame_t *_frame, int flags)
{
  enfle_frame_t *frame;
  enfle_instance_t *instance;

  debug_message_fn("()\n");

  frame = (enfle_frame_t *)_frame;
  instance = (enfle_instance_t *)frame->vo.instance;

  frame->rgb_ptr = frame->rgb_ptr_base;
  if ((flags & VO_TOP_FIELD) == 0)
    frame->rgb_ptr += instance->rgbstride;
}

static int
enfle_rgb_setup(vo_instance_t *_instance, int width, int height)
{
  enfle_instance_t *instance;
  Libmpeg2_info *info;
  VideoWindow *vw;
  Movie *m;
  Image *p;

  debug_message_fnc("(%d, %d)\n", width, height);

  instance = (enfle_instance_t *)_instance;
  m = instance->m;
  info = (Libmpeg2_info *)m->movie_private;
  vw = info->vw;
  p = info->p;

  m->width  = instance->width  = width;
  m->height = instance->height = height;
  m->rendering_width  = m->width;
  m->rendering_height = m->height;
  image_width(p)  = m->rendering_width;
  image_height(p) = m->rendering_height;
  image_bpl(p) = (image_width(p) * vw->bits_per_pixel) >> 3;
  instance->rgbstride = (image_width(p) * vw->bits_per_pixel) >> 3;
  instance->image_size = instance->rgbstride * height;

  debug_message_fnc("allocating %d bytes\n", image_bpl(p) * image_height(p));
  if (memory_alloc(image_rendered_image(p), image_bpl(p) * image_height(p)) == NULL)
    return 0;

  debug_message("video: (%d,%d) -> (%d,%d)\n",
		m->width, m->height, m->rendering_width, m->rendering_height);

  yuv2rgb_init(vw->bits_per_pixel, MODE_RGB);

  return enfle_alloc_frames((vo_instance_t *)instance,
			    image_width(p), image_height(p), sizeof(enfle_frame_t),
			    rgb_copy_slice, rgb_field,
			    enfle_draw);
}

vo_instance_t *
vo_enfle_rgb_open(VideoWindow *vw, Movie *m, Image *p)
{
  enfle_instance_t *instance;

  if ((instance = malloc(sizeof(enfle_instance_t))) == NULL)
    return NULL;

  instance->vo.setup = enfle_rgb_setup;
  instance->vo.close = enfle_free_frames;
  instance->vo.get_frame = rgb_get_frame;
  instance->m = m;
  instance->p = p;

  return (vo_instance_t *)instance;
}

/* YUV */

static void
enfle_yuv_draw(vo_frame_t *_frame)
{
  enfle_frame_t *frame = (enfle_frame_t *)_frame;
  enfle_instance_t *instance = (enfle_instance_t *)frame->vo.instance;
  Movie *m = instance->m;
  Image *p = instance->p;
  Libmpeg2_info *info = (Libmpeg2_info *)m->movie_private;

  if (m->status == _PLAY) {
    if ((int)m->framerate == 0 && info->mpeg2dec.frame_rate > 0) {
      m->framerate = info->mpeg2dec.frame_rate;
      show_message("(%d, %d) fps %2.5f\n", m->width, m->height, m->framerate);
    }

    pthread_mutex_lock(&info->update_mutex);
    memcpy(memory_ptr(image_rendered_image(p)), frame->vo.base[0], instance->image_size);
    info->to_render++;
    m->current_frame++;
    while (m->status == _PLAY && info->to_render > 0)
      pthread_cond_wait(&info->update_cond, &info->update_mutex);
    pthread_mutex_unlock(&info->update_mutex);
  }
}

static int
enfle_yuv_alloc_frames(vo_instance_t *_instance, int width, int height, int frame_size,
		   void (*draw)(vo_frame_t *))
{
  enfle_instance_t *instance;
  int size;
  uint8_t *alloc;
  int i;

  debug_message_fn("(%d, %d)\n", width, height);

  instance = (enfle_instance_t *)_instance;
  instance->prediction_index = 1;
  size = (width * height) >> 2;

  if ((alloc = memalign(ATTRIBUTE_ALIGNED_MAX, 18 * size)) == NULL) {
    debug_message_fnc("memalign() ERROR\n");
    return 1;
  } else {
    debug_message_fnc("memalign(%d, %d) = %p\n", ATTRIBUTE_ALIGNED_MAX, 18 * size, alloc);
  }

  for (i = 0; i < 3; i++) {
    instance->frame_ptr[i] = (vo_frame_t *)(instance->frame + i);
    instance->frame[i].vo.base[0] = alloc;
    instance->frame[i].vo.base[1] = alloc + 4 * size;
    instance->frame[i].vo.base[2] = alloc + 5 * size;
    instance->frame[i].vo.copy = NULL;
    instance->frame[i].vo.field = NULL;
    instance->frame[i].vo.draw = draw;
    instance->frame[i].vo.instance = (vo_instance_t *)instance;
    alloc += 6 * size;
  }

  debug_message_fnc("OK\n");

  return 0;
}

static vo_frame_t *
yuv_get_frame (vo_instance_t *_instance, int flags)
{
  enfle_instance_t *instance;
  enfle_frame_t *frame;

  instance = (enfle_instance_t *)_instance;
  frame = (enfle_frame_t *)enfle_get_frame((vo_instance_t *)instance, flags);

  return (vo_frame_t *)frame;
}

static int
enfle_yuv_setup(vo_instance_t *_instance, int width, int height)
{
  enfle_instance_t *instance;
  Libmpeg2_info *info;
  VideoWindow *vw;
  Movie *m;
  Image *p;

  debug_message_fnc("(%d, %d)\n", width, height);

  instance = (enfle_instance_t *)_instance;
  m = instance->m;
  info = (Libmpeg2_info *)m->movie_private;
  vw = info->vw;
  p = info->p;

  m->width  = instance->width  = width;
  m->height = instance->height = height;
  m->rendering_width  = m->width;
  m->rendering_height = m->height;
  image_width(p)  = m->rendering_width;
  image_height(p) = m->rendering_height;
  image_bpl(p) = (image_width(p) * 3) >> 1;
  instance->rgbstride = (image_width(p) * vw->bits_per_pixel) >> 3;
  instance->yuvstride = image_width(p);
  instance->image_size = image_bpl(p) * image_height(p);

  debug_message_fnc("allocating %d bytes\n", image_bpl(p) * image_height(p));
  if (memory_alloc(image_rendered_image(p), image_bpl(p) * image_height(p)) == NULL)
    return 0;

  debug_message("video: (%d,%d) -> (%d,%d)\n",
		m->width, m->height, m->rendering_width, m->rendering_height);

  return enfle_yuv_alloc_frames((vo_instance_t *)instance,
			    image_width(p), image_height(p), sizeof(enfle_frame_t),
			    enfle_yuv_draw);
}

vo_instance_t *
vo_enfle_yuv_open(VideoWindow *vw, Movie *m, Image *p)
{
  enfle_instance_t *instance;

  if ((instance = calloc(1, sizeof(enfle_instance_t))) == NULL)
    return NULL;

  instance->vo.setup = enfle_yuv_setup;
  instance->vo.close = enfle_free_frames;
  instance->vo.get_frame = yuv_get_frame;
  instance->m = m;
  instance->p = p;

  return (vo_instance_t *)instance;
}
