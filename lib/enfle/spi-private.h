/*
 * spi-private.h
 */

#include <sys/time.h>
#include "windef.h"

typedef struct _picture_info {
  long left, top;   /* 画像を展開する位置 */
  long width;       /* 画像の幅(pixel) */
  long height;      /* 画像の高さ(pixel) */
  WORD x_density;   /* 画素の水平方向密度 */
  WORD y_density;   /* 画素の垂直方向密度 */
  short colorDepth; /* １画素当たりのbit数 */
  HLOCAL hInfo;     /* 画像内のテキスト情報 */
} PictureInfo;

#define SPI_SUCCESS            0 /* 正常終了 */
#define SPI_UNIMPLEMENTED     -1 /* その機能はインプリメントされていない */
#define SPI_CB_RETURNS_NONZERO 1 /* コールバック関数が非0を返したので展開を中止した */
#define SPI_UNKNOWN_FORMAT     2 /* 未知のフォーマット */
#define SPI_BROKEN_DATA        3 /* データが壊れている */
#define SPI_NOENOUGH_MEMORY    4 /* メモリーが確保出来ない */
#define SPI_MEMORY_ERROR       5 /* メモリーエラー（Lock出来ない、等） */
#define SPI_FILE_READ_ERROR    6 /* ファイルリードエラー */
#define SPI_RESERVED           7 /* （予約） */
#define SPI_INTERNAL_ERROR     8 /* 内部エラー */

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
  unsigned char method[8]; /* 圧縮法の種類 */
  unsigned long position;  /* ファイル上での位置 */
  unsigned long compsize;  /* 圧縮されたサイズ */
  unsigned long filesize;  /* 元のファイルサイズ */
  time_t timestamp;        /* ファイルの更新日時 */
  char path[200];          /* 相対パス */
  char filename[200];      /* ファイルネーム */
  unsigned long crc;       /* CRC */
} fileInfo;

typedef struct _BITMAPINFOHEADER {
  unsigned long       biSize;         /*  0: 構造体のサイズ(バイト) */
  long                biWidth;        /*  4: イメージの幅 */
  long                biHeight;       /*  8: イメージの高さ */
  unsigned short      biPlanes;       /* 12: must be 1 */
  unsigned short      biBitCount;     /* 14: 1pixelあたりのbit数 */
  unsigned long       biCompression;  /* 16: 圧縮タイプ */
  unsigned long       biSizeImage;    /* 20: イメージのサイズ */
  long                biXPixPerMeter; /* 24: 水平解像度 */
  long                biYPixPerMeter; /* 28: 垂直解像度 */
  unsigned long       biClrUsed;      /* 32: カラーテーブル関係 */
  unsigned long       biClrImportant; /* 36: カラーテーブル関係 */
} BITMAPINFOHEADER;

/* typedef int PASCAL (*IsSupportedFunc)(LPSTR, DWORD); */
/* typedef int PASCAL (*GetPluginInfoFunc)(int, LPSTR, int); */
typedef int PASCAL (*GetArchiveInfoFunc)(LPSTR, long, unsigned int, HLOCAL *);
typedef int PASCAL (*GetFileInfoFunc)(LPSTR, long, LPSTR, unsigned int, fileInfo *);
typedef int PASCAL (*GetFileFunc)(LPSTR, long, LPSTR, unsigned int, ProgressCallback, long);
