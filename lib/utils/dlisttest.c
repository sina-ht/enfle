/*
 * dlisttest.c
 */

#include <stdio.h>
#include <string.h>

#include "common.h"

#include "dlist.h"

static void
print_list(Dlist *p)
{
  Dlist_data *t;

  dlist_iter(p, t)
    printf("%s ", (char *)dlist_data(t));
  printf("\n");
}

int
main(int argc, char **argv)
{
  Dlist *p;
  Dlist_data *t;

  if (!(p = dlist_create()))
    exit(1);

  p->add_str(p, strdup("1"));
  p->add_str(p, strdup("3"));
  p->add_str(p, strdup("5"));
  p->add_str(p, strdup("7"));
  p->add_str(p, strdup("9"));
  print_list(p);

  t = p->get_top(p);
  t = dlist_next(t);
  p->move_to_top(p, t);
  print_list(p);
  p->delete(p, dlist_next(p->get_top(p)));
  print_list(p);
  p->delete(p, dlist_next(dlist_next(p->get_top(p))));
  print_list(p);

  p->destroy(p);

  return 0;
}
