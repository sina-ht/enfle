/*
 * hashtest.c
 */

#include <stdio.h>

#include "common.h"

#include "hash.h"

int
main(int argc, char **argv)
{
  Hash *h;
  Dlist *dl;
  Dlist_data *dd;
  unsigned char *k;
  void *d;

  h = hash_create(1024);
  hash_set(h, "key1", "data1");
  hash_set(h, "key2", "data2");
  if (hash_set(h, "3key", "3data") != 1) {
    fprintf(stderr, "bug... cannot register 3key\n");
    exit(1);
  }
  if (hash_define(h, "3key", "data3") != -1) {
    fprintf(stderr, "bug... can register already registered 3key\n");
    exit(1);
  }

  hash_iter(h, dl, dd, k, d)
    printf("%s %s\n", k, (char *)d);
  printf("\n");

  hash_delete(h, "key1", 0);

  hash_set(h, "key3", "data3");

  hash_iter(h, dl, dd, k, d)
    printf("%s %s\n", k, (char *)d);
  printf("\n");

  hash_destroy(h, 0);

  return 0;
}
