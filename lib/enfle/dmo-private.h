/*
 * dmo-private.h
 */

#include <sys/time.h>
#include "dllloader/windef.h"

#if 0
#define DMO_SUCCESS

static const char *dmo_errormsg[] = {
  /* -1: Unimplemented */
  "Success",
};
#endif

typedef long PASCAL (*GetClassObjectFunc)(const GUID *, const GUID *, void **);

#define IUNKNOWN_MEMBERS \
  long PASCAL (*QueryInterface)(struct IUnknown *, const GUID *, void **); \
  long PASCAL (*AddRef)(struct IUnknown *); \
  long PASCAL (*Release)(struct IUnknown *)

struct IUnknown;
struct IUnknown_vt {
  IUNKNOWN_MEMBERS;
};

struct IUnknown {   
  struct IUnknown_vt *vt;
};

// IClassFactory

struct IClassFactory;
struct IClassFactory_vt {
  IUNKNOWN_MEMBERS;
  long PASCAL (*CreateInstance)(struct IClassFactory *, struct IUnknown *, const GUID *, void **);
};

struct IClassFactory {
  struct IClassFactory_vt *vt;
};

// IMediaBuffer

typedef struct _IMediaBuffer IMediaBuffer;
struct IMediaBuffer_vt {
  IUNKNOWN_MEMBERS;
  HRESULT PASCAL (*SetLength)(IMediaBuffer *, unsigned long);
  HRESULT PASCAL (*GetMaxLength)(IMediaBuffer *, unsigned long *);
  HRESULT PASCAL (*GetBufferAndLength)(IMediaBuffer *, char **, unsigned long *);
};

struct _IMediaBuffer {
  struct IMediaBuffer_vt *vt;
};

typedef long long REFERENCE_TIME;

typedef struct _DMO_OUTPUT_DATA_BUFFER {
  IMediaBuffer *pBuffer;
  unsigned long dwStatus;
  REFERENCE_TIME rtTimestamp;
  REFERENCE_TIME rtTimelength;
} DMO_OUTPUT_DATA_BUFFER;

// IMediaObject

typedef struct __attribute__((__packed__)) _MediaType {
  GUID            majortype;              // 0x0
  GUID            subtype;                // 0x10
  int             bFixedSizeSamples;      // 0x20
  int             bTemporalCompression;   // 0x24
  unsigned long   lSampleSize;            // 0x28
  GUID            formattype;             // 0x2c
  struct IUnknown *pUnk;                  // 0x3c
  unsigned long   cbFormat;               // 0x40
  char *          pbFormat;               // 0x44
} AM_MEDIA_TYPE, DMO_MEDIA_TYPE;

typedef struct _IMediaObject IMediaObject;
struct IMediaObject_vt {
  IUNKNOWN_MEMBERS;
  HRESULT PASCAL (*GetStreamCount)(IMediaObject *, unsigned long *, unsigned long *);
  HRESULT PASCAL (*GetInputStreamInfo)(IMediaObject *, unsigned long, unsigned long *);
  HRESULT PASCAL (*GetOutputStreamInfo)(IMediaObject *, unsigned long, unsigned long *);
  HRESULT PASCAL (*GetInputType)(IMediaObject *, unsigned long, unsigned long, DMO_MEDIA_TYPE *);
  HRESULT PASCAL (*GetOutputType)(IMediaObject *, unsigned long, unsigned long, DMO_MEDIA_TYPE *);
  HRESULT PASCAL (*SetInputType)(IMediaObject *, unsigned long, const DMO_MEDIA_TYPE *, unsigned long);
  HRESULT PASCAL (*SetOutputType)(IMediaObject *, unsigned long, const DMO_MEDIA_TYPE *, unsigned long);
  HRESULT PASCAL (*GetInputCurrentType)(IMediaObject *,	unsigned long, DMO_MEDIA_TYPE *);
  HRESULT PASCAL (*GetOutputCurrentType)(IMediaObject *, unsigned long, DMO_MEDIA_TYPE *);
  HRESULT PASCAL (*GetInputSizeInfo)(IMediaObject *, unsigned long, unsigned long *, unsigned long *, unsigned long *);
  HRESULT PASCAL (*GetOutputSizeInfo)(IMediaObject *, unsigned long, unsigned long *, unsigned long *);
  HRESULT PASCAL (*GetInputMaxLatency)(IMediaObject *, unsigned long, REFERENCE_TIME *);
  HRESULT PASCAL (*SetInputMaxLatency)(IMediaObject *, unsigned long, REFERENCE_TIME);
  HRESULT PASCAL (*Flush)(IMediaObject *);
  HRESULT PASCAL (*Discontinuity)(IMediaObject *, unsigned long);
  HRESULT PASCAL (*AllocateStreamingResources)(IMediaObject *);
  HRESULT PASCAL (*FreeStreamingResources)(IMediaObject *);
  HRESULT PASCAL (*GetInputStatus)(IMediaObject *, unsigned long, unsigned long *);
  HRESULT PASCAL (*ProcessInput)(IMediaObject *, unsigned long, IMediaBuffer *, unsigned long, REFERENCE_TIME, REFERENCE_TIME);
  HRESULT PASCAL (*ProcessOutput)(IMediaObject *, unsigned long, unsigned long, DMO_OUTPUT_DATA_BUFFER *, unsigned long *);
  HRESULT PASCAL (*Lock)(IMediaObject *, long);
};

struct _IMediaObject {
  struct IMediaObject_vt *vt;
};
