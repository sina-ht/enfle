/*
 * gamma.c -- Gamma Effect plugin
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Mar  6 12:00:56 2004.
 * $Id: gamma.c,v 1.5 2004/03/06 03:43:36 sian Exp $
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
#include <math.h>
#include <sys/stat.h>
#include <errno.h>

#define REQUIRE_STRING_H
#define REQUIRE_UNISTD_H
#include "compat.h"
#include "common.h"

#include "utils/libstring.h"
#include "enfle/effect-plugin.h"

static int set_gamma(void *);
static int effect(Image *, int, int);

static UIAction actions[] = {
  { "gamma_1", set_gamma, (void *)0, ENFLE_KEY_1, ENFLE_MOD_None, ENFLE_Button_None },
  { "gamma_2", set_gamma, (void *)1, ENFLE_KEY_2, ENFLE_MOD_None, ENFLE_Button_None },
  { "gamma_3", set_gamma, (void *)2, ENFLE_KEY_3, ENFLE_MOD_None, ENFLE_Button_None },
  { "gamma_4", set_gamma, (void *)3, ENFLE_KEY_4, ENFLE_MOD_None, ENFLE_Button_None },
  { "gamma_5", set_gamma, (void *)4, ENFLE_KEY_5, ENFLE_MOD_None, ENFLE_Button_None },
  { "gamma_6", set_gamma, (void *)5, ENFLE_KEY_6, ENFLE_MOD_None, ENFLE_Button_None },
  { "gamma_7", set_gamma, (void *)6, ENFLE_KEY_7, ENFLE_MOD_None, ENFLE_Button_None },
  UI_ACTION_END
};

int gamma_idx = 3;

static EffectPlugin plugin = {
  .type = ENFLE_PLUGIN_EFFECT,
  .name = "Gamma",
  .description = "Gamma Correction Effect plugin version 0.1",
  .author = "Hiroshi Takekawa",

  .actions = actions,
  .effect = effect
};

static void calc_gamma(unsigned char *, int);

ENFLE_PLUGIN_ENTRY(effect_gamma)
{
  EffectPlugin *ep;

  if ((ep = (EffectPlugin *)calloc(1, sizeof(EffectPlugin))) == NULL)
    return NULL;
  memcpy(ep, &plugin, sizeof(EffectPlugin));

  /* Precompute log table */
  calc_gamma(NULL, 0);

  return (void *)ep;
}

ENFLE_PLUGIN_EXIT(effect_gamma, p)
{
  free(p);
}

/* for internal use */

static int
set_gamma(void *a)
{
  gamma_idx = (int)a;

  return 1;
}

#define GAMMA_ACCURACY 13

/*
 * This gamma correction routine uses the algorithm by Adam
 * M. Costello <amc @ cs.berkeley.edu>, with slight modification.
 *
 * calc_gamma() requires 32bits or greater accuracy.
 */

void
calc_gamma(unsigned char g[256], int gamma_value)
{
  static unsigned int log_table[510];
  int i, j;

  if (gamma_value == 0) {
    /* initialize */
    double f;

    f = (double)0x7fffffff / log(510.0);
    for (i = 1; i <= 510; i++)
      log_table[i] = log(510.0 / i) * f + 0.5;
  } else {
    g[0] = 0;
    j = 1;
    for (i = 1; i <= 255; i++) {
      unsigned int k;
      k = (log_table[i << 1] >> GAMMA_ACCURACY) * gamma_value;
      while (log_table[j] > k)
	j++;
      g[i] = j >> 1;
    }
  }
}

#if 0
void
calc_gamma_fp(unsigned char g[256], double gamma)
{
  int i;

  g[0] = 0;
  for (i = 1; i <= 255; i++)
    g[i] = pow(i / 255.0, gamma) * 255 + 0.5;
}
#endif

/* methods */

static int
effect(Image *p, int src, int dst)
{
  unsigned int i;
  unsigned char g[256];
  double gammas[7] = { 2.2, 1.7, 1.45, 1.0, 1 / 1.45, 1 / 1.7, 1 / 2.2 };
  unsigned char *s, *d;

  if (p->type == _INDEX)
    return 2;

  if (gamma_idx < 0 || gamma_idx > 6)
    return 0;

  if (gamma_idx == 3)
    return 2;

  calc_gamma(g, gammas[gamma_idx] * (1 << GAMMA_ACCURACY));

  image_data_alloc_from_other(p, src, dst);
  s = memory_ptr(image_image_by_index(p, src));
  d = memory_ptr(image_image_by_index(p, dst));
  for (i = 0; i < image_bpl_by_index(p, src) * image_height_by_index(p, src); i++)
    d[i] = g[s[i]];

  return 1;
}
