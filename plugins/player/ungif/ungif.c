/*
 * ungif.c -- gif player plugin, which exploits libungif.
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Mon Oct  9 17:33:55 2000.
 * $Id: ungif.c,v 1.2 2000/10/09 20:23:43 sian Exp $
 *
 * NOTES:
 *  This file does NOT include LZW code.
 *  This plugin is incomplete, but works.
 *  Requires libungif version 3.1.0 or later(4.1.0 recommended).
 *
 *             The Graphics Interchange Format(c) is
 *       the Copyright property of CompuServe Incorporated.
 * GIF(sm) is a Service Mark property of CompuServe Incorporated.
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

#include <stdio.h>
#include <stdlib.h>
#include <gif_lib.h>

#include "common.h"

#include "player-plugin.h"

static PlayerStatus identify(Image *, Stream *);
static PlayerStatus play(Image *, Stream *);

static PlayerPlugin plugin = {
  type: ENFLE_PLUGIN_PLAYER,
  name: "UNGIF",
  description: "UNGIF Player plugin version 0.1",
  author: "Hiroshi Takekawa",

  identify: identify,
  play: play
};

void *
plugin_entry(void)
{
  PlayerPlugin *pp;

  if ((pp = (PlayerPlugin *)calloc(1, sizeof(PlayerPlugin))) == NULL)
    return NULL;
  memcpy(pp, &plugin, sizeof(PlayerPlugin));

  return (void *)pp;
}

void
plugin_exit(void *p)
{
  free(p);
}

/* for internal use */

static int
ungif_input_func(GifFileType *GifFile, GifByteType *p, int s)
{
  Stream *st = (Stream *)GifFile->UserData;

  return stream_read(st, p, s);
}

static PlayerStatus
play_image(Image *p, Stream *st)
{
  return PLAY_ERROR;
}

/* methods */

static PlayerStatus
identify(Image *p, Stream *st)
{
  char buf[3];

  if (stream_read(st, buf, 3) != 3)
    return PLAY_NOT;

  if (memcmp(buf, "GIF", 3) != 0)
    return PLAY_NOT;

  if (stream_read(st, buf, 3) != 3)
    return PLAY_NOT;

  if (memcmp(buf, "87a", 3) == 0)
    return PLAY_OK;
  if (memcmp(buf, "89a", 3) == 0)
    return PLAY_OK;

  show_message("GIF detected, but version is neither 87a nor 89a.\n");

  return PLAY_OK;
}

static PlayerStatus
play(Image *p, Stream *st)
{
  debug_message("ungif player: play() called\n");

#ifdef IDENTIFY_BEFORE_PLAY
  {
    PlayerStatus status;

    if ((status = identify(p, st)) != PLAY_OK)
      return status;
    stream_rewind(st);
  }
#endif

  return play_image(p, st);
}
