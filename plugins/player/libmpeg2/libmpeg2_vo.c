/*
 * libmpeg2_vo.c -- video output routine for libmpeg2 player plugin
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Feb 20 22:00:18 2001.
 * $Id: libmpeg2_vo.c,v 1.1 2001/02/20 13:54:59 sian Exp $
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

#include "libmpeg2_vo.h"

static void
enfle_draw(vo_frame_t *frame)
{
  enfle_instance_t *instance = (enfle_instance_t *)frame->instance;
  VideoWindow *vw = instance->vw;
  Movie *m = instance->m;
  Image *p = instance->p;

  m->render_frame(vw, m, p);
}

static int
enfle_alloc_frames(vo_instance_t *instance, int width, int height, int frame_size,
		   void (*copy)(vo_frame_t *, uint8_t **),
		   void (*field)(vo_frame_t *, int),
		   void (*draw)(vo_frame_t *))
{
  enfle_instance_t *instance;
  int size;
  uint8_t *alloc;
  int i;

  instance = (enfle_instance_t *)_instance;
  instance->prediction_index = 1;
  size = width * height >> 2;
  if ((alloc = memalign(16, 18 * size)) == NULL)
    return 1;

  for (i = 0; i < 3; i++) {
    instance->frame_ptr[i] =
      (vo_frame_t *)(((char *)instance) + sizeof(enfle_instance_t) + i * frame_size);
    instance->frame_ptr[i]->base[0] = alloc;
    instance->frame_ptr[i]->base[1] = alloc + 4 * size;
    instance->frame_ptr[i]->base[2] = alloc + 5 * size;
    instance->frame_ptr[i]->copy = copy;
    instance->frame_ptr[i]->field = field;
    instance->frame_ptr[i]->draw = draw;
    instance->frame_ptr[i]->instance = (vo_instance_t *)instance;
    alloc += 6 * size;
  }

  return 0;
}

static void
enfle_free_frames (vo_instance_t *_instance)
{
  enfle_instance_t *instance;

  instance = (enfle_instance_t *)_instance;
  free(instance->frame_ptr[0]->base[0]);
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

#if 0
static int
enfle_setup(vo_instance_t *instance, int width, int height)
{
  return enfle_alloc_frames(instance, width, height, sizeof(enfle_frame_t), NULL, NULL, enfle_draw);
}

vo_instance_t *
vo_enfle_open(void)
{
  enfle_instance_t *instance;

  if ((instance = malloc(sizeof(enfle_instance_t))) == NULL)
    return NULL;

  instance->vo.setup = enfle_setup;
  instance->vo.close = enfle_free_frames;
  instance->vo.get_frame = enfle_get_frame;

  return (vo_instance_t *)instance;
}

static void
enfle_copy_slice(vo_frame_t *frame, uint8_t **src)
{
}

static int
enfle_slice_setup(vo_instance_t *instance, int width, int height)
{
  return enfle_alloc_frames(instance, width, height,
			    sizeof(enfle_frame_t),
			    enfle_copy_slice, NULL, enfle_draw);
}

vo_instance_t *
vo_enfle_slice_open(void)
{
  enfle_instance_t *instance;

  if ((instance = malloc(sizeof(enfle_instance_t))) == NULL)
    return NULL;

  instance->vo.setup = enfle_slice_setup;
  instance->vo.close = enfle_free_frames;
  instance->vo.get_frame = enfle_get_frame;

  return (vo_instance_t *)instance;
}
#endif

static vo_frame_t *
rgb_get_frame (vo_instance_t *_instance, int flags)
{
  enfle_instance_t *instance;
  enfle_frame_t *frame;

  instance = (enfle_instance_t *)_instance;
  frame = (enfle_frame_t *)enfle_get_frame((vo_instance_t *)instance, flags);

  frame->rgb_ptr = instance->rgbdata;
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

  yuv2rgb(frame->rgb_ptr, src[0], src[1], src[2], instance->width, 16,
	  frame->rgb_stride, frame->yuv_stride, frame->yuv_stride >> 1);
  frame->rgb_ptr += frame->rgb_stride << 4;
}

static void
rgb_field (vo_frame_t *_frame, int flags)
{
  enfle_frame_t *frame;
  enfle_instance_t *instance;

  frame = (enfle_frame_t *)_frame;
  instance = (enfle_instance_t *)frame->vo.instance;

  frame->rgb_ptr = instance->rgbdata;
  if ((flags & VO_TOP_FIELD) == 0)
    frame->rgb_ptr += instance->rgbstride;
}

static int
enfle_rgb_setup(vo_instance_t *_instance, int width, int height)
{
  enfle_instance_t *instance;

  instance = (enfle_instance_t *)_instance;

  instance->width = width;
  instance->rgbstride = (width * instance->bpp + 7) >> 3;
  instance->rgbdata = malloc(instance->rgbstride * height);

  yuv2rgb_init(instance->bpp, MODE_RGB);

  return enfle_alloc_frames((vo_instance_t *)instance,
			    width, height, sizeof(enfle_frame_t),
			    rgb_copy_slice, rgb_field,
			    enfle_draw);
}

vo_instance_t *
vo_enfle_rgb_open(int bpp)
{
  enfle_instance_t *instance;

  if ((instance = malloc(sizeof(enfle_instance_t))) == NULL)
    return NULL;

  instance->vo.setup = enfle_rgb_setup;
  instance->vo.close = enfle_free_frames;
  instance->vo.get_frame = rgb_get_frame;
  instance->bpp = bpp;

  return (vo_instance_t *)instance;
}
