/*
 * misc.c -- miscellaneous routines
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Sep 13 02:52:41 2003.
 * $Id: misc.c,v 1.9 2003/10/12 04:00:40 sian Exp $
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

#include "misc.h"

char *
misc_basename(char *path)
{
  char *ptr = path + strlen(path);

  while (path < ptr && *--ptr != '/') ;
  if (*ptr == '/')
    ptr++;

  return ptr;
}

char *
misc_get_ext(const char *path, int if_lower)
{
  char *p, *ret, *r;

  if ((p = strrchr(path, '.')) == NULL)
    return NULL;
  p++;

  ret = r = malloc(strlen(p) + 1);
  if (!if_lower)
    return strcpy(r, p);
  while (*p) {
    *r++ = (char)tolower((int)*p);
    p++;
  }
  *r = '\0';

  return ret;
}

char *
misc_trim_ext(const char *path, const char *ext)
{
  char *p;
  int len;

  if ((p = strrchr(path, '.')) == NULL)
    return strdup(path);
  if (ext && strcasecmp(p + 1, ext))
    return strdup(path);
  len = p - path;
  if ((p = (char *)malloc(len + 1)) == NULL)
    return NULL;
  memcpy(p, path, len);
  p[len] = '\0';

  return p;
}

/* replace or add extension like 'png' */
char *
misc_replace_ext(char *filename, char *ext)
{
  char *ptr = strrchr(filename, '.');
  char *new;
  int base_len;

  if (ptr == NULL) {
    base_len = strlen(filename);
  } else {
    base_len = (int)(ptr - filename);
  }
  if ((new = malloc(base_len + 1 + strlen(ext) + 1)) == NULL)
    return NULL;
  if (ptr != filename)
    memcpy(new, filename, base_len);
  new[base_len] = '.';
  strcpy(new + base_len + 1, ext);

  return new;
}

/* Make a directory pathname canonical */
char *
misc_canonical_pathname(char *pathname)
{
  int l = strlen(pathname);
  char *p;

  if (pathname[l - 1] != '/') {
    /* Pathname should be ended with '/'. */
    if ((p = malloc(l + 2)) == NULL)
      return NULL;
    strcpy(p, pathname);
    strcat(p, "/");
  } else if (pathname[l - 2] == '/') {
    /* Pathname should be ended with single '/'. */
    l--;
    while (pathname[l - 2] == '/') {
      pathname[l - 1] = '\0';
      l--;
    }
    if ((p = malloc(l + 1)) == NULL)
      return NULL;
    strncpy(p, pathname, l);
  } else {
    if ((p = strdup(pathname)) == NULL)
      return NULL;
  }

  return p;
}

static int
count_token(char *str, char delimiter)
{
  int count = 0;

  while (*str != '\0')
    if (*str++ == delimiter)
      count++;

  return count;
}

char **
misc_str_split(char *str, char delimiter)
{
  int i, j, k, l;
  int count;
  char **ret;

  if (str == NULL)
    return NULL;

  count = count_token(str, delimiter) + 1;
  if ((ret = calloc(count + 1, sizeof(char *))) == NULL)
    return NULL;

  k = 0;
  for (i = 0; i < (int)strlen(str);) {
    j = i;
    while (str[i] != '\0' && str[i] != delimiter)
      i++;
    l = i - j;
    i++;
    if ((ret[k] = malloc(l + 1)) == NULL)
      goto free_and_return;
    if (l)
      strncpy(ret[k], str + j, l);
    ret[k++][l] = '\0';
  }
  bug_on(k > count);

  for (; k < count; k++) {
    debug_message("k = %d\n", k);
    if ((ret[k] = malloc(1)) == NULL)
      goto free_and_return;
    ret[k][0] = '\0';
  }
  ret[k] = NULL;

  return ret;

 free_and_return:
  for (k-- ; k >= 0; k--)
    free(ret[k]);
  free(ret);
  return NULL;
}

static int
count_token_delimiters(char *str, char *delimiters)
{
  char delimiter;
  int count = 0;
  int i;

  while (*str != '\0') {
    i = 0;
    while ((delimiter = delimiters[i++]) != '\0') {
      if (*str == delimiter) {
	count++;
	break;
      }
    }
    str++;
  }

  return count;
}

char **
misc_str_split_delimiters(char *str, char *delimiters, char **used_delim_r)
{
  int i, j, k, l;
  int count;
  char delimiter, *used_delim, **ret;

  if (str == NULL)
    return NULL;

  count = count_token_delimiters(str, delimiters) + 1;
  if ((ret = calloc(count + 1, sizeof(char *))) == NULL)
    return NULL;
  if ((*used_delim_r = calloc(count + 1, sizeof(char))) == NULL) {
    free(ret);
    return NULL;
  }
  used_delim = *used_delim_r;

  k = 0;
  for (i = 0; i < (int)strlen(str);) {
    j = i;
    while (1) {
      int m = 0;
      while ((delimiter = delimiters[m++]) != '\0') {
	if (str[i] == delimiter)
	  break;
      }
      /* '\0' is also checked */
      if (str[i] == delimiter)
	break;
      i++;
    }
    l = i - j;
    i++;
    if ((ret[k] = malloc(l + 1)) == NULL)
      goto free_and_return;
    used_delim[k] = delimiter;
    if (l)
      strncpy(ret[k], str + j, l);
    ret[k++][l] = '\0';
  }
  bug_on(k > count);

  for (; k < count; k++) {
    if ((ret[k] = malloc(1)) == NULL)
      goto free_and_return;
    ret[k][0] = '\0';
    used_delim[k] = delimiter;
  }
  ret[k] = NULL;
  used_delim[k] = '\0';

  return ret;

 free_and_return:
  for (k-- ; k >= 0; k--)
    free(ret[k]);
  free(ret);
  return NULL;
}

void
misc_free_str_array(char **strs)
{
  int i = 0;

  if (strs) {
    while (strs[i] != NULL)
      free(strs[i++]);
    free(strs);
  }
}

char *
misc_str_tolower(char *str)
{
  char *ret = str;

  if (str == NULL)
    return NULL;

  while (*str != '\0') {
    *str = tolower(*str);
    str++;
  }

  return ret;
}

char *
misc_remove_preceding_space(char *p)
{
  char *q, *t;

  for (q = p; isspace((int)*q); q++) ;
  if ((t = strdup(q)) == NULL)
    return NULL;
  free(p);

  return t;
}
