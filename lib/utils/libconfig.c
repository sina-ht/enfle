/*
 * libconfig.c -- configuration file manipulation library
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Aug 18 22:13:33 2002.
 * $Id: libconfig.c,v 1.24 2002/08/18 13:28:20 sian Exp $
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
#include "common.h"

#include "libstring.h"
#include "libconfig.h"
#include "misc.h"
#include "stdio-support.h"

#define DQUOTATION 0x22
#define SQUOTATION 0x27

Config *
config_create(void)
{
  Config *c;

  if ((c = calloc(1, sizeof(Config))) == NULL)
    return NULL;
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
  fatal("Parse error: %s in %s\n", p, string_get(path));
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

static int
set_internal(Config *c, String *config_path, char *path, char *remain, int is_list)
{
  String *value_path;
  int f;

  if ((value_path = string_dup(config_path)) == NULL)
    fatal("libconfig: %s: No enough memory.\n", __FUNCTION__);
  if (path != NULL) {
    string_cat(value_path, "/");
    string_cat(value_path, (const char *)path);
  }

  if (is_list) {
    f = config_set_list(c, string_get(value_path), remain);
  } else {
    if (*remain == '"') {
      char *end, *quoted;

      if ((end = strrchr((const char *)remain, '"')) == NULL || remain == end)
	fatal("libconfig: %s: Non-terminated double quoted string.\n", __FUNCTION__);
      if ((quoted = malloc(end - remain)) == NULL)
	fatal("libconfig: %s: No enough memory\n", __FUNCTION__);
      if (*(end + 1) != '\n' && *(end + 1) != '\0')
	show_message("libconfig: %s: Ignored trailing garbage: %s\n", __FUNCTION__, end + 1);
      memcpy(quoted, remain + 1, end - remain - 1);
      quoted[end - remain - 1] = '\0';
      f = config_set_str(c, string_get(value_path), quoted);
    } else if (isdigit(*remain) || ((*remain == '+' || *remain == '-') && isdigit(*(remain + 1)))) {
      f = config_set_int(c, string_get(value_path), atoi(remain));
    } else {
      char *p = strdup(remain);

      if (!p)
	fatal("libconfig: %s: No enough memory\n", __FUNCTION__);
      f = config_set_str(c, string_get(value_path), p);
    }
  }
  string_destroy(value_path);

  return f;
}

/* methods */

int
config_load(Config *c, const char *filepath)
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

    p = misc_remove_preceding_space(p);

    switch ((int)p[0]) {
    case '\0':
    case '%':
    case ';':
      break;
    case '/':
      if (p[1] != '/') {
	show_message("Missing '/'\n");
	parse_error(p, config_path);
      }
      break;
    case '#':
      /* special directives */
      if (strncasecmp(&p[1], "include", 7) == 0) {
	char *path;

	path = get_token(p + 8);
	config_load(c, path);
	free(path);
      } else {
	show_message("Unknown directive\n");
	parse_error(p, config_path);
      }
      break;
    default:
      /* normal */
      {
	char *path, *op, *remain;

	path = get_token(p);

	if (strlen(path) < strlen(p)) {
	  op = get_token(p + strlen(path) + 1);
	  remain = strdup(p + strlen(path) + 1 + strlen(op));
	  op = misc_remove_preceding_space(op);
	  remain = misc_remove_preceding_space(remain);
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
	  char *pos;

	  if ((pos = strrchr(string_get(config_path), '/')) == NULL) {
	    show_message("Missing '/'.\n");
	    parse_error(p, config_path);
	  }
	  string_shrink(config_path, pos - string_get(config_path));
	} else if (strcmp(op, ":=") == 0) {
	  (void)set_internal(c, config_path, path, remain, 1);
	} else if (strcmp(op, "=") == 0) {
	  if (!set_internal(c, config_path, path, remain, 0))
	    warning_fnc("set_internal(%s/%s, %s) failed\n", string_get(config_path), path, remain);
	} else {
	  show_message("Syntax error.\n");
	  parse_error(p, config_path);
	}
	free(path);
	free(op);
	free(remain);
      }
      break;
    }
    free(p);
  }
}

int
config_save(Config *c, char *path)
{
  err_message("Not implemented yet\n");
  return 0;
}

int
config_parse(Config *c, char *str)
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

  r = (isdigit(*value) || ((*value == '+' || *value == '-') && isdigit(*(value + 1)))) ? config_set_int(c, name, atoi(value)) : config_set_str(c, name, (char *)value);
  free(name);

  return r;
}

void *
config_get(Config *c, const char *path)
{
  return hash_lookup_str(c->hash, (char *)path);
}

int
config_set(Config *c, char *path, void *value)
{
  return hash_set_str(c->hash, path, value);
}

char *
config_get_str(Config *c, const char *path)
{
  return hash_lookup_str(c->hash, path);
}

int
config_set_str(Config *c, char *path, char *value)
{
  return hash_set_str(c->hash, path, value);
}

int
config_get_boolean(Config *c, const char *path, int *is_success)
{
  char *tmp;

  if ((tmp = config_get_str(c, path)) == NULL) {
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

int
config_set_boolean(Config *c, char *path, int boolean)
{
  char *tmp;

  if ((tmp = strdup(boolean ? "yes" : "no")) == NULL)
    return 0;
  return config_set_str(c, path, tmp);
}

static char *
check_typed_data(Config *c, const char *path, const char *type)
{
  char *p;

  if ((p = (char *)config_get(c, path)) == NULL)
    return NULL;
  if (*p != '\0' || memcmp(p + 1, type, 3))
    return NULL;

  return p;
}

static char *
setup_typed_data(Config *c, char *path, const char *type, int size)
{
  char *p;

  if ((p = malloc(4 + size)) == NULL)
    return NULL;
  p[0] = '\0';
  memcpy(p + 1, type, 3);

  return p;
}

int
config_get_int(Config *c, const char *path, int *is_success)
{
  char *p;

  if ((p = check_typed_data(c, path, "INT")) == NULL) {
    *is_success = 0;
    return 0;
  }
  *is_success = 1;

  return *((int *)(p + 4));
}

int
config_set_int(Config *c, char *path, int value)
{
  char *p;

  if ((p = setup_typed_data(c, path, "INT", sizeof(int))) == NULL)
    return 0;
  *((int *)(p + 4)) = value;

  return config_set(c, path, (void *)p);
}

char **
config_get_list(Config *c, const char *path, int *is_success)
{
  char *p;

  if ((p = check_typed_data(c, path, "LST")) == NULL) {
    *is_success = 0;
    return 0;
  }
  *is_success = 1;

  return *((char ***)(p + 4));
}

static void
list_destroy(void *arg)
{
  char *p = (char *)arg;
  char **list = *((char ***)(p + 4));

  bug_on(*p != '\0' || memcmp(p + 1, "LST", 3));
  show_message_fnc("OK\n");

  misc_free_str_array(list);
  free(p);
}

int
config_set_list(Config *c, char *path, char *lstr)
{
  char *p;
  char **list;

  if ((list = misc_str_split(lstr, ':')) == NULL)
    return 0;
  if ((p = setup_typed_data(c, path, "LST", sizeof(char ***))) == NULL) {
    misc_free_str_array(list);
    return 0;
  }
  *((char ***)(p + 4)) = list;

  return hash_set_object(c->hash, (void *)path, strlen(path) + 1, p, list_destroy);
}

void
config_destroy(Config *c)
{
  hash_destroy(c->hash);
  free(c);
}
