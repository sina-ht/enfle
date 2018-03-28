/*
 * module.h
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Jan  6 01:39:21 2001.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include "windef.h"

int module_register(const char *, HMODULE);
int module_deregister(const char *);
HMODULE module_lookup(const char *);
