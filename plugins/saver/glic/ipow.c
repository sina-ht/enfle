/*
 * ipow.c
 */

#include "ipow.h"

unsigned int
ipow(unsigned int base, unsigned int exponent)
{
  unsigned int ret, tmp;

  for (ret = 1; exponent; exponent >>= 1) {
    if (exponent & 1) {
      tmp = ret * base;
      if (tmp / base != ret)
	return 0;
      ret = tmp;
    }
    base *= base;
  }

  return ret;
}
