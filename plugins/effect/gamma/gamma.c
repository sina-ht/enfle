/*
 * gamma.c -- Gamma Effect plugin
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Sep 17 22:39:45 2001.
 * $Id: gamma.c,v 1.2 2001/09/18 05:22:24 sian Exp $
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

static Image *effect(Image *, void *);

static EffectPlugin plugin = {
  type: ENFLE_PLUGIN_EFFECT,
  name: "Gamma",
  description: "Gamma Correction Effect plugin version 0.1",
  author: "Hiroshi Takekawa",

  effect: effect
};

static void calc_gamma(unsigned char *, int);

void *
plugin_entry(void)
{
  EffectPlugin *ep;

  if ((ep = (EffectPlugin *)calloc(1, sizeof(EffectPlugin))) == NULL)
    return NULL;
  memcpy(ep, &plugin, sizeof(EffectPlugin));

  /* Precompute log table */
  calc_gamma(NULL, 0);

  return (void *)ep;
}

void
plugin_exit(void *p)
{
  free(p);
}

/* for internal use */

#define GAMMA_ACCURACY 13

/*
 * This gamma correction routine uses the algorithm by Adam
 * M. Costello <amc @ cs.berkeley.edu>, with slight modification.
 *
 * calc_gamma() requires 32bits or greater accuracy.
 */

void
calc_gamma(unsigned char g[256], int gamma)
{
  static unsigned int log_table[510];
  int i, j;

  if (gamma == 0) {
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
      k = (log_table[i << 1] >> GAMMA_ACCURACY) * gamma;
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

static Image *
effect(Image *p, void *params)
{
  Config *c = (Config *)params;
  Image *save;
  int index, result;
  unsigned int i;
  unsigned char g[256];
  double gammas[7] = { 2.2, 1.7, 1.45, 1.0, 1 / 1.45, 1 / 1.7, 1 / 2.2 };
  unsigned char *s, *d;

  index = config_get_int(c, "/enfle/plugins/effect/gamma/index", &result);
  if (!result)
    return NULL;
  if (index < 0 || index > 6)
    return NULL;

  if ((save = image_dup(p)) == NULL)
    return NULL;

  calc_gamma(g, gammas[index] * (1 << GAMMA_ACCURACY));

  s = memory_ptr(save->image);
  d = memory_ptr(p->image);
  for (i = 0; i < p->bytes_per_line * p->height; i++)
    d[i] = g[s[i]];

  return save;
}
