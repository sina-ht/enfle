/*
 * pe_image.h
 */

#include "pe.h"
#include "hash.h"

typedef struct _pe_image PE_image;
struct _pe_image {
  char *filepath;
  IMAGE_FILE_HEADER pe_header;
  IMAGE_OPTIONAL_HEADER opt_header;
  IMAGE_SECTION_HEADER *sect_headers;
  Hash *export_symbols;
  unsigned char *image;

  /* methods */
  int (*load)(PE_image *, char *);
  void *(*resolve)(PE_image *, const char *);
  void (*destroy)(PE_image *);
};

#define peimage_load(p, path) (p)->load((p), (path))
#define peimage_resolve(p, name) (p)->resolve((p), (name))
#define peimage_destroy(p) (p)->destroy((p))

PE_image *peimage_create(void);
