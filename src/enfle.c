/*
 * enfle.c -- graphic loader Enfle main program
 * (C)Copyright 2000 by Hiroshi Takekawa
 * This file is part of Enfle.
 *
 * Last Modified: Tue Oct 10 05:09:30 2000.
 * $Id: enfle.c,v 1.5 2000/10/09 20:29:56 sian Exp $
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

#define REQUIRE_STRING_H
#define REQUIRE_UNISTD_H
#include "compat.h"

#ifdef HAVE_GETOPT_H
#  include <getopt.h>
#endif

#include "common.h"

#include "libstring.h"
#include "libconfig.h"
#include "enfle-plugin.h"
#include "ui.h"
#include "streamer.h"
#include "loader.h"
#include "archiver.h"
#include "player.h"

typedef enum _argument_requirement {
  _NO_ARGUMENT,
  _REQUIRED_ARGUMENT,
  _OPTIONAL_ARGUMENT
} ArgumentRequirement;

typedef struct _option {
  char *longopt; /* not supported so far */
  char opt;
  ArgumentRequirement argreq;
  unsigned char *description;
} Option;

static Option enfle_options[] = {
  { "help", 'h', _NO_ARGUMENT,       "Show help message." },
  { "ui",   'u', _REQUIRED_ARGUMENT, "Specify which UI to use."  },
  { NULL }
};

static void
usage(void)
{
  int i;

  printf(PROGNAME " version " VERSION "\n");
  printf("(C)Copyright 2000 by Hiroshi Takekawa\n\n");
  printf("usage: enfle [options] [path...]\n");
  
  printf("Options:\n");
  i = 0;
  while (enfle_options[i].longopt != NULL) {
    printf(" %c(%s): \t%s\n",
	   enfle_options[i].opt, enfle_options[i].longopt, enfle_options[i].description);
    i++;
  }
}

static char *
gen_optstring(Option opt[])
{
  int i;
  String *s;
  char *optstr;

  s = string_create();
  i = 0;
  while (opt[i].longopt != NULL) {
    string_cat_ch(s, opt[i].opt);
    switch (opt[i].argreq) {
    case _NO_ARGUMENT:
      break;
    case _REQUIRED_ARGUMENT:
      string_cat_ch(s, ':');
      break;
    case _OPTIONAL_ARGUMENT:
      string_cat(s, "::");
      break;
    }
    i++;
  }

  optstr = strdup(string_get(s));

  string_destroy(s);

  return optstr;
}
  
int
main(int argc, char **argv)
{
  extern char *optarg;
  extern int optind;
  UIData uidata;
  UI *ui;
  Config *c;
  Streamer *st;
  Loader *ld;
  Archiver *ar;
  Player *player;
  Archive *a;
  int i, ch;
  char *plugin_path, *path, *ext, *name, *ui_name = NULL;
  char *optstr;

  optstr = gen_optstring(enfle_options);
  while ((ch = getopt(argc, argv, optstr)) != EOF) {
    i = 0;
    switch (ch) {
    case 'h':
      usage();
      if (ui_name)
	free(ui_name);
      return 0;
    case 'u':
      ui_name = strdup(optarg);
      break;
    default:
      fprintf(stderr, "unknown option %c\n", ch);
      usage();
      if (ui_name)
	free(ui_name);
      exit(1);
    }
  }
  free(optstr);

  if (argc == optind) {
    usage();
    if (ui_name)
      free(ui_name);
    return 0;
  }

  c = uidata.c = config_create();
  config_load(c, "enfle.rc");

  st = uidata.st = streamer_create();
  ui = ui_create();
  ld = uidata.ld = loader_create();
  ar = uidata.ar = archiver_create();
  player = uidata.player = player_create();

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
      case ENFLE_PLUGIN_PLAYER:
	name = player_load(player, plugin);
	printf("%s by %s\n",
	       player_get_description(player, name),
	       player_get_author(player, name));
	break;
      case ENFLE_PLUGIN_SAVER:
      case ENFLE_PLUGIN_EFFECT:
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

  if (strcmp(argv[optind], "-") == 0) {
    archive_add(uidata.a, argv[1], strdup(argv[1]));
  } else {
    for (i = optind; i < argc; i++) {
      if (strcmp(argv[i], "-") == 0) {
	fprintf(stderr, "Cannot specify stdin(-) with other files. ignored.\n");
      } else {
	archive_add(uidata.a, argv[i], strdup(argv[i]));
      }
    }
  }

  if (ui_name == NULL && ((ui_name = config_get(c, "/enfle/plugins/ui/default")) == NULL)) {
    fprintf(stderr, "configuration error\n");
    return 1;
  }

  if (!ui_call(ui, ui_name, &uidata))
    fprintf(stderr, "No UI %s or UI %s initialize failed\n", ui_name, ui_name);

  archive_destroy(uidata.a);
  archiver_destroy(ar);
  streamer_destroy(st);
  config_destroy(c);
  loader_destroy(ld);
  ui_destroy(ui);

  return 0;
}
