/*
 * floorlog2.c
 */

#include "floorlog2.h"

int
floorlog2(unsigned int n)
{
  int i;

  if (n == 0)
    return -1;

  i = 0;
  while ((n >>= 1))
    i++;
  return i;
}
