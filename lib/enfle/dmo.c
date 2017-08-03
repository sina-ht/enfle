/*
 * dmo.c -- dmo to enfle bridge
 * (C)Copyright 2000-2004 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Oct  2 02:31:23 2005.
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

#define DMO_BRIDGE_DESCRIPTION "DMO bridge version 0.1 "

static unsigned int vd_query(unsigned int, void *);
static VideoDecoder *vd_init(unsigned int, void *);
static unsigned int ad_query(unsigned int, void *);
static AudioDecoder *ad_init(unsigned int, void *);

typedef struct _dmovideodecoder {
  PE_image *pe;
  IMediaObject *mo;
  DMO_MEDIA_TYPE in_mt, out_mt;
  Image *p;
  const char *vcodec_name;
  int to_skip, if_image_alloced;
} DMOVideoDecoder;

typedef struct _dmoaudiodecoder {
  PE_image *pe;
  IMediaObject *mo;
  AM_MEDIA_TYPE in_mt, out_mt;
  const char *acodec_name;
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

// GUID's

static const GUID iid_iunknown = {   
  0x00000000, 0x0000, 0x0000,
  { 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 }
};

static const GUID iid_icf = {   
  0x00000001, 0x0000, 0x0000,
  { 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 }
};

static const GUID iid_imo = {
  0xd8ad0f58, 0x5494, 0x4102,
  { 0x97, 0xc5, 0xec, 0x79, 0x8e, 0x59, 0xbc, 0xf4 }
};

static const GUID iid_imb = {
  0x59eff8b9, 0x938c, 0x4a26,
  { 0x82, 0xf2, 0x95, 0xcb, 0x84, 0xcd, 0xc8, 0x37 }
};

static const GUID iid_imoip = {
  0x651b9ad0, 0x0fc7, 0x4aa9,
  { 0x95, 0x38, 0xd8, 0x99, 0x31, 0x01, 0x07, 0x41}
};

static const GUID iid_idmovoo = {
  0xbe8f4f4e, 0x5b16, 0x4d29,
  { 0xb3, 0x50, 0x7f, 0x6b, 0x5d, 0x92, 0x98, 0xac}
};

// wmvdmod.dll
static const GUID wmvdmo_clsid = {
  0x82d353df, 0x90bd, 0x4382,
  {0x8b, 0xc2, 0x3f, 0x61, 0x92, 0xb7, 0x6e, 0x34}
};

// wmv9dmod.dll
static const GUID wmv3_clsid = {
  0x724bb6a4, 0xe526, 0x450f,
  {0xaf, 0xfa, 0xab, 0x9b, 0x45, 0x12, 0x91, 0x11}
};

enum codec_type {
  _CODEC_VIDEO = 0,
  _CODEC_AUDIO = 1,
  _CODEC_UNKNOWN = -1
};

struct codec_info {
  enum codec_type type;
  const char *dll;
  const GUID *clsid;
  const char *desc;
};

static struct codec_info CodecInfo [] = {
  { _CODEC_VIDEO, "wmv9dmod.dll", &wmv3_clsid, "Windows Media Video (WMV3)" },
  { _CODEC_VIDEO, "wmvdmod.dll", &wmvdmo_clsid, "Windows Media Video (WMV3)" },
};

// buffer implementation

static HRESULT PASCAL
CMediaBuffer_SetLength(IMediaBuffer *This, unsigned long cbLength)
{
  CMediaBuffer *cmb = (CMediaBuffer *)This;

  //debug_message_fn("(%p, %ld)\n", This, cbLength);

  if (cbLength > cmb->maxlen)
    return E_INVALIDARG;
  cmb->len = cbLength;

  return S_OK;
}

static HRESULT PASCAL
CMediaBuffer_GetMaxLength(IMediaBuffer* This, unsigned long *pcbMaxLength)
{
  CMediaBuffer *cmb = (CMediaBuffer *)This;

  //debug_message_fn("(%p): maxlen = %ld\n", This, cmb->maxlen);

  if (!pcbMaxLength)
    return E_POINTER;
  *pcbMaxLength = cmb->maxlen;

  return S_OK;
}

static HRESULT PASCAL
CMediaBuffer_GetBufferAndLength(IMediaBuffer *This, char **ppBuffer, unsigned long *pcbLength)
{
  CMediaBuffer *cmb = (CMediaBuffer *)This;

  //debug_message_fn("(%p): mem = %p, len = %ld\n", This, cmb->mem, cmb->len);

  if (!ppBuffer && !pcbLength)
    return E_POINTER;
  if (ppBuffer)
    *ppBuffer = cmb->mem;
  if (pcbLength)
    *pcbLength = cmb->len;

  return S_OK;
}

static void
CMediaBuffer_Destroy(CMediaBuffer *This)
{
  //debug_message_fn("(%p)\n", This);

  if (This->freemem)
    free(This->mem);
  free(This->vt);
  free(This);
}

static long PASCAL
CMediaBuffer_QueryInterface(IUnknown *This, const GUID *riid, void **ppvObject)
{
  CMediaBuffer *me = (CMediaBuffer *)This;
  const GUID *r;
  unsigned int i = 0;

  //debug_message_fn("(%p)\n", This);

  if (!ppvObject)
    return E_POINTER;
  for (r = me->interfaces; i < sizeof(me->interfaces) / sizeof(me->interfaces[0]); r++, i++) {
    if (!memcmp(r, riid, sizeof(*r))) {
      me->vt->AddRef(This);
      *ppvObject = This;
      return 0;
    }
  }

#if 0
  {
    unsigned char *d = (unsigned char *)riid;
    debug_message_fnc("Query failed! (GUID: %p = %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X)\n", d,
		      d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7],
		      d[8], d[9], d[10], d[11], d[12], d[13], d[14], d[15]);
  }
#endif

  return E_NOINTERFACE;
}

static long PASCAL
CMediaBuffer_AddRef(IUnknown *This)
{
  CMediaBuffer *me = (CMediaBuffer *)This;

  //debug_message_fn("(%p): ref = %d\n", This, me->refcount);

  return ++(me->refcount);
}

static long PASCAL
CMediaBuffer_Release(IUnknown *This)
{
  CMediaBuffer *me = (CMediaBuffer *)This;

  //debug_message_fn("(%p): new ref = %d\n", This, me->refcount - 1);

  if(--(me->refcount) == 0)
    CMediaBuffer_Destroy(me);

  return 0;
}

static CMediaBuffer *
CMediaBufferCreate(unsigned long maxlen, void *mem, unsigned long len, int copy)
{
  CMediaBuffer *This = (CMediaBuffer *)malloc(sizeof(CMediaBuffer));

  if (!This)
    return NULL;

  This->vt = (struct IMediaBuffer_vt *)malloc(sizeof(struct IMediaBuffer_vt));
  if (!This->vt) {
    CMediaBuffer_Destroy(This);
    return NULL;
  }

  This->refcount = 1;
  This->len = len;
  This->maxlen = maxlen;
  This->freemem = 0;
  This->mem = mem;
  if (copy)
    /* make a private copy of data */
    This->mem = NULL;
  if (This->mem == NULL) {
    if (This->maxlen) {
      This->mem = malloc(This->maxlen);
      if (!This->mem) {
	CMediaBuffer_Destroy(This);
	return NULL;
      }
      This->freemem = 1;
      if (copy)
	memcpy(This->mem, mem, This->len);
    }
  }
  This->vt->QueryInterface = CMediaBuffer_QueryInterface;
  This->vt->AddRef = CMediaBuffer_AddRef;
  This->vt->Release = CMediaBuffer_Release;

  This->vt->SetLength = CMediaBuffer_SetLength;
  This->vt->GetMaxLength = CMediaBuffer_GetMaxLength;
  This->vt->GetBufferAndLength = CMediaBuffer_GetBufferAndLength;

  This->interfaces[0] = iid_iunknown;
  This->interfaces[1] = iid_imb;

  //debug_message_fn("(maxlen = %ld, mem = %p, len = %ld, copy = %d) -> %p\n", maxlen, mem, len, copy, This);

  return This;
}

// methods

static VideoDecoderStatus
vd_decode(VideoDecoder *vdec, Movie *m, Image *p, DemuxedPacket *dp, unsigned int len, unsigned int *used_r)
{
  DMOVideoDecoder *vdm = (DMOVideoDecoder *)vdec->opaque;
  CMediaBuffer *mb;
  HRESULT res;
  DMO_OUTPUT_DATA_BUFFER db;
  unsigned char *buf = dp->data;
  unsigned long status;

  if (len == 0)
    return VD_NEED_MORE_DATA;

  //debug_message_fn("(%p,buf = %p, len = %d, is_key = %d)\n", vdec, buf, len, dp->is_key);

  mb = CMediaBufferCreate(len, (void *)buf, len, 0);
  *used_r = len;

  res = vdm->mo->vt->ProcessInput(vdm->mo, 0, (IMediaBuffer *)mb, dp->is_key, 0, 0);
  ((IMediaBuffer *)mb)->vt->Release((IUnknown *)mb);
  if (res != S_OK) {
    if (res == S_FALSE)
      err_message_fnc("ProcessInput returned S_FALSE\n");
    else
      err_message_fnc("ProcessInput failed: res = 0x%lx\n", res);
    return VD_ERROR;
  }

  if (!vdm->if_image_alloced) {
    show_message_fnc("(%d, %d) fps %2.5f buffer %d bytes\n", m->width, m->height, rational_to_double(m->framerate), image_bpl(p) * image_height(p));
    if (memory_alloc(image_rendered_image(p), image_bpl(p) * image_height(p)) == NULL) {
      err_message("No enough memory for image body (%d bytes).\n", image_bpl(p) * image_height(p));
      return VD_ERROR;
    }
    vdm->if_image_alloced = 1;
  }

  db.rtTimestamp = 0;
  db.rtTimelength = 0;
  db.dwStatus = 0;
  db.pBuffer = (IMediaBuffer *)CMediaBufferCreate(vdm->out_mt.lSampleSize, memory_ptr(image_rendered_image(p)), 0, 0);
  res = vdm->mo->vt->ProcessOutput(vdm->mo, 0, 1, &db, &status);
  if (res != S_OK) {
    if (res == DMO_E_NOTACCEPTING)
      err_message_fnc("ProcessOutput failed: Not accepting\n");
    else
      err_message_fnc("ProcessOutput failed: res = 0x%lx\n", res);
    return VD_ERROR;
  }
  ((IMediaBuffer *)db.pBuffer)->vt->Release((IUnknown *)db.pBuffer);

  m->current_frame++;

  if (vdm->to_skip > 0) {
    vdm->to_skip--;
    return VD_OK;
  }

  pthread_mutex_lock(&vdec->update_mutex);

  vdec->ts_base = 0; /* XXX: set vdec->pts. */
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

  if (vdm) {
    peimage_destroy(vdm->pe);
    free(vdm);
  }

  _videodecoder_destroy(vdec);
}

static const GUID MEDIATYPE_Video = {
  0x73646976, 0x0000, 0x0010,
  {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}
};
static const GUID FORMAT_VideoInfo = {
  0x05589f80, 0xc356, 0x11ce,
  {0xbf, 0x01, 0x00, 0xaa, 0x00, 0x55, 0x59, 0x5a}
};
static const GUID MEDIASUBTYPE_RGB24 = {
  0xe436eb7d, 0x524f, 0x11ce,
  {0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70}
};
static const GUID MEDIASUBTYPE_YUY2 = {
  0x32595559, 0x0000, 0x0010,
  {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}
};
static const GUID MEDIASUBTYPE_UYVY = {
  0x59565955, 0x0000, 0x0010,
  {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}
};
static const GUID MEDIASUBTYPE_YV12 = {
  0x32315659, 0x0000, 0x0010,
  {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}
};
static const GUID MEDIASUBTYPE_I420 = {
  0x30323449, 0x0000, 0x0010,
  {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}
};
static const GUID MEDIATYPE_Audio = {
  0x73647561, 0x0000, 0x0010,
  {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}
};

static int
vd_setup(VideoDecoder *vdec, Movie *m, Image *p, int w, int h)
{
  DMOVideoDecoder *vdm = (DMOVideoDecoder *)vdec->opaque;
  DMO_MEDIA_TYPE *in_mt = &vdm->in_mt;
  DMO_MEDIA_TYPE *out_mt = &vdm->out_mt;
  BITMAPINFOHEADER *bih;
  VIDEOINFOHEADER *vih, *vih2;
  HRESULT res;
  unsigned long ninputs, noutputs;
  unsigned int bihs;

  vdm->p = p;
  vdm->if_image_alloced = 0;

  bih = m->video_header;
  bihs = bih->biSize > sizeof(BITMAPINFOHEADER) ? bih->biSize : sizeof(BITMAPINFOHEADER);

  // setup input media type
  in_mt->majortype = MEDIATYPE_Video;
  in_mt->subtype = MEDIATYPE_Video;
  in_mt->subtype.f1 = bih->biCompression;
  in_mt->formattype = FORMAT_VideoInfo;
  in_mt->bFixedSizeSamples = FALSE;
  in_mt->bTemporalCompression = TRUE;
  in_mt->pUnk = 0;
  // vih
  in_mt->cbFormat = bih->biSize + sizeof(VIDEOINFOHEADER) - sizeof(BITMAPINFOHEADER);
  vih = calloc(1, in_mt->cbFormat);
  memcpy(&vih->bmiHeader, bih, bih->biSize);
  in_mt->pbFormat = (char *)vih;
  vih->rcSource.left = vih->rcSource.top = 0;
  vih->rcSource.right = w;
  vih->rcSource.bottom = h;
  vih->rcTarget = vih->rcSource;

  // setup output media type
  out_mt->majortype = MEDIATYPE_Video;
  out_mt->subtype = MEDIASUBTYPE_YUY2; //RGB24;
  out_mt->formattype = FORMAT_VideoInfo;
  out_mt->bFixedSizeSamples = TRUE;
  out_mt->bTemporalCompression = FALSE;
  out_mt->pUnk = 0;
  // vih2
  out_mt->cbFormat = in_mt->cbFormat;
  vih2 = calloc(1, out_mt->cbFormat);
  memcpy(vih2, vih, out_mt->cbFormat);
  vih2->bmiHeader.biCompression = FCC_YUY2; //0;
  vih2->bmiHeader.biBitCount = 16; //24;
  vih2->rcTarget = vih->rcTarget;
  out_mt->pbFormat = (char *)vih2;
  //
  //out_mt->lSampleSize = labs(vih2->bmiHeader.biWidth * vih2->bmiHeader.biHeight * ((vih2->bmiHeader.biBitCount + 7) / 8));
  out_mt->lSampleSize = labs(vih2->bmiHeader.biWidth * vih2->bmiHeader.biHeight * vih2->bmiHeader.biBitCount / 8);
  debug_message_fnc("vih2: width %ld, height %ld, bit %d, size %ld\n", vih2->bmiHeader.biWidth, vih2->bmiHeader.biHeight, vih2->bmiHeader.biBitCount, out_mt->lSampleSize);
  vih2->bmiHeader.biSizeImage = out_mt->lSampleSize;

  debug_message_fnc("SetInputType...\n");
  if ((res = vdm->mo->vt->SetInputType(vdm->mo, 0, in_mt, 0)) != 0) {
    err_message_fnc("SetInputType failed: res = 0x%lx\n", res);
    return 0;
  }
  debug_message_fnc("SetInputType OK\n");

  debug_message_fnc("SetOutputType...\n");
  if ((res = vdm->mo->vt->SetOutputType(vdm->mo, 0, out_mt, 0)) != 0) {
    err_message_fnc("SetOututType failed: res = 0x%lx\n", res);
    return 0;
  }
  debug_message_fnc("SetOutputType OK\n");

  res = vdm->mo->vt->GetOutputSizeInfo(vdm->mo, 0, &ninputs, &noutputs);
  debug_message_fnc("OutputSizeInfo: res=0x%lx   size:%ld  align:%ld\n", res, ninputs, noutputs);

  // vdm->mo->vt->AllocateStreamingResources(vdm->mo);
  res = vdm->mo->vt->GetStreamCount(vdm->mo, &ninputs, &noutputs);
  debug_message_fnc("StreamCount: res=0x%lx  %ld  %ld\n", res, ninputs, noutputs);

  return 1;
}

static unsigned int
vd_query(unsigned int fourcc, void *priv)
{
  //DMOVideoDecoder *dmo_vd = (DMOVideoDecoder *)priv;

  switch (fourcc) {
  case 0:
  case FCC_WMV3: // wmv3
    return IMAGE_YUY2 |
      IMAGE_BGRA32 | IMAGE_ARGB32 |
      IMAGE_RGB24 | IMAGE_BGR24 |
      IMAGE_BGR555 | IMAGE_RGB555 |
      IMAGE_BGR565 | IMAGE_RGB565;
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
  free(ep);
}

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
  HRESULT res;
  GUID *rclsid = NULL;
  struct IClassFactory *icf = NULL, **icf_r;
  IUnknown *obj = NULL, **obj_r;
  IMediaObject *mo = NULL, **mo_r;
  int i;
  const char *desc = "";
  enum codec_type type = _CODEC_UNKNOWN;

  for (i = 0; i < sizeof(CodecInfo) / sizeof(CodecInfo[0]); i++) {
    if (strcasecmp(CodecInfo[i].dll, misc_basename(path)) == 0) {
      rclsid = (GUID *)CodecInfo[i].clsid;
      type = CodecInfo[i].type;
      desc = CodecInfo[i].desc;
      show_message("Using DMO DLL: %s: %s\n", path, desc);
      break;
    }
  }
  if (rclsid == NULL) {
    show_message_fnc("Unknown dll %s\n", path);
    return NULL;
  }

  debug_message_fnc("loading %s...\n", path);
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
  icf_r = &icf;
  if ((res = get_class_object(rclsid, &iid_icf, (void **)icf_r)) != 0 || icf == NULL) {
    err_message_fnc("GetClassObject failed.\n");
    goto error;
  }
  debug_message_fnc("GetClassObject OK.\n");

  debug_message_fnc("CreateInstance...\n");
  obj_r = &obj;
  res = icf->vt->CreateInstance(icf, 0, &iid_iunknown, (void **)obj_r);
  icf->vt->Release((IUnknown *)icf);
  if (res != 0 || obj == NULL) {
    err_message("CreateInstance failed.\n");
    goto error;
  }
  debug_message_fnc("CreateInstance OK.\n");

  debug_message_fnc("QueryInterface...\n");
  mo_r = &mo;
  res = obj->vt->QueryInterface(obj, &iid_imo, (void **)mo_r);

  /* Query extra interfaces */
  if (res == 0) {
    IMediaObjectInPlace *moip = NULL, **moip_r;
    IDMOVideoOutputOptimizations *dmovoo = NULL, **dmovoo_r;

    moip_r = &moip;
    HRESULT r = obj->vt->QueryInterface(obj, &iid_imoip, (void **)moip_r);
    if (r == 0 && moip != NULL) {
      show_message_fnc("DMO dll supports InPlace interface.  Not supported.\n");
      moip->vt->Release((IUnknown *)moip);
    }

    dmovoo_r = &dmovoo;
    r = obj->vt->QueryInterface(obj, &iid_idmovoo, (void **)dmovoo_r);
    if (r == 0 && dmovoo != NULL) {
      unsigned long flags;

      r = dmovoo->vt->QueryOperationModePreferences(dmovoo, 0, &flags);
      if (flags & DMO_VOSF_NEEDS_PREVIOUS_SAMPLE)
	warning_fnc("DMO dll might use previous sample when requested.\n");
      dmovoo->vt->Release((IUnknown *)dmovoo);
    }
  }

  obj->vt->Release((IUnknown *)obj);
  if (res != 0 || mo == NULL) {
    err_message_fnc("QueryInterface failed.\n");
    goto error;
  }
  debug_message_fnc("QueryInterface OK.\n");

  switch (type) {
  case _CODEC_VIDEO:
    if ((vdp = calloc(1, sizeof(VideoDecoderPlugin))) == NULL) {
      show_message("No enough memory for VideoDecoderPlugin\n");
      goto error;
    }
    memcpy(vdp, &vd_template, sizeof(VideoDecoderPlugin));
    if ((vdp->vd_private = dmo_vd = calloc(1, sizeof(DMOVideoDecoder))) == NULL) {
      err_message("No enough memory for DMOVideoDecoder\n");
      goto error;
    }
    dmo_vd->pe = pe;
    dmo_vd->mo = mo;
    ep = (EnflePlugin *)vdp;
    *type_return = ENFLE_PLUGIN_VIDEODECODER;
    break;
  case _CODEC_AUDIO:
    if ((adp = calloc(1, sizeof(AudioDecoderPlugin))) == NULL) {
      err_message("No enough memory for AudioDecoderPlugin\n");
      goto error;
    }
    memcpy(adp, &ad_template, sizeof(AudioDecoderPlugin));
    if ((adp->ad_private = dmo_ad = calloc(1, sizeof(DMOAudioDecoder))) == NULL) {
      err_message("No enough memory for DMOAudioDecoder\n");
      goto error;
    }
    dmo_ad->pe = pe;
    dmo_ad->mo = mo;
    ep = (EnflePlugin *)adp;
    *type_return = ENFLE_PLUGIN_AUDIODECODER;
    break;
  default:
    break;
  }

  if (ep) {
    static const char *plugin_desc = DMO_BRIDGE_DESCRIPTION;
    char *tmp;

    ep->name = strdup(misc_basename(path));
    tmp = malloc(strlen(plugin_desc) + strlen(desc) + 1);
    strcpy(tmp, plugin_desc);
    strcat(tmp, desc);
    tmp[strlen(plugin_desc) + strlen(desc)]  = '\0';
    ep->description = tmp;
    ep->author = "DMO author";

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
