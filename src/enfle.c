/*
 * enfle.c -- graphic loader Enfle main program
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Sat Sep 30 03:49:39 2000.
 * $Id: enfle.c,v 1.1 2000/09/30 17:36:36 sian Exp $
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
#include <string.h>

#include "common.h"

#include "libconfig.h"
#include "enfle-plugin.h"
#include "ui.h"
#include "streamer.h"
#include "loader.h"
#include "archiver.h"

int
main(int argc, char **argv)
{
  UIData uidata;
  UI *ui;
  Config *c;
  Streamer *st;
  Loader *ld;
  Archiver *ar;
  Archive *a;
  int i;
  char *plugin_path, *path, *ext, *name, *ui_name;

  if (argc == 1) {
    printf(PROGNAME " version " VERSION "\n");
    printf("(C)Copyright 2000 by Hiroshi Takekawa\n\n");
    printf("%s imagefilename [imagefilename...]\n", argv[0]);
    return 1;
  }

  c = uidata.c = config_create();
  config_load(c, "enfle.rc");

  st = uidata.st = streamer_create();
  ui = ui_create();
  ld = uidata.ld = loader_create();
  ar = uidata.ar = archiver_create();

  /* scanning... */
  if ((plugin_path = config_get(c, "/enfle/plugins/dir")) == NULL) {
    fprintf(stderr, "plugins/dir not found: configuration error\n");
    return 1;
  }
  a = archive_create();
  archive_read_directory(a, plugin_path, 0);
  path = archive_iteration_start(a);
  while (path) {
    if ((ext = strrchr(path, '.')) && !strncmp(ext, ".so", 3)) {
      PluginType type;
      void *plugin;

      if ((plugin = enfle_plugin_load(path, &type)) == NULL) {
	fprintf(stderr, "enfle_plugin_load %s failed.\n", path);
	return 1;
      }
      switch (type) {
      case ENFLE_PLUGIN_LOADER:
	name = loader_load(ld, plugin);
	printf("%s by %s\n",
	       loader_get_description(ld, name),
	       loader_get_author(ld, name));
	break;
      case ENFLE_PLUGIN_STREAMER:
	name = streamer_load(st, plugin);
	printf("%s by %s\n",
	       streamer_get_description(st, name),
	       streamer_get_author(st, name));
	break;
      case ENFLE_PLUGIN_UI:
	name = ui_load(ui, plugin);
	printf("%s by %s\n",
	       ui_get_description(ui, name),
	       ui_get_author(ui, name));
	break;
      case ENFLE_PLUGIN_ARCHIVER:
	name = archiver_load(ar, plugin);
	printf("%s by %s\n",
	       archiver_get_description(ar, name),
	       archiver_get_author(ar, name));
	break;
      case ENFLE_PLUGIN_SAVER:
      case ENFLE_PLUGIN_EFFECT:
      case ENFLE_PLUGIN_PLAYER:
	fprintf(stderr, "not yet implemented.\n");
	break;
      default:
	fprintf(stderr, "unknown plugin type or not enfle plugin.\n");
	break;
      }
    }
    path = archive_iteration_next(a);
  }
  archive_destroy(a);

  uidata.a = archive_create();

  if (strcmp(argv[1], "-") == 0) {
    archive_add(uidata.a, argv[1], strdup(argv[1]));
  } else {
    for (i = 1; i < argc; i++) {
      if (strcmp(argv[i], "-") == 0) {
	fprintf(stderr, "Cannot specify stdin(-) with other files. ignored.\n");
      } else {
	archive_add(uidata.a, argv[i], strdup(argv[i]));
      }
    }
  }

  if ((ui_name = config_get(c, "/enfle/plugins/ui/default")) == NULL) {
    fprintf(stderr, "configuration error\n");
    return 1;
  }
  ui_call(ui, ui_name, &uidata);

  archive_destroy(uidata.a);
  archiver_destroy(ar);
  streamer_destroy(st);
  config_destroy(c);
  loader_destroy(ld);
  ui_destroy(ui);

  return 0;
}
