/*
 * audiodecoder.c -- audiodecoder plugin interface
 * (C)Copyright 2000-2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Apr 10 17:52:14 2004.
 * $Id: audiodecoder.c,v 1.5 2004/04/12 04:14:10 sian Exp $
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

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "audiodecoder-plugin.h"
#include "utils/misc.h"
#include "utils/libstring.h"

AudioDecoder *
_audiodecoder_init(void)
{
  AudioDecoder *vdec;

  if ((vdec = calloc(1, sizeof(*vdec))) == NULL)
    return NULL;
  return vdec;
}

void
_audiodecoder_destroy(AudioDecoder *vdec)
{
  if (vdec)
    free(vdec);
}

const char *
audiodecoder_codec_name(unsigned int fourcc)
{
  switch (fourcc) {
  case WAVEFORMAT_TAG_PCM:
    return "pcm";
  case WAVEFORMAT_TAG_MS_ADPCM:
    return "ms_adpcm";
  case WAVEFORMAT_TAG_IMA_ADPCM:
    return "ima_adpcm";
  case WAVEFORMAT_TAG_MS_GSM_6_10:
    return "ms_gsm_6_10";
  case WAVEFORMAT_TAG_MSN_Audio:
    return "MSN_Audio";
  case WAVEFORMAT_TAG_MP2:
    return "mp2";
  case WAVEFORMAT_TAG_MP3:
    return "mp3";
  case WAVEFORMAT_TAG_Voxware:
    return "voxware";
  case WAVEFORMAT_TAG_Acelp:
    return "acelp";
  case WAVEFORMAT_TAG_DivX_WMAv1:
    return "wmav1";
  case WAVEFORMAT_TAG_DivX_WMAv2:
    return "wmav2";
  case WAVEFORMAT_TAG_IMC:
    return "imc";
  case WAVEFORMAT_TAG_AC3:
    return "ac3";
  case WAVEFORMAT_TAG_VORBIS:
    return "vorbis";
  default:
    break;
  }

  return NULL;
}

int
audiodecoder_query(EnflePlugins *eps, Movie *m, unsigned int fourcc, unsigned int *types_r, Config *c)
{
  PluginList *pl = eps->pls[ENFLE_PLUGIN_AUDIODECODER];
  Plugin *p;
  AudioDecoderPlugin *adp;
  String *s;
  char *pluginname, **pluginnames;
  int res;
  void *k;
  unsigned int kl;
  const char *codec_name = audiodecoder_codec_name(fourcc);

  if (codec_name == NULL)
    return 0;

  s = string_create();
  string_catf(s, "/enfle/plugins/audiodecoder/preference/%s", codec_name);
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
	adp = plugin_get(p);
	debug_message_fnc("try %s (prefered for %s)\n", pluginname, codec_name);
	if ((*types_r = adp->query(fourcc, adp->ad_private)) != 0)
	  return 1;
	debug_message_fnc("%s failed.\n", pluginname);
      } else {
	show_message_fnc("%s (prefered for %s) not found.\n", pluginname, codec_name);
      }
      i++;
    }
  }
  
  pluginlist_iter(pl, k, kl, p) {
    adp = plugin_get(p);
    debug_message_fnc("try %s\n", (char *)k);
    if ((*types_r = adp->query(fourcc, adp->ad_private)) != 0) {
      pluginlist_move_to_top;
      return 1;
    }
    //debug_message("%s: failed\n", (char *)k);
  }
  pluginlist_iter_end;

  return 0;
}

int
audiodecoder_select(EnflePlugins *eps, Movie *m, unsigned int fourcc, Config *c)
{
  PluginList *pl = eps->pls[ENFLE_PLUGIN_AUDIODECODER];
  Plugin *p;
  AudioDecoderPlugin *adp;
  String *s;
  char *pluginname, **pluginnames;
  int res;
  void *k;
  unsigned int kl;
  const char *codec_name = audiodecoder_codec_name(fourcc);

  if (codec_name == NULL)
    return 0;

  s = string_create();
  string_catf(s, "/enfle/plugins/audiodecoder/preference/%s", codec_name);
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
	adp = plugin_get(p);
	debug_message_fnc("try %s (prefered for %s)\n", pluginname, codec_name);
	if ((m->adec = adp->init(fourcc, adp->ad_private)) != NULL)
	  return 1;
	debug_message_fnc("%s failed.\n", pluginname);
      } else {
	show_message_fnc("%s (prefered for %s) not found.\n", pluginname, codec_name);
      }
      i++;
    }
  }
  
  pluginlist_iter(pl, k, kl, p) {
    adp = plugin_get(p);
    debug_message_fnc("try %s\n", (char *)k);
    if ((m->adec = adp->init(fourcc, adp->ad_private)) != NULL) {
      pluginlist_move_to_top;
      return 1;
    }
    //debug_message("%s: failed\n", (char *)k);
  }
  pluginlist_iter_end;

  return 0;
}

AudioDecoder *
audiodecoder_create(EnflePlugins *eps, const char *pluginname)
{
  Plugin *p;
  AudioDecoderPlugin *adp;

  if ((p = pluginlist_get(eps->pls[ENFLE_PLUGIN_AUDIODECODER], (char *)pluginname)) == NULL)
    return NULL;
  adp = plugin_get(p);

  return adp->init(0, adp->ad_private);
}
