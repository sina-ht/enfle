/*
 * dmo.c -- dmo to enfle bridge
 * (C)Copyright 2000, 2001, 2002by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Apr 10 20:37:25 2004.
 * $Id: dmo.c,v 1.1 2004/04/12 04:12:32 sian Exp $
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

#ifdef USE_DMO

#include "dllloader/pe_image.h"
#include "dmo-private.h"
#include "dmo.h"
#include "videodecoder-plugin.h"
#include "audiodecoder-plugin.h"
#include "utils/misc.h"

static unsigned int vd_query(unsigned int, void *);
static VideoDecoder *vd_init(unsigned int, void *);
static unsigned int ad_query(unsigned int, void *);
static AudioDecoder *ad_init(unsigned int, void *);

typedef struct _dmovideodecoder {
  PE_image *pe;
  IMediaObject *mo;
  AM_MEDIA_TYPE in_mt, out_mt;
  Image *p;
  const char *vcodec_name;
  unsigned char *buf;
  int offset, size, to_skip, if_image_alloced;
} DMOVideoDecoder;

typedef struct _dmoaudiodecoder {
  PE_image *pe;
  IMediaObject *mo;
  AM_MEDIA_TYPE in_mt, out_mt;
} DMOAudioDecoder;

static VideoDecoderPlugin vd_template = {
  .type = ENFLE_PLUGIN_VIDEODECODER,
  .name = NULL,
  .description = NULL,
  .author = NULL,

  .query = vd_query,
  .init = vd_init
};

static AudioDecoderPlugin ad_template = {
  .type = ENFLE_PLUGIN_AUDIODECODER,
  .name = NULL,
  .description = NULL,
  .author = NULL,

  .query = ad_query,
  .init = ad_init
};

static VideoDecoderStatus
vd_decode(VideoDecoder *vdec, Movie *m, Image *p, unsigned char *buf, unsigned int len, unsigned int *used_r)
{
  DMOVideoDecoder *vdm = (DMOVideoDecoder *)vdec->opaque;
  int l;
  int got_picture;

  if (vdm->size <= 0) {
    if (buf == NULL)
      return VD_NEED_MORE_DATA;
    vdm->buf = buf;
    vdm->offset = 0;
    vdm->size = len;
    *used_r = len;
  }

  //l = avcodec_decode_video(vdm->vcodec_ctx, vdm->vcodec_picture, &got_picture, vdm->buf + vdm->offset, vdm->size);
  l = -1;
  if (l < 0) {
    warning_fnc("avcodec: avcodec_decode_video return %d\n", len);
    return VD_ERROR;
  }

#if 0
  if (!vdm->if_image_alloced && vdm->vcodec_ctx->width > 0) {
    m->width = image_width(p) = vdm->vcodec_ctx->width;
    m->height = image_height(p) = vdm->vcodec_ctx->height;
    m->framerate = (double)vdm->vcodec_ctx->frame_rate / vdm->vcodec_ctx->frame_rate_base;
    show_message_fnc("(%d, %d) fps %2.5f\n", m->width, m->height, m->framerate);
    image_bpl(p) = vdm->vcodec_ctx->width * 2; /* XXX: hmm... */
    if (memory_alloc(image_rendered_image(p), image_bpl(p) * image_height(p)) == NULL) {
      err_message("No enough memory for image body (%d bytes).\n", image_bpl(p) * image_height(p));
      return VD_ERROR;
    }
    vdm->if_image_alloced = 1;
  }
#endif

  vdm->size -= l;
  vdm->offset += l;
  if (!got_picture)
    return VD_OK;

  m->current_frame++;
  if (vdm->to_skip > 0) {
    vdm->to_skip--;
    return VD_OK;
  }

  pthread_mutex_lock(&vdec->update_mutex);

  vdec->to_render++;
  while (m->status == _PLAY && vdec->to_render > 0)
    pthread_cond_wait(&vdec->update_cond, &vdec->update_mutex);
  pthread_mutex_unlock(&vdec->update_mutex);

  return VD_OK;
}

static void
vd_destroy(VideoDecoder *vdec)
{
  DMOVideoDecoder *vdm = (DMOVideoDecoder *)vdec->opaque;

  if (vdm)
    free(vdm);
  _videodecoder_destroy(vdec);
}


static int
vd_setup(VideoDecoder *vdec, Movie *m, Image *p, int w, int h)
{
  DMOVideoDecoder *vdm = (DMOVideoDecoder *)vdec->opaque;

  vdm->p = p;
  vdm->if_image_alloced = 0;
  if (image_width(p) > 0) {
    if (memory_alloc(image_rendered_image(p), image_bpl(p) * image_height(p)) == NULL) {
      err_message("No enough memory for image body (%d bytes).\n", image_bpl(p) * image_height(p));
      return 0;
    }
    //vdm->width = w;
    //vdm->height = h;
    vdm->if_image_alloced = 1;
  }

  return 1;

/*
  in_mt.majortype = MEDIATYPE_Video;
  in_mt.subtype = MEDIATYPE_Video;
  in_mt.subtype.f1 = this->m_sVhdr->bmiHeader.biCompression;
  in_mt.formattype = FORMAT_VideoInfo;
  in_mt.bFixedSizeSamples = false;
  in_mt.bTemporalCompression = true;
  in_mt.pUnk = 0;
  in_mt.cbFormat = bihs;
  in_mt.pbFormat = (char*)this->m_sVhdr;

  out_mt.majortype = MEDIATYPE_Video;
  out_mt.subtype = MEDIASUBTYPE_RGB24;
  out_mt.formattype = FORMAT_VideoInfo;
  out_mt.bFixedSizeSamples = true;
  out_mt.bTemporalCompression = false;
  out_mt.lSampleSize = labs(this->m_sVhdr2->bmiHeader.biWidth*this->m_sVhdr2->bmiHeader.biHeight
			    * ((this->m_sVhdr2->bmiHeader.biBitCount + 7) / 8));
*/
}

static unsigned int
vd_query(unsigned int fourcc, void *priv)
{
  //DMOVideoDecoder *dmo_vd = (DMOVideoDecoder *)priv;

  switch (fourcc) {
  case 0:
  case FCC_WMV3: // wmv3
    return (IMAGE_I420 |
	    IMAGE_BGRA32 | IMAGE_ARGB32 |
	    IMAGE_RGB24 | IMAGE_BGR24 |
	    IMAGE_BGR_WITH_BITMASK | IMAGE_RGB_WITH_BITMASK);
  default:
    break;
  }
  return 0;
}

static VideoDecoder *
vd_init(unsigned int fourcc, void *priv)
{
  DMOVideoDecoder *dmo_vd = (DMOVideoDecoder *)priv;
  VideoDecoder *vdec;

  switch (fourcc) {
  case 0:
  case FCC_WMV3: // wmv3
    break;
  default:
    debug_message_fnc("Video [%c%c%c%c](%08X) was not identified as any dmo supported format.\n",
		       fourcc        & 0xff,
		      (fourcc >>  8) & 0xff,
		      (fourcc >> 16) & 0xff,
		      (fourcc >> 24) & 0xff,
		       fourcc);
    return NULL;
  }

  if ((vdec = _videodecoder_init()) == NULL)
    return NULL;
  vdec->opaque = dmo_vd;
  vdec->name = "dmo";
  vdec->setup = vd_setup;
  vdec->decode = vd_decode;
  vdec->destroy = vd_destroy;

  dmo_vd->vcodec_name = videodecoder_codec_name(fourcc);
  return vdec;
}

static unsigned int
ad_query(unsigned int fourcc, void *priv)
{
  //DMOAudioDecoder *dmo_ad = (DMOAudioDecoder *)priv;
  return 0;
}

static AudioDecoder *
ad_init(unsigned int fourcc, void *priv)
{
  //DMOAudioDecoder *dmo_ad = (DMOAudioDecoder *)priv;
  return NULL;
}

static void
dmo_plugin_exit(void *p)
{
  EnflePlugin *ep = (EnflePlugin *)p;

  if (ep->name)
    free((void *)ep->name);
  if (ep->description)
    free((void *)ep->description);

  switch (ep->type) {
  case ENFLE_PLUGIN_VIDEODECODER:
    {
      VideoDecoderPlugin *vdp = (VideoDecoderPlugin *)p;
      DMOVideoDecoder *dmo_vd = vdp->vd_private;

      peimage_destroy(dmo_vd->pe);
      free(dmo_vd);
    }
    break;
  case ENFLE_PLUGIN_AUDIODECODER:
    {
      AudioDecoderPlugin *adp = (AudioDecoderPlugin *)p;
      DMOAudioDecoder *dmo_ad = adp->ad_private;

      peimage_destroy(dmo_ad->pe);
      free(dmo_ad);
    }
    break;
  default:
    show_message_fnc("inappropriate type %d\n", ep->type);
    break;
  }

  free(ep);
}

static const GUID iid_iunknown = {   
  0x00000000, 0x0000, 0x0000,
  {0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}
};

static const GUID iid_icf = {   
  0x00000001, 0x0000, 0x0000,
  {0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}
};

static const GUID iid_imo = {
  0xd8ad0f58, 0x5494, 0x4102,
  { 0x97, 0xc5, 0xec, 0x79, 0x8e, 0x59, 0xbc, 0xf4}
};

static GUID wmvdmo_clsid = {
  0x82d353df, 0x90bd, 0x4382,
  {0x8b, 0xc2, 0x3f, 0x61, 0x92, 0xb7, 0x6e, 0x34}
};

char *
dmo_load(EnflePlugins *eps, char *path, PluginType *type_return)
{
  PE_image *pe;
  DMOVideoDecoder *dmo_vd = NULL;
  DMOAudioDecoder *dmo_ad = NULL;
  GetClassObjectFunc get_class_object;
  EnflePlugin *ep = NULL;
  VideoDecoderPlugin *vdp = NULL;
  AudioDecoderPlugin *adp = NULL;
  Plugin *p = NULL;
  PluginList *pl;
  long res;
  GUID *rclsid = &wmvdmo_clsid;
  struct IClassFactory *icf = NULL;
  struct IUnknown *obj = NULL;
  IMediaObject *mo = NULL;

  debug_message_fnc("%s...\n", path);
  pe = peimage_create();
  if (!peimage_load(pe, path)) {
    show_message("peimage_load() failed: %s\n", path);
    return NULL;
  }
  debug_message_fnc("peimage_load() OK\n");

  if ((get_class_object = (GetClassObjectFunc)peimage_resolve(pe, "DllGetClassObject")) == NULL) {
    show_message("Cannot resolve DllGetClassObject.\n");
    goto error;
  }

  debug_message_fnc("GetClassObject...\n");
  if ((res = get_class_object(rclsid, &iid_icf, (void **)&icf)) != 0 || icf == NULL) {
    err_message_fnc("GetClassObject failed.\n");
    goto error;
  }
  debug_message_fnc("GetClassObject OK.\n");

  debug_message_fnc("CreateInstance...\n");
  res = icf->vt->CreateInstance(icf, 0, &iid_iunknown, (void **)&obj);
  icf->vt->Release((struct IUnknown *)icf);
  if (res != 0 || obj == NULL) {
    show_message("CreateInstance failed.\n");
    goto error;
  }
  debug_message_fnc("CreateInstance OK.\n");

  debug_message_fnc("QueryInterface...\n");
  res = obj->vt->QueryInterface(obj, &iid_imo, (void **)&mo);
  if (res == 0) {
    /* XXX: query for some extra available interface */
    /* InPlace */
    /* Optim */
  }
  obj->vt->Release((struct IUnknown *)obj);
  if (res != 0 || mo == NULL) {
    debug_message_fnc("QueryInterface failed.\n");
    goto error;
  }
  debug_message_fnc("QueryInterface OK.\n");

  /* XXX: Video only */
  if ((vdp = calloc(1, sizeof(VideoDecoderPlugin))) == NULL) {
    show_message("No enough memory for VideoDecoderPlugin\n");
    goto error;
  }
  memcpy(vdp, &vd_template, sizeof(VideoDecoderPlugin));
  if ((vdp->vd_private = dmo_vd = calloc(1, sizeof(DMOVideoDecoder))) == NULL) {
    show_message("No enough memory for DMOVideoDecoder\n");
    goto error;
  }
  dmo_vd->pe = pe;
  dmo_vd->mo = mo;
  ep = (EnflePlugin *)vdp;
  *type_return = ENFLE_PLUGIN_VIDEODECODER;

  if (0) {
    /* Audio */
    if ((adp = calloc(1, sizeof(AudioDecoderPlugin))) == NULL) {
      show_message("No enough memory for AudioDecoderPlugin\n");
      goto error;
    }
    memcpy(adp, &ad_template, sizeof(AudioDecoderPlugin));
    if ((adp->ad_private = dmo_ad = calloc(1, sizeof(DMOAudioDecoder))) == NULL) {
      show_message("No enough memory for DMOAudioDecoder\n");
      goto error;
    }
    dmo_ad->pe = pe;
    dmo_ad->mo = mo;
    ep = (EnflePlugin *)adp;
    *type_return = ENFLE_PLUGIN_AUDIODECODER;
  }

  if (ep) {
    ep->name = strdup(misc_basename(path));
    ep->description = strdup(misc_basename(path));
    ep->author = (char *)"DMO author";

    p = plugin_create();
    p->filepath = strdup(path);
    p->substance = ep;
    p->substance_unload = dmo_plugin_exit;

    pl = eps->pls[ep->type];
    if (!pluginlist_add(pl, p ,ep->name)) {
      show_message("pluginlist_add failed: %s\n", ep->name);
      goto error;
    }

    return (char *)ep->name;
  }

 error:
  peimage_destroy(pe);
  if (p)
    plugin_destroy(p);
  if (dmo_vd)
    free(dmo_vd);
  if (dmo_ad)
    free(dmo_ad);
  if (vdp)
    free(vdp);
  if (adp)
    free(adp);

  return NULL;
}

#endif
