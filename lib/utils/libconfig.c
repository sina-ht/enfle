/*
 * libconfig.c -- configuration file manipulation library
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Sep 18 13:54:34 2001.
 * $Id: libconfig.c,v 1.14 2001/09/18 05:22:24 sian Exp $
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

#include <stdlib.h>
#include <ctype.h>

#define REQUIRE_STRING_H
#include "compat.h"

#define REQUIRE_FATAL
#include "common.h"

#include "libstring.h"
#include "libconfig.h"
#include "stdio-support.h"

static int load(Config *, const char *);
static int save(Config *, char *);
static int parse(Config *, char *);
static void *get(Config *, const char *);
static int set(Config *, char *, void *);
static unsigned char *get_str(Config *, const char *);
static int set_str(Config *, char *, unsigned char *);
static int get_int(Config *, const char *, int *);
static int set_int(Config *, char *, int);
static int get_boolean(Config *, const char *, int *);
static int set_boolean(Config *, char *, int);
static void destroy(Config *);

#define DQUOTATION 0x22
#define SQUOTATION 0x27

static Config config_template = {
  hash: NULL,

  load: load,
  save: save,
  parse: parse,
  get: get,
  set: set,
  get_str: get_str,
  set_str: set_str,
  get_int: get_int,
  set_int: set_int,
  get_boolean: get_boolean,
  set_boolean: set_boolean,
  destroy: destroy
};

Config *
config_create(void)
{
  Config *c;

  if ((c = calloc(1, sizeof(Config))) == NULL)
    return NULL;

  memcpy(c, &config_template, sizeof(Config));

  if ((c->hash = hash_create(LIBCONFIG_HASH_SIZE)) == NULL) {
    free(c);
    return NULL;
  }

  return c;
}

/* internal functions */

static inline void parse_error(char *, String *) __attribute__ ((noreturn));
static inline void
parse_error(char *p, String *path)
{
  fatal(1, "Parse error: %s in %s\n", p, string_get(path));
}

static char *
get_token(char *p)
{
  enum _token_state { FIRST, NORMAL, SQUOTED, DQUOTED, END } state = FIRST;
  char *q = NULL, *t;
  int size;

  while (state != END) {
    if (*p == '\0')
      break;
    switch (state) {
    case FIRST:
      if (*p == DQUOTATION) {
	state = DQUOTED;
	p++;
	q = p;
      } else if (*p == SQUOTATION) {
	state = SQUOTED;
	p++;
	q = p;
      } else {
	state = NORMAL;
	q = p;
	p++;
      }
      break;
    case NORMAL:
      if (isspace((int)*p))
	state = END;
      else
	p++;
      break;
    case DQUOTED:
      if (*p == DQUOTATION)
	state = END;
      else
	p++;
      break;
    case SQUOTED:
      if (*p == SQUOTATION)
	state = END;
      else
	p++;
      break;
    case END:
      break;
    }
  }

  size = p - q;
  if ((t = calloc(1, size + 1)) == NULL)
    return NULL;
  strncpy(t, q, size);
  t[size] = '\0';

  return t;
}

static char *
remove_preceding_space(char *p)
{
  char *q, *t;

  for (q = p; isspace((int)*q); q++) ;

  if ((t = strdup(q)) == NULL)
    return NULL;

  free(p);

  return t;
}

static int
set_internal(Config *c, String *config_path, char *path, char *remain)
{
  String *value_path;
  int f;

  if ((value_path = string_dup(config_path)) == NULL)
    fatal(1, "libconfig: " __FUNCTION__ "(): No enough memory\n");
  if (path != NULL) {
    string_cat(value_path, "/");
    string_cat(value_path, path);
  }

  if (*remain == '"') {
    char *end, *quoted;

    if ((end = strrchr(remain, '"')) == NULL || remain == end)
      fatal(1, "libconfig: " __FUNCTION__ "(): Non-terminated double quoted string.\n");
    if ((quoted = malloc(end - remain)) == NULL)
      fatal(1, "libconfig: " __FUNCTION__ "(): No enough memory\n");
    if (*(end + 1) != '\n' && *(end + 1) != '\0')
      show_message("libconfig: " __FUNCTION__ "(): Ignored trailing garbage: %s\n", end + 1);
    memcpy(quoted, remain + 1, end - remain - 1);
    quoted[end - remain - 1] = '\0';
    f = set_str(c, string_get(value_path), quoted);
  } else if (isdigit(*remain) || ((*remain == '+' || *remain == '-') && isdigit(*(remain + 1)))) {
    f = set_int(c, string_get(value_path), atoi(remain));
  } else {
    f = set_str(c, string_get(value_path), strdup(remain));
  }
  string_destroy(value_path);

  return f;
}

/* methods */

static int
load(Config *c, const char *filepath)
{
  String *config_path;
  FILE *fp;
  char *p;
  int f;

  if ((fp = fopen(filepath, "r")) == NULL)
    return 0;

  if ((config_path = string_create()) == NULL) {
    fclose(fp);
    return 0;
  }

  for (;;) {
    if ((p = stdios_gets(fp)) == NULL) {
      f = (feof(fp) != 0) ? 1 : 0;
      fclose(fp);
      string_destroy(config_path);
      return f;
    }

    if (p[strlen(p) - 1] == '\n')
      p[strlen(p) - 1] = '\0';

    p = remove_preceding_space(p);

    switch ((int)p[0]) {
    case '\0':
    case '%':
    case ';':
      break;
    case '/':
      if (p[1] != '/')
	parse_error(p, config_path);
      break;
    case '#':
      /* special directives */
      if (strncasecmp(&p[1], "include", 7) == 0) {
	char *filepath;

	filepath = get_token(p + 8);
	load(c, filepath);
	free(filepath);
      } else
	parse_error(p, config_path);
      break;
    default:
      /* normal */
      {
	char *path, *op, *remain;

	path = get_token(p);
	if (strlen(path) < strlen(p)) {
	  op = get_token(p + strlen(path) + 1);
	  remain = strdup(p + strlen(path) + 1 + strlen(op));
	  op = remove_preceding_space(op);
	  remain = remove_preceding_space(remain);
	} else {
	  op = strdup("");
	  remain = strdup("");
	}

	if (strcmp(path, "}") == 0) {
	  free(op);
	  op = path;
	  path = strdup("");
	}

	if (strcmp(op, "{") == 0) {
	  string_cat(config_path, "/");
	  string_cat(config_path, path);
	} else if (strcmp(op, "}") == 0) {
	  unsigned char *pos;

	  if ((pos = strrchr(string_get(config_path), '/')) == NULL)
	    parse_error(p, config_path);
	  string_shrink(config_path, pos - string_get(config_path));
	} else if (strcmp(op, "=") == 0) {
	  (void)set_internal(c, config_path, path, remain);
	} else
	  parse_error(p, config_path);
	free(path);
	free(op);
	free(remain);
      }
      break;
    }
    free(p);
  }
}

static int
save(Config *c, char *path)
{
  fprintf(stderr, "Not implemented yet\n");
  return 0;
}

static int
parse(Config *c, char *str)
{
  char *name, *namestart, *nameend;
  char *value, *valuestart;
  int namelen, r;

  namestart = str;
  for (nameend = namestart; *nameend; nameend++)
    if (*nameend == '=')
      break;
  if (!*nameend)
    return 0;
  valuestart = nameend + 1;
  while (isspace(*(nameend - 1)))
    nameend--;
  namelen = nameend - namestart;
  if ((name = malloc(namelen + 1)) == NULL)
    return 0;
  memcpy(name, namestart, namelen);
  name[namelen] = '\0';
  while (isspace(*valuestart))
    valuestart++;
  value = strdup(valuestart);

  r = (isdigit(*value) || ((*value == '+' || *value == '-') && isdigit(*(value + 1)))) ? set_int(c, name, atoi(value)) : set_str(c, name, (unsigned char *)value);
  free(name);

  return r;
}

static void *
get(Config *c, const char *path)
{
  return hash_lookup_str(c->hash, (unsigned char *)path);
}

static int
set(Config *c, char *path, void *value)
{
  return hash_set_str(c->hash, path, value);
}

static unsigned char *
get_str(Config *c, const char *path)
{
  return (unsigned char *)hash_lookup_str(c->hash, (unsigned char *)path);
}

static int
set_str(Config *c, char *path, unsigned char *value)
{
  return hash_set_str(c->hash, path, value);
}

static int
get_int(Config *c, const char *path, int *is_success)
{
  char *p;

  *is_success = 0;
  if ((p = (char *)get(c, path)) == NULL)
    return 0;
  if (*p != '\0' || memcmp(p + 1, "INT", 3))
    return 0;

  *is_success = 1;
  return *((int *)(p + 4));
}

static int
set_int(Config *c, char *path, int value)
{
  char *p;

  if ((p = malloc(4 + sizeof(int))) == NULL)
    return 0;
  p[0] = '\0';
  memcpy(p + 1, "INT", 3);
  *((int *)(p + 4)) = value;

  return set(c, path, (void *)p);
}

static int
get_boolean(Config *c, const char *path, int *is_success)
{
  char *tmp;

  if ((tmp = get_str(c, path)) == NULL) {
    *is_success = 0;
    return 0;
  } else if (strcasecmp(tmp, "yes") == 0 || strcasecmp(tmp, "true") == 0) {
    *is_success = 1;
    return 1;
  } else if (strcasecmp(tmp, "no") == 0  || strcasecmp(tmp, "false") == 0) {
    *is_success = 1;
    return 0;
  }

  *is_success = -1;
  return 0;
}

static int
set_boolean(Config *c, char *path, int boolean)
{
  char *tmp;

  if ((tmp = strdup(boolean ? "yes" : "no")) == NULL)
    return 0;
  return set_str(c, path, tmp);
}

static void
destroy(Config *c)
{
  hash_destroy(c->hash, 1);
  free(c);
}
