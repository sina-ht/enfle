/*
 * videodecoder.c -- Video decoder
 * (C)Copyright 2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Nov 25 17:13:14 2008.
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

#include <stdio.h>
#include <stdlib.h>

#define REQUIRE_STRING_H
#define REQUIRE_UNISTD_H
#include "compat.h"
#include "common.h"

#ifndef USE_PTHREAD
#  error pthread is mandatory for videodecoder
#endif

#include <pthread.h>

#include "videodecoder-plugin.h"
#include "utils/libstring.h"

VideoDecoder *
_videodecoder_init(void)
{
  VideoDecoder *vdec;

  if ((vdec = calloc(1, sizeof(*vdec))) == NULL)
    return NULL;
  pthread_mutex_init(&vdec->update_mutex, NULL);
  pthread_cond_init(&vdec->update_cond, NULL);

  return vdec;
}

void
_videodecoder_destroy(VideoDecoder *vdec)
{
  if (vdec) {
    pthread_mutex_destroy(&vdec->update_mutex);
    pthread_cond_destroy(&vdec->update_cond);
    free(vdec);
  }
}

const char *
videodecoder_codec_name(unsigned int fourcc)
{
  switch (fourcc) {
  case FCC_H261:
    return "h261";
  case FCC_H263:
    return "h263";
  case FCC_I263:
    return "h263i";
  case FCC_U263:
  case FCC_viv1:
    return "h263p";
  case FCC_H264:
  case FCC_X264:
    return "h264";
  case FCC_DIVX: // invalid_asf
  case FCC_divx: // invalid_asf
  case FCC_DX50: // invalid_asf
  case FCC_xvid: // invalid_asf
  case FCC_XVID: // invalid_asf
  case FCC_MP4S:
  case FCC_M4S2:
  case FCC_0x04000000:
  case FCC_DIV1:
  case FCC_BLZ0:
  case FCC_mp4v:
  case FCC_UMP4:
  case FCC_FMP4:
    return "mpeg4";
  case FCC_DIV3: // invalid_asf
  case FCC_div3: // invalid_asf
  case FCC_DIV4:
  case FCC_DIV5:
  case FCC_DIV6:
  case FCC_MP43:
  case FCC_MPG3:
  case FCC_AP41:
  case FCC_COL1:
  case FCC_COL0:
    return "msmpeg4";
  case FCC_MP42:
  case FCC_mp42:
  case FCC_DIV2:
    return "msmpeg4v2";
  case FCC_MP41:
  case FCC_MPG4:
  case FCC_mpg4:
    return "msmpeg4v1";
  case FCC_WMV1:
    return "wmv1";
  case FCC_WMV2:
    return "wmv2";
  case FCC_WMV3:
    return "wmv3";
  case FCC_dvsd:
  case FCC_dvhd:
  case FCC_dvsl:
  case FCC_dv25:
    return "dvvideo";
  case FCC_mpg1:
  case FCC_mpg2:
  case FCC_PIM1:
  case FCC_VCR2:
    return "mpeg1video";
  case FCC_mjpg:
  case FCC_MJPG:
    return "mjpeg";
  case FCC_JPGL:
  case FCC_LJPG:
    return "ljpeg";
  case FCC_HFYU:
    return "huffyuv";
  case FCC_CYUV:
    return "cyuv";
  case FCC_Y422:
  case FCC_I420:
  case FCC_HM12:
    return "rawvideo";
  case FCC_IV31:
  case FCC_IV32:
    return "indeo3";
  case FCC_VP31:
    return "vp3";
  case FCC_ASV1:
    return "asv1";
  case FCC_ASV2:
    return "asv2";
  case FCC_VCR1:
    return "vcr1";
  case FCC_FFV1:
    return "ffv1";
  case FCC_FFVH:
    return "ffvhuff";
  case FCC_Xxan:
    return "xan_wc4";
  case FCC_mrle:
  case FCC_0x01000000:
    return "msrle";
  case FCC_cvid:
    return "cinepak";
  case FCC_MSVC:
  case FCC_msvc:
  case FCC_CRAM:
  case FCC_cram:
  case FCC_WHAM:
  case FCC_wham:
    return"msvideo1";
  case FCC_DIB:
  case FCC_RGB2:
    return "raw";
  case FCC_GIF:
    return "gif";
  default:
    break;
  }

  return NULL;
}

int
videodecoder_query(EnflePlugins *eps, Movie *m __attribute__((unused)), unsigned int fourcc, unsigned int *types_r, Config *c)
{
  PluginList *pl = eps->pls[ENFLE_PLUGIN_VIDEODECODER];
  Plugin *p;
  VideoDecoderPlugin *vdp;
  String *s;
  char *pluginname, **pluginnames;
  int res;
  void *k;
  unsigned int kl;
  const char *codec_name = videodecoder_codec_name(fourcc);

  if (codec_name == NULL)
    return 0;

  s = string_create();
  string_catf(s, "/enfle/plugins/videodecoder/preference/%s", codec_name);
  pluginnames = config_get_list(c, string_get(s), &res);
  string_destroy(s);
  if (pluginnames) {
    int i = 0;

    while ((pluginname = pluginnames[i])) {
      if (strcmp(pluginname, ".") == 0) {
	debug_message_fnc("Failed, no further try.\n");
	return 0;
      }
      if ((p = pluginlist_get(pl, pluginname))) {
	if ((vdp = plugin_get(p)) == NULL) {
	  err_message_fnc("plugin %s (prefered for %s) is NULL.\n", pluginname, codec_name);
	  continue;
	}
	debug_message_fnc("try %s (prefered for %s)\n", pluginname, codec_name);
	if ((*types_r = vdp->query(fourcc, vdp->vd_private)) != 0)
	  return 1;
	debug_message_fnc("%s failed.\n", pluginname);
      } else {
	show_message_fnc("%s (prefered for %s) not found.\n", pluginname, codec_name);
      }
      i++;
    }
  }
  
  pluginlist_iter(pl, k, kl, p) {
    vdp = plugin_get(p);
    debug_message_fnc("try %s\n", (char *)k);
    if ((*types_r = vdp->query(fourcc, vdp->vd_private)) != 0) {
      pluginlist_move_to_top;
      return 1;
    }
    //debug_message("%s: failed\n", (char *)k);
  }
  pluginlist_iter_end;

  return 0;
}

int
videodecoder_select(EnflePlugins *eps, Movie *m, unsigned int fourcc, Config *c)
{
  PluginList *pl = eps->pls[ENFLE_PLUGIN_VIDEODECODER];
  Plugin *p;
  VideoDecoderPlugin *vdp;
  String *s;
  char *pluginname, **pluginnames;
  int res;
  void *k;
  unsigned int kl;
  const char *codec_name = videodecoder_codec_name(fourcc);

  if (codec_name == NULL)
    return 0;

  s = string_create();
  string_catf(s, "/enfle/plugins/videodecoder/preference/%s", codec_name);
  pluginnames = config_get_list(c, string_get(s), &res);
  string_destroy(s);
  if (pluginnames) {
    int i = 0;

    while ((pluginname = pluginnames[i])) {
      if (strcmp(pluginname, ".") == 0) {
	debug_message_fnc("Failed, no further try.\n");
	return 0;
      }
      if ((p = pluginlist_get(pl, pluginname))) {
	vdp = plugin_get(p);
	debug_message_fnc("try %s (preferred for %s)\n", pluginname, codec_name);
	if ((m->vdec = vdp->init(fourcc, vdp->vd_private)) != NULL)
	  return 1;
	warning_fnc("%s (preferred for %s) failed.\n", pluginname, codec_name);
      } else {
	warning_fnc("%s (preferred for %s) not found.\n", pluginname, codec_name);
      }
      i++;
    }
  }
  
  pluginlist_iter(pl, k, kl, p) {
    vdp = plugin_get(p);
    debug_message_fnc("try %s\n", (char *)k);
    if ((m->vdec = vdp->init(fourcc, vdp->vd_private)) != NULL) {
      pluginlist_move_to_top;
      return 1;
    }
    //debug_message("%s: failed\n", (char *)k);
  }
  pluginlist_iter_end;

  return 0;
}

VideoDecoder *
videodecoder_create(EnflePlugins *eps, const char *pluginname)
{
  Plugin *p;
  VideoDecoderPlugin *vdp;

  if ((p = pluginlist_get(eps->pls[ENFLE_PLUGIN_VIDEODECODER], (char *)pluginname)) == NULL)
    return NULL;
  vdp = plugin_get(p);

  return vdp->init(0, vdp->vd_private);
}
