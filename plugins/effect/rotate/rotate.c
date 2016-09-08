/*
 * rotate.c -- Rotate Effect plugin
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Mar  6 12:00:34 2004.
 * $Id: rotate.c,v 1.5 2008/04/19 09:28:05 sian Exp $
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

#include <sys/stat.h>
#include <errno.h>

#define REQUIRE_STRING_H
#define REQUIRE_UNISTD_H
#include "compat.h"

#include "common.h"

#include "utils/libstring.h"
#include "enfle/effect-plugin.h"

static int set_rotate(void *);
static int set_flip(void *);
static int init_rotate(void *);
static int effect(Image *, int, int);

static UIAction actions[] = {
  { "rotate_left", set_rotate, (void *)1, ENFLE_KEY_l, ENFLE_MOD_None, ENFLE_Button_None },
  { "rotate_right", set_rotate, (void *)-1, ENFLE_KEY_r, ENFLE_MOD_None, ENFLE_Button_None },
  { "rotate_180", set_rotate, (void *)2, ENFLE_KEY_l, ENFLE_MOD_Shift, ENFLE_Button_None },
  { "rotate_180", set_rotate, (void *)-2, ENFLE_KEY_r, ENFLE_MOD_Shift, ENFLE_Button_None },
  { "flip_vertical", set_flip, (void *)1, ENFLE_KEY_v, ENFLE_MOD_None, ENFLE_Button_None },
  { "flip_horizontal", set_flip, (void *)2, ENFLE_KEY_h, ENFLE_MOD_None, ENFLE_Button_None },
  { "rotate_init", init_rotate, NULL, ENFLE_KEY_v, ENFLE_MOD_Shift, ENFLE_Button_None },
  { "rotate_init", init_rotate, NULL, ENFLE_KEY_h, ENFLE_MOD_Shift, ENFLE_Button_None },
  UI_ACTION_END
};

int rotate_mode = 0;
int flip_mode = 0;

static EffectPlugin plugin = {
  .type = ENFLE_PLUGIN_EFFECT,
  .name = "Rotate",
  .description = "Rotate Effect plugin version 0.1",
  .author = "Hiroshi Takekawa",

  .actions = actions,
  .effect = effect
};

ENFLE_PLUGIN_ENTRY(effect_rotate)
{
  EffectPlugin *ep;

  if ((ep = (EffectPlugin *)calloc(1, sizeof(EffectPlugin))) == NULL)
    return NULL;
  memcpy(ep, &plugin, sizeof(EffectPlugin));

  return (void *)ep;
}

ENFLE_PLUGIN_EXIT(effect_rotate, p)
{
  free(p);
}

/* for internal use */

static int
set_rotate(void *a)
{
  int m = (int)(long)a;

  rotate_mode += m;
  rotate_mode &= 3;

  return 1;
}

static int
set_flip(void *a)
{
  int m = (int)(long)a;

  flip_mode ^= m;

  return 1;
}

static int
init_rotate(void *a)
{
  rotate_mode = 0;
  flip_mode = 0;

  return 1;
}

static int
horizontal_flip(Image *p, int src, int dst)
{
  unsigned char *d = memory_ptr(image_image_by_index(p, dst));
  unsigned char *s = memory_ptr(image_image_by_index(p, src));
  int bytes_per_pixel = p->bits_per_pixel >> 3;
  unsigned int y;

  if ((bytes_per_pixel << 3) != p->bits_per_pixel)
    return 0;

  s += image_bpl_by_index(p, src) * (image_height_by_index(p, src) - 1);
  for (y = 0; y < image_height_by_index(p, src); y++) {
    memcpy(d, s, image_bpl_by_index(p, src));
    d += image_bpl_by_index(p, src);
    s -= image_bpl_by_index(p, src);
  }

  return 1;
}

static int
left_rotate(Image *p, int src, int dst)
{
  unsigned char *d = memory_ptr(image_image_by_index(p, dst));
  unsigned char *s = memory_ptr(image_image_by_index(p, src));
  int bytes_per_pixel = p->bits_per_pixel >> 3;
  unsigned int x, y;
  int i;

  if ((bytes_per_pixel << 3) != p->bits_per_pixel)
    return 0;

  image_width_by_index(p, dst) = image_height_by_index(p, src);
  image_height_by_index(p, dst) = image_width_by_index(p, src);
  image_bpl_by_index(p, dst) = bytes_per_pixel * image_width_by_index(p, dst);

  s += (image_width_by_index(p, src) - 1) * bytes_per_pixel;
  for (x = 0; x < image_width_by_index(p, src); x++) {
    for (y = 0; y < image_height_by_index(p, src); y++) {
      for (i = 0; i < bytes_per_pixel; i++)
	d[i] = s[i];
      s += image_bpl_by_index(p, src);
      d += bytes_per_pixel;
    }
    s -= image_height_by_index(p, src) * image_bpl_by_index(p, src) + bytes_per_pixel;
  }

  return 1;
}

static int
vertical_flip(Image *p, int src, int dst)
{
  unsigned char *d = memory_ptr(image_image_by_index(p, dst));
  unsigned char *s = memory_ptr(image_image_by_index(p, src));
  int bytes_per_pixel = p->bits_per_pixel >> 3;
  unsigned int x, y;
  int i;

  if ((bytes_per_pixel << 3) != p->bits_per_pixel)
    return 0;

  s += (image_width_by_index(p, src) - 1) * bytes_per_pixel;
  for (y = 0; y < image_height_by_index(p, src); y++) {
    for (x = 0; x < image_width_by_index(p, src); x++) {
      for (i = 0; i < bytes_per_pixel; i++)
	d[i] = s[i];
      s -= bytes_per_pixel;
      d += bytes_per_pixel;
    }
    s += image_bpl_by_index(p, src) + image_width_by_index(p, src) * bytes_per_pixel;
    d += image_bpl_by_index(p, src) - image_width_by_index(p, src) * bytes_per_pixel;
  }

  return 1;
}

static int
right_rotate(Image *p, int src, int dst)
{
  unsigned char *d = memory_ptr(image_image_by_index(p, dst));
  unsigned char *s = memory_ptr(image_image_by_index(p, src));
  int bytes_per_pixel = p->bits_per_pixel >> 3;
  unsigned int x, y;
  int i;

  if ((bytes_per_pixel << 3) != p->bits_per_pixel)
    return 0;

  image_width_by_index(p, dst) = image_height_by_index(p, src);
  image_height_by_index(p, dst) = image_width_by_index(p, src);
  image_bpl_by_index(p, dst) = bytes_per_pixel * image_width_by_index(p, dst);

  s += (image_height_by_index(p, src) - 1) * image_bpl_by_index(p, src);
  for (x = 0; x < image_width_by_index(p, src); x++) {
    for (y = 0; y < image_height_by_index(p, src); y++) {
      for (i = 0; i < bytes_per_pixel; i++)
	d[i] = s[i];
      s -= image_bpl_by_index(p, src);
      d += bytes_per_pixel;
    }
    s += image_height_by_index(p, src) * image_bpl_by_index(p, src) + bytes_per_pixel;
  }

  return 1;
}

static void
swap(int *a, int *b)
{
  int t = *a;

  *a = *b;
  *b = t;
}

/* methods */

static int
effect(Image *p, int src, int dst)
{
  int result = 0;
  int f = 0;

  if (flip_mode == 0 && rotate_mode == 0)
    return 2;

  image_data_alloc_from_other(p, src, dst);

  if (flip_mode & 1) {
    if ((result = vertical_flip(p, src, dst)) == 0)
      return 0;
    swap(&src, &dst);
    f++;
  }
  if (flip_mode & 2) {
    if ((result = horizontal_flip(p, src, dst)) == 0)
      return 0;
    swap(&src, &dst);
    f++;
  }

  switch (rotate_mode) {
  case 0:
    break;
  case 1:
    if ((result = left_rotate(p, src, dst)) == 0)
      return 0;
    swap(&src, &dst);
    f++;
    break;
  case 2:
    if ((result = left_rotate(p, src, dst)) == 0)
      return 0;
    if ((result = left_rotate(p, dst, src)) == 0)
      return 0;
    f += 2;
    break;
  case 3:
    if ((result = right_rotate(p, src, dst)) == 0)
      return 0;
    swap(&src, &dst);
    f++;
    break;
  default:
    break;
  }

  if (!(f & 1))
    image_data_swap(p, src, dst);

  return 1;
}
