/*
 * ipow.c
 */

#include "ipow.h"

int
ipow(int base, int exponent)
{
  int ret;

  for (ret = 1; exponent; exponent >>= 1) {
    if (exponent & 1)
      ret *= base;
    base *= base;
  }

  return ret;
}
