/*
 * dlist.c -- double-linked list data structure
 * (C)Copyright 1998, 99, 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sun Jan 14 10:55:30 2001.
 * $Id: dlist.c,v 1.5 2001/01/14 15:20:13 sian Exp $
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

#define REQUIRE_STRING_H
#include "compat.h"

#include "common.h"

#include "dlist.h"

static Dlist_data *add(Dlist *, void *);
static Dlist_data *add_str(Dlist *, char *);
static int delete_item(Dlist *, Dlist_data *);
static int move_to_top(Dlist *, Dlist_data *);
static void set_compfunc(Dlist *, Dlist_compfunc);
static int do_sort(Dlist *);
static Dlist_data *get_top(Dlist *);
static Dlist_data *get_head(Dlist *);
static int get_datasize(Dlist *);
static int destroy(Dlist *);

static Dlist dlist_template = {
  ndata: 0,
  top: NULL,
  head: NULL,
  add: add,
  add_str: add_str,
  delete_item: delete_item,
  move_to_top: move_to_top,
  set_compfunc: set_compfunc,
  do_sort: do_sort,
  get_top: get_top,
  get_head: get_head,
  get_datasize: get_datasize,
  destroy: destroy
};

Dlist *
dlist_create(void)
{
  Dlist *p;

  if ((p = calloc(1, sizeof(Dlist))) == NULL)
    return NULL;

  memcpy(p, &dlist_template, sizeof(Dlist));

  return p;
}

static int
destroy(Dlist *p)
{
  Dlist_data *t, *tn;

  for (t = p->top; t != NULL; t = tn) {
    tn = t->next;
    if (!delete_item(p, t))
      return 0;
  }
  free(p);

  return 1;
}

static Dlist_data *
add(Dlist *p, void *d)
{
  Dlist_data *t;

  if ((t = (Dlist_data *)calloc(1, sizeof(Dlist_data))) == NULL)
    return NULL;

  t->data = d;

  if (p->top == NULL)
    p->top = t;
  else {
    p->head->next = t;
    t->prev = p->head;
  }
  p->head = t;
  p->ndata++;

  return t;
}

static Dlist_data *
add_str(Dlist *p, char *str)
{
  Dlist_data *t;

  if (str == NULL)
    return 0;

  if ((t = (Dlist_data *)calloc(1, sizeof(Dlist_data))) == NULL)
    return NULL;

  if ((t->data = strdup(str)) == NULL) {
    free(t);
    return NULL;
  }

  if (p->top == NULL)
    p->top = t;
  else {
    p->head->next = t;
    t->prev = p->head;
  }
  p->head = t;
  p->ndata++;

  return t;
}

static void
detach(Dlist *p, Dlist_data *t)
{
  if (t->prev == NULL && t->next == NULL)
    p->top = p->head = NULL;
  else {
    if (t->prev)
      t->prev->next = t->next;
    else
      p->top = t->next;

    if (t->next)
      t->next->prev = t->prev;
    else 
      p->head = t->prev;
  }
}

static int
delete_item(Dlist *p, Dlist_data *t)
{
  if (p == NULL || t == NULL)
    return 0;

  detach(p, t);

  if (t->data != NULL)
    free(t->data);
  free(t);
  p->ndata--;

  return 1;
}

static int
move_to_top(Dlist *p, Dlist_data *t)
{
  if (p == NULL || t == NULL)
    return 0;
  if (p->top == t)
    return 1;

  detach(p, t);

  t->prev = NULL;
  p->top->prev = t;
  t->next = p->top;
  p->top = t;

  return 1;
}

static void
set_compfunc(Dlist *dl, Dlist_compfunc cf)
{
  dl->cf = cf;
}

#if 0
static int
validate(Dlist *dl)
{
  Dlist_data *i;

  for (i = get_top(dl); i && i != get_head(dl); i = i->next);
  if (i != get_head(dl))
    return 0;
  for (i = get_head(dl); i && i != get_top(dl); i = i->prev);
  if (i != get_top(dl))
    return 0;
  return 1;
}
#endif

static int
do_sort(Dlist *dl)
{
  int i;
  Dlist_data *t;
  void **tmp;

  if (get_datasize(dl) <= 1)
    return 1;

  if ((tmp = calloc(get_datasize(dl), sizeof(void *))) == NULL)
    return 0;
  t = get_top(dl);
  for (i = 0; i < get_datasize(dl); i++) {
    tmp[i] = t->data;
    t = t->next;
  }
  qsort(&tmp[0], get_datasize(dl), sizeof(void *), dl->cf);
  t = get_top(dl);
  for (i = 0; i < get_datasize(dl); i++) {
    t->data = tmp[i];
    t = t->next;
  }
  free(tmp);

  return 1;
}

static Dlist_data *
get_top(Dlist *p)
{
  return p->top;
}

static Dlist_data *
get_head(Dlist *p)
{
  return p->head;
}

static int
get_datasize(Dlist *p)
{
  return p->ndata;
}
