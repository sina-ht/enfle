/*
 * dlist.h -- double-linked list data structure
 * (C)Copyright 1998, 99, 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Thu Jan  4 03:27:03 2001.
 * $Id: dlist.h,v 1.2 2001/01/06 23:55:47 sian Exp $
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

#ifndef _DLIST_H_
#define _DLIST_H_

typedef struct _dlist_data {
  void *data;
  struct _dlist_data *prev;
  struct _dlist_data *next;
} Dlist_data;

typedef struct _dlist Dlist;
struct _dlist {
  int ndata;
  Dlist_data *top;
  Dlist_data *head;

  Dlist_data *(*add)(Dlist *, void *);
  Dlist_data *(*add_str)(Dlist *, char *);
  int (*delete_item)(Dlist *, Dlist_data *);
  int (*move_to_top)(Dlist *, Dlist_data *);
  Dlist_data *(*get_top)(Dlist *);
  Dlist_data *(*get_head)(Dlist *);
  int (*get_datasize)(Dlist *);
  int (*destroy)(Dlist *);
};

#define dlist_add(dl, d) (dl)->add((dl), (d))
#define dlist_add_str(dl, d) (dl)->add_str((dl), (d))
#define dlist_delete(dl, dd) (dl)->delete_item((dl), (dd))
#define dlist_move_to_top(dl, dd) (dl)->move_to_top((dl), (dd))
#define dlist_get_top(dl) (dl)->get_top((dl))
#define dlist_get_head(dl) (dl)->get_head((dl))
#define dlist_get_datasize(dl) (dl)->get_datasize((dl))
#define dlist_destroy(dl) (dl)->destroy((dl))

#define dlist_next(dd) ((dd)->next)
#define dlist_prev(dd) ((dd)->prev)
#define dlist_data(dd) ((dd)->data)
#define dlist_iter(dl,dd) for (dd = (dl)->get_top(dl); dd != NULL; dd = dlist_next((dd)))

Dlist *dlist_create(void);

#endif
