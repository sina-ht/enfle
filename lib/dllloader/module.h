/*
 * module.h
 */

#include "windef.h"

int module_register(const char *, HMODULE);
int module_deregister(const char *);
HMODULE module_lookup(const char *);
