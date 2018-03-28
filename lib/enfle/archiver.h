/*
 * archiver.h -- archiver header
 * (C)Copyright 2000, 2001, 2002 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Feb  9 12:26:15 2002.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _ARCHIVER_H
#define _ARCHIVER_H

#include "enfle-plugins.h"
#include "archiver-extra.h"
#include "archive.h"
#include "stream.h"
#include "utils/libconfig.h"

int archiver_identify(EnflePlugins *, Archive *, Stream *, Config *);
ArchiverStatus archiver_open(EnflePlugins *, Archive *, char *, Stream *);

#endif
