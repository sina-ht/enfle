/*
 * spi-private.h
 */

#include <sys/time.h>
#include "windef.h"

typedef struct _picture_info {
  long left, top;   /* ������Ÿ��������� */
  long width;       /* ��������(pixel) */
  long height;      /* �����ι⤵(pixel) */
  WORD x_density;   /* ���Ǥο�ʿ����̩�� */
  WORD y_density;   /* ���Ǥο�ľ����̩�� */
  short colorDepth; /* �������������bit�� */
  HLOCAL hInfo;     /* ������Υƥ����Ⱦ��� */
} PictureInfo;

#define SPI_SUCCESS            0 /* ���ｪλ */
#define SPI_UNIMPLEMENTED     -1 /* ���ε�ǽ�ϥ���ץ���Ȥ���Ƥ��ʤ� */
#define SPI_CB_RETURNS_NONZERO 1 /* ������Хå��ؿ�����0���֤����Τ�Ÿ������ߤ��� */
#define SPI_UNKNOWN_FORMAT     2 /* ̤�ΤΥե����ޥå� */
#define SPI_BROKEN_DATA        3 /* �ǡ���������Ƥ��� */
#define SPI_NOENOUGH_MEMORY    4 /* ���꡼�����ݽ���ʤ� */
#define SPI_MEMORY_ERROR       5 /* ���꡼���顼��Lock����ʤ������� */
#define SPI_FILE_READ_ERROR    6 /* �ե�����꡼�ɥ��顼 */
#define SPI_RESERVED           7 /* ��ͽ��� */
#define SPI_INTERNAL_ERROR     8 /* �������顼 */

static const char *spi_errormsg[] = {
  /* -1: Unimplemented */
  "Success",
  "Callback returned non-zero",
  "Unknown format",
  "Data broken",
  "No enough memory",
  "Memory error",
  "File read error",
  "Reserved",
  "Internal error"
};

typedef int PASCAL (*ProgressCallback)(int, int, long);

typedef int PASCAL (*IsSupportedFunc)(LPSTR, DWORD);
typedef int PASCAL (*GetPluginInfoFunc)(int, LPSTR, int);
typedef int PASCAL (*GetPictureInfoFunc)(LPSTR, long, unsigned int, PictureInfo *);
typedef int PASCAL (*GetPictureFunc)(LPSTR, long, unsigned int, HANDLE *, HANDLE *, ProgressCallback, long);

typedef struct _file_info {
  unsigned char method[8]; /* ����ˡ�μ��� */
  unsigned long position;  /* �ե������Ǥΰ��� */
  unsigned long compsize;  /* ���̤��줿������ */
  unsigned long filesize;  /* ���Υե����륵���� */
  time_t timestamp;        /* �ե�����ι������� */
  char path[200];          /* ���Хѥ� */
  char filename[200];      /* �ե�����͡��� */
  unsigned long crc;       /* CRC */
} fileInfo;

typedef struct _BITMAPINFOHEADER {
  unsigned long       biSize;         /*  0: ��¤�ΤΥ�����(�Х���) */
  long                biWidth;        /*  4: ���᡼������ */
  long                biHeight;       /*  8: ���᡼���ι⤵ */
  unsigned short      biPlanes;       /* 12: must be 1 */
  unsigned short      biBitCount;     /* 14: 1pixel�������bit�� */
  unsigned long       biCompression;  /* 16: ���̥����� */
  unsigned long       biSizeImage;    /* 20: ���᡼���Υ����� */
  long                biXPixPerMeter; /* 24: ��ʿ������ */
  long                biYPixPerMeter; /* 28: ��ľ������ */
  unsigned long       biClrUsed;      /* 32: ���顼�ơ��֥�ط� */
  unsigned long       biClrImportant; /* 36: ���顼�ơ��֥�ط� */
} BITMAPINFOHEADER;

/* typedef int PASCAL (*IsSupportedFunc)(LPSTR, DWORD); */
/* typedef int PASCAL (*GetPluginInfoFunc)(int, LPSTR, int); */
typedef int PASCAL (*GetArchiveInfoFunc)(LPSTR, long, unsigned int, HLOCAL *);
typedef int PASCAL (*GetFileInfoFunc)(LPSTR, long, LPSTR, unsigned int, fileInfo *);
typedef int PASCAL (*GetFileFunc)(LPSTR, long, LPSTR, unsigned int, ProgressCallback, long);
