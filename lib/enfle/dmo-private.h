/*
 * dmo-private.h
 */

#include <sys/time.h>
#include "dllloader/windef.h"

/*
  DMO_E_INVALIDSTREAMINDEX
   Invalid stream index.
  DMO_E_INVALIDTYPE
   Invalid media type.
  DMO_E_TYPE_NOT_SET
   Media type was not set. One or more streams require a media
   type before this operation can be performed.
  DMO_E_NOTACCEPTING
   Data cannot be accepted on this stream. You might need to
   process more output data; see IMediaObject::ProcessInput.
  DMO_E_TYPE_NOT_ACCEPTED
   Media type was not accepted.
  DMO_E_NO_MORE_ITEMS
   Media-type index is out of range.
*/

#define DMO_E_INVALIDSTREAMINDEX 0x80040201
#define DMO_E_INVALIDTYPE 0x80040202
#define DMO_E_TYPE_NOT_SET 0x80040203
#define DMO_E_NOTACCEPTING 0x80040204
#define DMO_E_TYPE_NOT_ACCEPTED 0x80040205
#define DMO_E_NO_MORE_ITEMS 0x80040206

typedef long STDCALL (*GetClassObjectFunc)(const GUID *, const GUID *, void **);

#define IUNKNOWN_MEMBERS \
  HRESULT STDCALL (*QueryInterface)(IUnknown *, const GUID *, void **); \
  HRESULT STDCALL (*AddRef)(IUnknown *); \
  HRESULT STDCALL (*Release)(IUnknown *)

typedef struct _IUnknown IUnknown;
struct IUnknown_vt {
  IUNKNOWN_MEMBERS;
};

struct _IUnknown {   
  struct IUnknown_vt *vt;
};

// IClassFactory

struct IClassFactory;
struct IClassFactory_vt {
  IUNKNOWN_MEMBERS;
  long STDCALL (*CreateInstance)(struct IClassFactory *, IUnknown *, const GUID *, void **);
};

struct IClassFactory {
  struct IClassFactory_vt *vt;
};

// IMediaBuffer

typedef struct _IMediaBuffer IMediaBuffer;
struct IMediaBuffer_vt {
  IUNKNOWN_MEMBERS;
  HRESULT STDCALL (*SetLength)(IMediaBuffer *, unsigned long);
  HRESULT STDCALL (*GetMaxLength)(IMediaBuffer *, unsigned long *);
  HRESULT STDCALL (*GetBufferAndLength)(IMediaBuffer *, char **, unsigned long *);
};

struct _IMediaBuffer {
  struct IMediaBuffer_vt *vt;
};

typedef long long REFERENCE_TIME;

typedef struct _dmo_output_data_buffer {
  IMediaBuffer *pBuffer;
  unsigned long dwStatus;
  REFERENCE_TIME rtTimestamp;
  REFERENCE_TIME rtTimelength;
} DMO_OUTPUT_DATA_BUFFER;

typedef struct _CMediaBuffer {
  struct IMediaBuffer_vt *vt;
  int refcount;
  GUID interfaces[2];
  void *mem;
  unsigned long len;
  unsigned long maxlen;
  int freemem;
} CMediaBuffer;

// IMediaObject

typedef struct __attribute__((__packed__)) _media_type {
  GUID             majortype;              // 0x0
  GUID             subtype;                // 0x10
  int              bFixedSizeSamples;      // 0x20
  int              bTemporalCompression;   // 0x24
  unsigned long    lSampleSize;            // 0x28
  GUID             formattype;             // 0x2c
  IUnknown        *pUnk;                   // 0x3c
  unsigned long    cbFormat;               // 0x40
  char            *pbFormat;               // 0x44
} AM_MEDIA_TYPE, DMO_MEDIA_TYPE;

typedef struct _IMediaObject IMediaObject;
struct IMediaObject_vt {
  IUNKNOWN_MEMBERS;
  HRESULT STDCALL (*GetStreamCount)(IMediaObject *, unsigned long *, unsigned long *);
  HRESULT STDCALL (*GetInputStreamInfo)(IMediaObject *, unsigned long, unsigned long *);
  HRESULT STDCALL (*GetOutputStreamInfo)(IMediaObject *, unsigned long, unsigned long *);
  HRESULT STDCALL (*GetInputType)(IMediaObject *, unsigned long, unsigned long, DMO_MEDIA_TYPE *);
  HRESULT STDCALL (*GetOutputType)(IMediaObject *, unsigned long, unsigned long, DMO_MEDIA_TYPE *);
  HRESULT STDCALL (*SetInputType)(IMediaObject *, unsigned long, const DMO_MEDIA_TYPE *, unsigned long);
  HRESULT STDCALL (*SetOutputType)(IMediaObject *, unsigned long, const DMO_MEDIA_TYPE *, unsigned long);
  HRESULT STDCALL (*GetInputCurrentType)(IMediaObject *,	unsigned long, DMO_MEDIA_TYPE *);
  HRESULT STDCALL (*GetOutputCurrentType)(IMediaObject *, unsigned long, DMO_MEDIA_TYPE *);
  HRESULT STDCALL (*GetInputSizeInfo)(IMediaObject *, unsigned long, unsigned long *, unsigned long *, unsigned long *);
  HRESULT STDCALL (*GetOutputSizeInfo)(IMediaObject *, unsigned long, unsigned long *, unsigned long *);
  HRESULT STDCALL (*GetInputMaxLatency)(IMediaObject *, unsigned long, REFERENCE_TIME *);
  HRESULT STDCALL (*SetInputMaxLatency)(IMediaObject *, unsigned long, REFERENCE_TIME);
  HRESULT STDCALL (*Flush)(IMediaObject *);
  HRESULT STDCALL (*Discontinuity)(IMediaObject *, unsigned long);
  HRESULT STDCALL (*AllocateStreamingResources)(IMediaObject *);
  HRESULT STDCALL (*FreeStreamingResources)(IMediaObject *);
  HRESULT STDCALL (*GetInputStatus)(IMediaObject *, unsigned long, unsigned long *);
  HRESULT STDCALL (*ProcessInput)(IMediaObject *, unsigned long, IMediaBuffer *, unsigned long, REFERENCE_TIME, REFERENCE_TIME);
  HRESULT STDCALL (*ProcessOutput)(IMediaObject *, unsigned long, unsigned long, DMO_OUTPUT_DATA_BUFFER *, unsigned long *);
  HRESULT STDCALL (*Lock)(IMediaObject *, long);
};

struct _IMediaObject {
  struct IMediaObject_vt *vt;
};

// IMediaObjectInPlace

typedef struct _IMediaObjectInPlace IMediaObjectInPlace;
struct IMediaObjectInPlace_vt {   
  IUNKNOWN_MEMBERS;
  HRESULT STDCALL (*Process)(IMediaObjectInPlace *, unsigned long, BYTE *, REFERENCE_TIME, unsigned long);
  HRESULT STDCALL (*Clone)(IMediaObjectInPlace *, IMediaObjectInPlace **);
  HRESULT STDCALL (*GetLatency)(IMediaObjectInPlace *, REFERENCE_TIME *);
};

struct _IMediaObjectInPlace {
  struct IMediaObjectInPlace_vt *vt;
};

// IDMOVideoOutputOptimizations

enum _DMO_VIDEO_OUTPUT_STREAM_FLAGS {
  DMO_VOSF_NEEDS_PREVIOUS_SAMPLE = 0x1
};

typedef struct _IDMOVideoOutputOptimizations IDMOVideoOutputOptimizations;
struct IDMOVideoOutputOptimizations_vt {
  IUNKNOWN_MEMBERS;
  HRESULT STDCALL (*QueryOperationModePreferences)(IDMOVideoOutputOptimizations *, unsigned long, unsigned long *);
  HRESULT STDCALL (*SetOperationMode)(IDMOVideoOutputOptimizations *, unsigned long, unsigned long);
  HRESULT STDCALL (*GetCurrentOperationMode)(IDMOVideoOutputOptimizations *, unsigned long, unsigned long *);
  HRESULT STDCALL (*GetCurrentSampleRequirements)(IDMOVideoOutputOptimizations *, unsigned long, unsigned long *);
};

struct _IDMOVideoOutputOptimizations {
  struct IDMOVideoOutputOptimizations_vt *vt;
};

// VIDEOINFOHEADER

typedef struct __attribute__((__packed__)) _rect32 {
  int left, top, right, bottom;
} RECT32;

#include "enfle/demultiplexer_bitmapinfoheader.h"

typedef struct __attribute__((__packed__)) _videoinfoheader {
  RECT32            rcSource;          // The bit we really want to use
  RECT32            rcTarget;          // Where the video should go
  unsigned long     dwBitRate;         // Approximate bit data rate
  unsigned long     dwBitErrorRate;    // Bit error rate for this stream
  REFERENCE_TIME    AvgTimePerFrame;   // Average time per frame (100ns units)
  BITMAPINFOHEADER  bmiHeader;
} VIDEOINFOHEADER;
