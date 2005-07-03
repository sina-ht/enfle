/*
 * pe_image.h
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Jul  3 13:37:47 2005.
 * $Id: pe_image.h,v 1.5 2005/07/03 13:02:30 sian Exp $
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

#include "pe.h"
#include "utils/hash.h"

/* Must be prime */
#define PE_EXPORT_HASH_SIZE 4099
#define PE_RESOURCE_HASH_SIZE 4099

typedef struct _pe_image PE_image;
struct _pe_image {
  char *filepath;
  IMAGE_FILE_HEADER pe_header;
  IMAGE_OPTIONAL_HEADER opt_header;
  IMAGE_SECTION_HEADER *sect_headers;
  Hash *export_symbols;
  Hash *resource;
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
