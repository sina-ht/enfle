/*
 * loader.h -- loader plugin interface header
 * (C)Copyright 2000, 2001 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Jul  3 20:19:23 2001.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef _LOADER_H
#define _LOADER_H

#include "enfle-plugins.h"
#include "loader-extra.h"
#include "stream.h"
#include "image.h"
#include "video.h"

int loader_identify(EnflePlugins *, Image *, Stream *, VideoWindow *, Config *);
LoaderStatus loader_load(EnflePlugins *, char *, Image *, Stream *, VideoWindow *, Config *);

#endif
