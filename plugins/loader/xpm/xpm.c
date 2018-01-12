/*
 * xpm.c -- xpm loader plugin
 * (C)Copyright 1999, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue May 29 00:13:50 2007.
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

#include <stdlib.h>
#include <ctype.h>

#define REQUIRE_STRING_H
#include "compat.h"
#include "common.h"

#include "utils/hash.h"
#include "utils/misc.h"
#include "X11/rgbparse.h"
#include "enfle/stream-utils.h"
#include "enfle/loader-plugin.h"

#define TRANSPARENT_COLOR "None"

static const char *rgbfile_paths[] = {
  "/usr/X11R6/share/X11/rgb.txt",
  "/usr/X11R6/lib/X11/rgb.txt",
  "/usr/share/X11/rgb.txt",
  NULL
};

DECLARE_LOADER_PLUGIN_METHODS;

static LoaderPlugin plugin = {
  .type = ENFLE_PLUGIN_LOADER,
  .name = "XPM",
  .description = "XPM Loader plugin version 0.1.3",
  .author = "Hiroshi Takekawa",
  .image_private = NULL,

  .identify = identify,
  .load = load
};

static Hash *rgbhash = NULL;

ENFLE_PLUGIN_ENTRY(loader_xpm)
{
  LoaderPlugin *lp;
  int i;

  if ((lp = (LoaderPlugin *)calloc(1, sizeof(LoaderPlugin))) == NULL)
    return NULL;
  memcpy(lp, &plugin, sizeof(LoaderPlugin));

  /* parse rgb.txt */
  for (i = 0; rgbfile_paths[i] != NULL; i++) {
    debug_message_fnc("Searching for rgb.txt at %s\n", rgbfile_paths[i]);
    if ((rgbhash = rgbparse((char *)rgbfile_paths[i])) != NULL)
      break;
    debug_message_fnc(" Not found or parse error.\n");
  }
  if (rgbfile_paths[i] == NULL) {
    show_message("xpm: cannot load rgb.txt\n");
    return NULL;
  }

  return (void *)lp;
}

ENFLE_PLUGIN_EXIT(loader_xpm, p)
{
  if (rgbhash)
    hash_destroy(rgbhash);
  free(p);
}

/* for internal use */

#define GUESS_LENGTH 256
static char *
get_string(Stream *st)
{
  char *p, *buf;
  int c, count = 0;

  /* skip spaces */
  for (;;) {
    while (isspace(c = stream_getc(st))) ;

    /* skip comment */
    if (c == '/') {
      if ((c = stream_getc(st)) != '*') {
	fprintf(stderr, "got %c after /\n", c);
	return NULL;
      }
      for (;;) {
	while (stream_getc(st) != '*') ;
	if (stream_getc(st) == '/')
	  break;
      }
    } else
      break;
  }

  /* does not start with a double quotation */
  if (c != '"')
    return NULL;

  /* allocate memory */
  if ((buf = malloc(GUESS_LENGTH)) == NULL)
    return NULL;

  for (;;) {
    /* read string up to GUESS_LENGTH bytes */
    if (stream_ngets(st, buf + count * (GUESS_LENGTH - 1), GUESS_LENGTH) == NULL) {
      free(buf);
      return NULL;
    }

    /* search read string for a double quotation which closes a string */
    for (p = buf + count * (GUESS_LENGTH - 1); *p != '\0' && *p != '"'; p++) ;
    if (*p == '"')
      /* yes, found a double quotation */
      break;
    /* no, more data required */
    count++;
    if ((p = realloc(buf, (count + 1) * GUESS_LENGTH)) == NULL) {
      free(buf);
      return NULL;
    }
    buf = p;
  }

  /* terminate string */
  *p = '\0';

  /* seek archive pointer to assure that we will get next to '"' at next reading. */
  if ((c = buf + strlen(buf) - p + 2))
    stream_seek(st, -c, SEEK_CUR);

  /* shrink memory */
  if ((p = realloc(buf, strlen(buf) + 1)) == NULL) {
    free(buf);
    return NULL;
  }

  return p;
}

static int
hexchar_to_int(char c)
{
  if (isxdigit(c))
    return (isdigit(c)) ? c - '0' : toupper(c) - 'A' + 10;
  else
    return -1;
}

static int
hextwochars_to_int(char *p)
{
  int c1, c2;

  c1 = hexchar_to_int(p[0]);
  if (c1 < 0)
    return -1;
  c2 = hexchar_to_int(p[1]);
  if (c2 < 0)
    return -1;
  return (c1 << 4) + c2;
}

static int
hexfourchars_to_int(char *p)
{
  int c1, c2;

  c1 = hextwochars_to_int(p);
  if (c1 < 0)
    return -1;
  c2 = hextwochars_to_int(p + 2);
  if (c2 < 0)
    return -1;
  return (c1 << 8) + c2;
}

static char *
shrink_spaces(char *str)
{
  char *new, *p;

  if ((new = malloc(strlen(str) + 1)) == NULL)
    return NULL;
  while (isspace(*str))
    str++;
  for (p = new; *str != '\0';)
    if (isspace(*str)) {
      *p++ = ' ';
      while (isspace(*str))
	str++;
    } else
      *p++ = *str++;
  *p = '\0';
  if ((p = realloc(new, strlen(new) + 1)) == NULL)
    return NULL;
  return p;
}

static int
parse_header(Stream *st, Image *p, int *cpp)
{
  int i;
  char *ptr, *line, **tokens;

  if ((line = get_string(st)) == NULL)
    return LOAD_ERROR;
  ptr = shrink_spaces(line);
  free(line);
  line = ptr;

  if ((tokens = misc_str_split(line, ' ')) == NULL)
    return LOAD_ERROR;
  free(line);

  /* count a number of arguments */
  for (i = 0; tokens[i] != NULL; i++) ;

  /* four arguments are necessary */
  if (i < 4 || i == 5 || i > 7) {
    misc_free_str_array(tokens);
    return LOAD_NOT;
  }
  image_width(p) = atoi(tokens[0]);
  image_height(p) = atoi(tokens[1]);
  p->ncolors = atoi(tokens[2]);
  *cpp = atoi(tokens[3]);

  /* optional hotspot */
  if (i > 4) {
    image_left(p) = atoi(tokens[4]);
    image_top(p) = atoi(tokens[5]);
  }

  /* check extension */
  if (i == 7) {
    if (strcmp(tokens[6], "XPMEXT")) {
      misc_free_str_array(tokens);
      return LOAD_NOT;
    }
    printf("XPMEXT found, but ignored\n");
  }

  misc_free_str_array(tokens);

  return LOAD_OK;
}

static Hash *
parse_color(Image *p, Stream *st, int cpp)
{
  Hash *colortable;
  char *chars, *line, *ptr, **tokens;
  int c, i, num, size, t;
  unsigned int j, n;
  NColor *color, *lcolor;

  /* memory allocate for key */
  if ((chars = malloc(cpp + 1)) == NULL)
    return NULL;
  chars[cpp] = '\0';

  /* create hash for color lookup */
  size = 2069;
  t = p->ncolors >> 8;
  while (t > 1) {
    size = (size << 1) + 1;
    t >>= 1;
  }
  if ((colortable = hash_create(size)) == NULL)
    return NULL;

  for (n = 0; n < p->ncolors; n++) {
    while (isspace(c = stream_getc(st))) ;
    if (c != ',') {
      fprintf(stderr, "got %c\n", c);
      hash_destroy(colortable);
      free(chars);
      return NULL;
    }
    if ((line = get_string(st)) == NULL) {
      hash_destroy(colortable);
      free(chars);
      return NULL;
    }

    /* get chars */
    for (i = 0; i < cpp; i++)
      chars[i] = line[i];

    /* replace multiple '\t', ' ' with single ' '. */
    ptr = shrink_spaces(line + cpp);
    free(line);
    line = ptr;

    /* split */
    if ((tokens = misc_str_split(line, ' ')) == NULL) {
      hash_destroy(colortable);
      free(chars);
      return NULL;
    }
    free(line);

    /* count a number of arguments */
    for (num = 0; tokens[num] != NULL; num++) ;

    /* parse arguments */
    for (i = 0; i < num;) {
      switch (tokens[i][0]) {
      case 'm':
	i += 2;
	break;
      case 'g':
	if ((tokens[i][1] == '4' && tokens[i][2] == '\0') || tokens[i][1] == '\0') {
	  i += 2;
	  break;
	}
	goto free_and_return;
      case 's':
#if 0
	if ((color = hash_lookup(rgbhash, tokens[i + 1])) == NULL)
	    goto free_and_return;
	hash_register(colortable, chars, color);
#endif
	i += 2;
	break;
      case 'c':
	i++;
	if (!strcasecmp(tokens[i], TRANSPARENT_COLOR)) {
	  if ((color = malloc(sizeof(NColor))) == NULL)
	    goto free_and_return;
	  color->c[0] = color->c[1] = color->c[2] = -1;
	  color->n = n;
	  p->transparent_color.index = n;
	  p->transparent_color.red = 0;
	  p->transparent_color.green = 0;
	  p->transparent_color.blue = 1;
	  //p->transparent_disposal = info->transparent_disposal;
	} else if (tokens[i][0] != '#') {
	  /* color names are case insensitive */
	  for (j = 0; j < strlen(tokens[i]); j++)
	    if (isupper(tokens[i][j]))
	      tokens[i][j] = tolower(tokens[i][j]);
	  if ((lcolor = hash_lookup_str(rgbhash, tokens[i])) == NULL) {
	    fprintf(stderr, "color %s not found\n", tokens[i]);
	    goto free_and_return;
	  }
	  if ((color = malloc(sizeof(NColor))) == NULL)
	    goto free_and_return;
	  memcpy(color, lcolor, sizeof(NColor));
	} else {
	  if ((color = malloc(sizeof(NColor))) == NULL)
	    goto free_and_return;

	  switch (strlen(tokens[i] + 1)) {
	  case 6:
	    /* normal #FFFFFF style */
	    color->c[0] = hextwochars_to_int(tokens[i] + 1);
	    color->c[1] = hextwochars_to_int(tokens[i] + 3);
	    color->c[2] = hextwochars_to_int(tokens[i] + 5);
	    break;
	  case 12:
	    /* 16bits #FFFFFFFFFFFF style */
	    color->c[0] = hexfourchars_to_int(tokens[i] + 1) >> 8;
	    color->c[1] = hexfourchars_to_int(tokens[i] + 5) >> 8;
	    color->c[2] = hexfourchars_to_int(tokens[i] + 9) >> 8;
	    break;
	  case 3:
	    /* 4bits #FFF style */
	    color->c[0] = hexchar_to_int(tokens[i][1]) << 4;
	    color->c[1] = hexchar_to_int(tokens[i][2]) << 4;
	    color->c[2] = hexchar_to_int(tokens[i][3]) << 4;
	    break;
	  default:
	    fprintf(stderr, "unknown color style %s\n", tokens[i]);
	    goto free_and_return;
	  }
	}
	if (n < 256) {
	  p->colormap[n][0] = color->c[0];
	  p->colormap[n][1] = color->c[1];
	  p->colormap[n][2] = color->c[2];
	}
	color->n = n;
	hash_define_str(colortable, chars, color);
	i++;
	break;
      default:
	goto free_and_return;
      }
    }
    misc_free_str_array(tokens);
  }

  free(chars);
  return colortable;

 free_and_return:
  hash_destroy(colortable);
  free(chars);
  misc_free_str_array(tokens);
  return NULL;
}

static int
parse_body_index(Image *p, Stream *st, Hash *colortable, int cpp)
{
  unsigned char *d = memory_ptr(image_image(p));
  char *chars, *line, *ptr;
  int c, i, j;
  unsigned int linelen;
  NColor *color;

  if ((chars = malloc(cpp + 1)) == NULL)
    return 0;
  chars[cpp] = '\0';
  linelen = image_width(p) * cpp;

  for (i = image_height(p); i > 0; i--) {
    while (isspace(c = stream_getc(st))) ;
    if (c != ',') {
      free(chars);
      return 0;
    }
    if ((line = get_string(st)) == NULL) {
      free(chars);
      return 0;
    }
    if (strlen(line) != linelen)
      return 0;
    for (ptr = line; *ptr != '\0';) {
      for (j = 0; j < cpp; j++)
	chars[j] = *ptr++;
      if ((color = hash_lookup_str(colortable, chars)) == NULL)
	return 0;
      *d++ = color->n;
    }
    free(line);
  }
  free(chars);

  return 1;
}

static int
parse_body_rgb24(Image *p, Stream *st, Hash *colortable, int cpp)
{
  unsigned char *d = memory_ptr(image_image(p));
  char *chars, *line, *ptr;
  int c, i, j;
  unsigned int linelen;
  NColor *color;

  if ((chars = malloc(cpp + 1)) == NULL)
    return 0;
  chars[cpp] = '\0';
  linelen = image_width(p) * cpp;

  for (i = image_height(p); i > 0; i--) {
    while (isspace(c = stream_getc(st))) ;
    if (c != ',') {
      free(chars);
      return 0;
    }
    if ((line = get_string(st)) == NULL) {
      free(chars);
      return 0;
    }
    if (strlen(line) != linelen) {
      free(chars);
      return 0;
    }
    for (ptr = line; *ptr != '\0';) {
      for (j = 0; j < cpp; j++)
	chars[j] = *ptr++;
      if ((color = hash_lookup_str(colortable, chars)) == NULL) {
	free(chars);
	return 0;
      }
      if (color->c[0] >= 0) {
	/* avoid transparent color */
	if (color->c[0] == 0 &&
	    color->c[1] == 0 &&
	    color->c[2] == 1)
	  color->c[2] = 2;
	*d++ = color->c[0];
	*d++ = color->c[1];
	*d++ = color->c[2];
      } else {
	/* transparent */
	*d++ = 0;
	*d++ = 0;
	*d++ = 1;
      }
    }
    free(line);
  }
  free(chars);

  return 1;
}

static int
load_image(Image *p, Stream *st)
{
  Hash *colortable;
  int i, cpp;
  char buf[16];

  /* see if this file is XPM */
  stream_ngets(st, buf, 16);

  //debug_message("XPM checking: %02X%02X%02X%02X\n", buf[0], buf[1], buf[2], buf[3]);

  if (strncmp(buf, "/* XPM */", 9))
    return 0;

  debug_message("XPM identified\n");

  /* skip unused data */
  while (stream_getc(st) != '{') ;

  /* parse header */
  if ((i = parse_header(st, p, &cpp)) != LOAD_OK)
    return 0;

  debug_message("XPM header: %d %d %d %d\n", image_width(p), image_height(p), p->ncolors, cpp);

  if (p->ncolors > 256) {
    p->type = _RGB24;
    image_bpl(p) = image_width(p) * 3;
  } else {
    p->type = _INDEX;
    image_bpl(p) = image_width(p);
  }
  if (!image_image(p))
    if ((image_image(p) = memory_create()) == NULL) {
      show_message("xbm: No enough memory.\n");
      return 0;
    }
  if ((memory_alloc(image_image(p), image_bpl(p) * image_height(p))) == NULL)
    return 0;

  /* parse colors */
  if ((colortable = parse_color(p, st, cpp)) == NULL)
    return 0;

  debug_message("parsing body\n");

  /* parse body */
  if (p->type == _RGB24 ? !parse_body_rgb24(p, st, colortable, cpp) :
      !parse_body_index(p, st, colortable, cpp)) {
    hash_destroy(colortable);
    return 0;
  }

  hash_destroy(colortable);
  debug_message("XPM file parse done.\n");

  return 1;
}

DEFINE_LOADER_PLUGIN_IDENTIFY(p, st, vw __attribute__((unused)), c __attribute__((unused)), priv __attribute__((unused)))
{
  if (!load_image(p, st))
    return LOAD_NOT;

  return LOAD_OK;
}

DEFINE_LOADER_PLUGIN_LOAD(p, st, vw
#if !defined(IDENTIFY_BEFORE_LOAD)
__attribute__((unused))
#endif
, c
#if !defined(IDENTIFY_BEFORE_LOAD)
__attribute__((unused))
#endif
, priv
#if !defined(IDENTIFY_BEFORE_LOAD)
__attribute__((unused))
#endif
)
{
  debug_message("xpm loader: load() called\n");

#ifdef IDENTIFY_BEFORE_LOAD
  {
    LoaderStatus status;

    if ((status = identify(p, st, vw, c, priv)) != LOAD_OK)
      return status;
    stream_rewind(st);
  }
#endif

  if (!load_image(p, st))
    return LOAD_ERROR;

  return LOAD_OK;
}
